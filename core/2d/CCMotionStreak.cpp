/****************************************************************************
Copyright (c) 2011      ForzeField Studios S.L.
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

https://adxeproject.github.io/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN false EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "2d/CCMotionStreak.h"
#include "math/CCVertex.h"
#include "base/CCDirector.h"
#include "base/ccUtils.h"
#include "renderer/CCTextureCache.h"
#include "renderer/CCTexture2D.h"
#include "renderer/CCRenderer.h"
#include "renderer/ccShaders.h"
#include "renderer/backend/ProgramState.h"

NS_CC_BEGIN

MotionStreak::MotionStreak()
{
    _customCommand.setDrawType(CustomCommand::DrawType::ARRAY);
    _customCommand.setPrimitiveType(CustomCommand::PrimitiveType::TRIANGLE_STRIP);
}

MotionStreak::~MotionStreak()
{
    CC_SAFE_RELEASE(_texture);
    CC_SAFE_FREE(_pointState);
    CC_SAFE_FREE(_pointVertexes);
    CC_SAFE_FREE(_vertices);
    CC_SAFE_FREE(_colorPointer);
    CC_SAFE_FREE(_texCoords);
}

MotionStreak* MotionStreak::create(float fade, float minSeg, float stroke, const Color3B& color, std::string_view path)
{
    MotionStreak* ret = new MotionStreak();
    if (ret->initWithFade(fade, minSeg, stroke, color, path))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

MotionStreak* MotionStreak::create(float fade, float minSeg, float stroke, const Color3B& color, Texture2D* texture)
{
    MotionStreak* ret = new MotionStreak();
    if (ret->initWithFade(fade, minSeg, stroke, color, texture))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool MotionStreak::initWithFade(float fade, float minSeg, float stroke, const Color3B& color, std::string_view path)
{
    CCASSERT(!path.empty(), "Invalid filename");

    Texture2D* texture = _director->getTextureCache()->addImage(path);
    return initWithFade(fade, minSeg, stroke, color, texture);
}

bool MotionStreak::initWithFade(float fade, float minSeg, float stroke, const Color3B& color, Texture2D* texture)
{
    Node::setPosition(Vec2::ZERO);
    setAnchorPoint(Vec2::ZERO);
    setIgnoreAnchorPointForPosition(true);
    _startingPositionInitialized = false;

    _positionR.setZero();
    _fastMode = true;
    _minSeg   = (minSeg == -1.0f) ? stroke / 5.0f : minSeg;
    _minSeg *= _minSeg;

    _stroke    = stroke;
    _fadeDelta = 1.0f / fade;

    double fps = 1 / _director->getAnimationInterval();
    _maxPoints = (int)(fade * fps) + 2;

    _pointState    = (float*)malloc(sizeof(float) * _maxPoints);
    _pointVertexes = (Vec2*)malloc(sizeof(Vec2) * _maxPoints);

    const size_t VERTEX_SIZE = sizeof(Vec2) + sizeof(Tex2F) + sizeof(uint8_t) * 4;

    _vertexCount  = _maxPoints * 2;
    _vertices     = (Vec2*)malloc(sizeof(Vec2) * _vertexCount);
    _texCoords    = (Tex2F*)malloc(sizeof(Tex2F) * _vertexCount);
    _colorPointer = (uint8_t*)malloc(sizeof(uint8_t) * 4 * _vertexCount);
    _customCommand.createVertexBuffer(VERTEX_SIZE, _vertexCount, CustomCommand::BufferUsage::DYNAMIC);

    std::vector<uint8_t> zeros;
    zeros.resize(VERTEX_SIZE * _vertexCount);
    std::fill(zeros.begin(), zeros.end(), 0);
    _customCommand.updateVertexBuffer(zeros.data(), zeros.size());

    setTexture(texture);
    setColor(color);
    scheduleUpdate();

    return true;
}

void MotionStreak::setPosition(const Vec2& position)
{
    if (!_startingPositionInitialized)
    {
        _startingPositionInitialized = true;
    }
    _positionR = position;
}

void MotionStreak::setPosition(float x, float y)
{
    if (!_startingPositionInitialized)
    {
        _startingPositionInitialized = true;
    }
    _positionR.x = x;
    _positionR.y = y;
}

const Vec2& MotionStreak::getPosition() const
{
    return _positionR;
}

void MotionStreak::getPosition(float* x, float* y) const
{
    *x = _positionR.x;
    *y = _positionR.y;
}

float MotionStreak::getPositionX() const
{
    return _positionR.x;
}

Vec3 MotionStreak::getPosition3D() const
{
    return Vec3(_positionR.x, _positionR.y, getPositionZ());
}

void MotionStreak::setPositionX(float x)
{
    if (!_startingPositionInitialized)
    {
        _startingPositionInitialized = true;
    }
    _positionR.x = x;
}

float MotionStreak::getPositionY() const
{
    return _positionR.y;
}

void MotionStreak::setPositionY(float y)
{
    if (!_startingPositionInitialized)
    {
        _startingPositionInitialized = true;
    }
    _positionR.y = y;
}

void MotionStreak::tintWithColor(const Color3B& colors)
{
    setColor(colors);

    // Fast assignation
    for (unsigned int i = 0; i < _nuPoints * 2; i++)
    {
        *((Color3B*)(_colorPointer + i * 4)) = colors;
    }
}

Texture2D* MotionStreak::getTexture() const
{
    return _texture;
}

void MotionStreak::setTexture(Texture2D* texture)
{
    if (_texture != texture)
    {
        CC_SAFE_RETAIN(texture);
        CC_SAFE_RELEASE(_texture);
        _texture = texture;

        setProgramStateWithRegistry(backend::ProgramType::POSITION_TEXTURE_COLOR, _texture);
    }
}

bool MotionStreak::setProgramState(backend::ProgramState* programState, bool needsRetain)
{
    if (Node::setProgramState(programState, needsRetain))
    {
        CCASSERT(programState, "argument should not be nullptr");
        auto& pipelineDescriptor        = _customCommand.getPipelineDescriptor();
        pipelineDescriptor.programState = _programState;

        _mvpMatrixLocaiton = _programState->getUniformLocation("u_MVPMatrix");
        _textureLocation   = _programState->getUniformLocation("u_texture");

        auto vertexLayout         = _programState->getVertexLayout();
        const auto& attributeInfo = _programState->getProgram()->getActiveAttributes();
        auto iter                 = attributeInfo.find("a_position");
        if (iter != attributeInfo.end())
        {
            vertexLayout->setAttribute("a_position", iter->second.location, backend::VertexFormat::FLOAT2, 0, false);
        }
        iter = attributeInfo.find("a_texCoord");
        if (iter != attributeInfo.end())
        {
            vertexLayout->setAttribute("a_texCoord", iter->second.location, backend::VertexFormat::FLOAT2,
                                       2 * sizeof(float), false);
        }
        iter = attributeInfo.find("a_color");
        if (iter != attributeInfo.end())
        {
            vertexLayout->setAttribute("a_color", iter->second.location, backend::VertexFormat::UBYTE4,
                                       4 * sizeof(float), true);
        }
        vertexLayout->setLayout(4 * sizeof(float) + 4 * sizeof(uint8_t));

        updateProgramStateTexture(_texture);
        return true;
    }
    return false;
}

void MotionStreak::setBlendFunc(const BlendFunc& blendFunc)
{
    _blendFunc = blendFunc;
}

const BlendFunc& MotionStreak::getBlendFunc() const
{
    return _blendFunc;
}

void MotionStreak::setOpacity(uint8_t /*opacity*/)
{
    CCASSERT(false, "Set opacity no supported");
}

uint8_t MotionStreak::getOpacity() const
{
    CCASSERT(false, "Opacity no supported");
    return 0;
}

void MotionStreak::setOpacityModifyRGB(bool /*bValue*/) {}

bool MotionStreak::isOpacityModifyRGB() const
{
    return false;
}

void MotionStreak::update(float delta)
{
    if (!_startingPositionInitialized)
        return;

    delta *= _fadeDelta;

    unsigned int newIdx, newIdx2, i, i2;
    unsigned int mov = 0;

    // Update current points
    for (i = 0; i < _nuPoints; i++)
    {
        _pointState[i] -= delta;

        if (_pointState[i] <= 0)
            mov++;
        else
        {
            newIdx = i - mov;

            if (mov > 0)
            {
                // Move data
                _pointState[newIdx] = _pointState[i];

                // Move point
                _pointVertexes[newIdx] = _pointVertexes[i];

                // Move vertices
                i2                     = i * 2;
                newIdx2                = newIdx * 2;
                _vertices[newIdx2]     = _vertices[i2];
                _vertices[newIdx2 + 1] = _vertices[i2 + 1];

                // Move color
                i2 *= 4;
                newIdx2 *= 4;
                _colorPointer[newIdx2 + 0] = _colorPointer[i2 + 0];
                _colorPointer[newIdx2 + 1] = _colorPointer[i2 + 1];
                _colorPointer[newIdx2 + 2] = _colorPointer[i2 + 2];
                _colorPointer[newIdx2 + 4] = _colorPointer[i2 + 4];
                _colorPointer[newIdx2 + 5] = _colorPointer[i2 + 5];
                _colorPointer[newIdx2 + 6] = _colorPointer[i2 + 6];
            }
            else
                newIdx2 = newIdx * 8;

            const uint8_t op           = (uint8_t)(_pointState[newIdx] * 255.0f);
            _colorPointer[newIdx2 + 3] = op;
            _colorPointer[newIdx2 + 7] = op;
        }
    }
    _nuPoints -= mov;

    // Append new point
    bool appendNewPoint = true;
    if (_nuPoints >= _maxPoints)
        appendNewPoint = false;
    else if (_nuPoints > 0)
    {
        bool a1 = _pointVertexes[_nuPoints - 1].getDistanceSq(_positionR) < _minSeg;
        bool a2 =
            (_nuPoints == 1) ? false : (_pointVertexes[_nuPoints - 2].getDistanceSq(_positionR) < (_minSeg * 2.0f));
        if (a1 || a2)
            appendNewPoint = false;
    }

    if (appendNewPoint)
    {
        _pointVertexes[_nuPoints] = _positionR;
        _pointState[_nuPoints]    = 1.0f;

        // Color assignment
        const unsigned int offset                 = _nuPoints * 8;
        *((Color3B*)(_colorPointer + offset))     = _displayedColor;
        *((Color3B*)(_colorPointer + offset + 4)) = _displayedColor;

        // Opacity
        _colorPointer[offset + 3] = 255;
        _colorPointer[offset + 7] = 255;

        // Generate polygon
        if (_nuPoints > 0 && _fastMode)
        {
            if (_nuPoints > 1)
            {
                ccVertexLineToPolygon(_pointVertexes, _stroke, _vertices, _nuPoints, 1);
            }
            else
            {
                ccVertexLineToPolygon(_pointVertexes, _stroke, _vertices, 0, 2);
            }
        }

        _nuPoints++;
    }

    if (!_fastMode)
        ccVertexLineToPolygon(_pointVertexes, _stroke, _vertices, 0, _nuPoints);

    // Updated Tex Coords only if they are different than previous step
    if (_nuPoints && _previousNuPoints != _nuPoints)
    {
        float texDelta = 1.0f / _nuPoints;
        for (i = 0; i < _nuPoints; i++)
        {
            _texCoords[i * 2]     = Tex2F(0, texDelta * i);
            _texCoords[i * 2 + 1] = Tex2F(1, texDelta * i);
        }

        _previousNuPoints = _nuPoints;
    }
}

void MotionStreak::reset()
{
    _nuPoints = 0;
}

void MotionStreak::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    if (_nuPoints <= 1)
        return;

    auto drawCount = _nuPoints * 2;

    _customCommand.init(_globalZOrder, _blendFunc);
    _customCommand.setVertexDrawInfo(0, drawCount);
    renderer->addCommand(&_customCommand);

    auto programState = _customCommand.getPipelineDescriptor().programState;

    const auto& projectionMat = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    Mat4 finalMat             = projectionMat * transform;
    programState->setUniform(_mvpMatrixLocaiton, finalMat.m, sizeof(Mat4));

    unsigned int offset     = 0;
    unsigned int vertexSize = sizeof(Vec2) + sizeof(Vec2) + sizeof(uint8_t) * 4;
    for (unsigned int i = 0; i < drawCount; ++i)
    {
        offset = i * vertexSize;
        _customCommand.updateVertexBuffer(&_vertices[i], offset, sizeof(_vertices[0]));
        _customCommand.updateVertexBuffer(&_texCoords[i], offset + sizeof(_vertices[0]), sizeof(_texCoords[0]));
        _customCommand.updateVertexBuffer(&_colorPointer[i * 4], offset + sizeof(_vertices[0]) + sizeof(_texCoords[0]),
                                          4 * sizeof(uint8_t));
    }
}

NS_CC_END
