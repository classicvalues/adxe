/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
Copyright (c) 2021 Bytedance Inc.

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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include <stdarg.h>
#include "2d/CCLayer.h"
#include "base/CCScriptSupport.h"
#include "base/ccUtils.h"
#include "platform/CCDevice.h"
#include "renderer/CCRenderer.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"
#include "base/CCEventTouch.h"
#include "base/CCEventKeyboard.h"
#include "base/CCEventListenerKeyboard.h"
#include "base/CCEventAcceleration.h"
#include "base/CCEventListenerAcceleration.h"
#include "base/ccUTF8.h"
#include "renderer/backend/Buffer.h"
#include "renderer/ccShaders.h"
#include "renderer/backend/ProgramState.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#    include "platform/desktop/CCGLViewImpl-desktop.h"
#endif

NS_CC_BEGIN

// Layer
Layer::Layer()
{
    _ignoreAnchorPointForPosition = true;
    setAnchorPoint(Vec2(0.5f, 0.5f));
}

Layer::~Layer() {}

bool Layer::init()
{
    setContentSize(_director->getWinSize());
    return true;
}

Layer* Layer::create()
{
    Layer* ret = new Layer();
    if (ret->init())
    {
        ret->autorelease();
        return ret;
    }
    else
    {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
}

std::string Layer::getDescription() const
{
    return StringUtils::format("<Layer | Tag = %d>", _tag);
}

/// LayerColor

LayerColor::LayerColor()
{
    // default blend function
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

    auto& pipelinePS = _customCommand.getPipelineDescriptor().programState;
    auto* program    = backend::Program::getBuiltinProgram(backend::ProgramType::POSITION_COLOR);

    //!!! LayerColor private programState don't want affect by Node::_programState, so store at _customCommand
    pipelinePS = new backend::ProgramState(program);

    auto vertexLayout         = pipelinePS->getVertexLayout();
    const auto& attributeInfo = pipelinePS->getProgram()->getActiveAttributes();
    auto iter                 = attributeInfo.find("a_position");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_position", iter->second.location, backend::VertexFormat::FLOAT3, 0, false);
    }
    iter = attributeInfo.find("a_color");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_color", iter->second.location, backend::VertexFormat::FLOAT4,
                                   sizeof(_vertexData[0].vertices), false);
    }
    vertexLayout->setLayout(sizeof(_vertexData[0]));

    _mvpMatrixLocation = pipelinePS->getUniformLocation("u_MVPMatrix");

    _customCommand.createIndexBuffer(CustomCommand::IndexFormat::U_SHORT, 6, CustomCommand::BufferUsage::STATIC);
    unsigned short indices[] = {0, 1, 2, 2, 1, 3};
    _customCommand.updateIndexBuffer(indices, sizeof(indices));

    _customCommand.createVertexBuffer(sizeof(_vertexData[0]), 4, CustomCommand::BufferUsage::DYNAMIC);

    _customCommand.setDrawType(CustomCommand::DrawType::ELEMENT);
    _customCommand.setPrimitiveType(CustomCommand::PrimitiveType::TRIANGLE);
}

LayerColor::~LayerColor()
{
    CC_SAFE_RELEASE_NULL(_customCommand.getPipelineDescriptor().programState);
}

/// blendFunc getter
const BlendFunc& LayerColor::getBlendFunc() const
{
    return _blendFunc;
}
/// blendFunc setter
void LayerColor::setBlendFunc(const BlendFunc& var)
{
    _blendFunc = var;
}

LayerColor* LayerColor::create()
{
    LayerColor* ret = new LayerColor();
    if (ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

LayerColor* LayerColor::create(const Color4B& color, float width, float height)
{
    LayerColor* ret = new LayerColor();
    if (ret->initWithColor(color, width, height))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

LayerColor* LayerColor::create(const Color4B& color)
{
    LayerColor* ret = new LayerColor();
    if (ret->initWithColor(color))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayerColor::init()
{
    Vec2 s = _director->getWinSize();
    return initWithColor(Color4B(0, 0, 0, 0), s.width, s.height);
}

bool LayerColor::initWithColor(const Color4B& color, float w, float h)
{
    if (Layer::init())
    {

        // default blend function
        _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;

        _displayedColor.r = _realColor.r = color.r;
        _displayedColor.g = _realColor.g = color.g;
        _displayedColor.b = _realColor.b = color.b;
        _displayedOpacity = _realOpacity = color.a;

        for (size_t i = 0; i < sizeof(_squareVertices) / sizeof(_squareVertices[0]); i++)
        {
            _squareVertices[i].x = 0.0f;
            _squareVertices[i].y = 0.0f;
        }

        updateColor();
        setContentSize(Vec2(w, h));

        return true;
    }
    return false;
}

bool LayerColor::initWithColor(const Color4B& color)
{
    Vec2 s = _director->getWinSize();
    return initWithColor(color, s.width, s.height);
}

/// override contentSize
void LayerColor::setContentSize(const Vec2& size)
{
    _squareVertices[1].x = size.width;
    _squareVertices[2].y = size.height;
    _squareVertices[3].x = size.width;
    _squareVertices[3].y = size.height;

    Layer::setContentSize(size);
}

void LayerColor::changeWidthAndHeight(float w, float h)
{
    this->setContentSize(Vec2(w, h));
}

void LayerColor::changeWidth(float w)
{
    this->setContentSize(Vec2(w, _contentSize.height));
}

void LayerColor::changeHeight(float h)
{
    this->setContentSize(Vec2(_contentSize.width, h));
}

void LayerColor::updateColor()
{
    for (int i = 0; i < 4; i++)
    {
        _vertexData[i].colors.r = _displayedColor.r / 255.0f;
        _vertexData[i].colors.g = _displayedColor.g / 255.0f;
        _vertexData[i].colors.b = _displayedColor.b / 255.0f;
        _vertexData[i].colors.a = _displayedOpacity / 255.0f;
    }
}

void LayerColor::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder, _blendFunc);
    renderer->addCommand(&_customCommand);

    cocos2d::Mat4 projectionMat = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    auto& pipelineDescriptor    = _customCommand.getPipelineDescriptor();
    pipelineDescriptor.programState->setUniform(_mvpMatrixLocation, projectionMat.m, sizeof(projectionMat.m));

    for (int i = 0; i < 4; ++i)
    {
        Vec4 pos;
        pos.x = _squareVertices[i].x;
        pos.y = _squareVertices[i].y;
        pos.z = _positionZ;
        pos.w = 1;
        _modelViewTransform.transformVector(&pos);
        _vertexData[i].vertices = Vec3(pos.x, pos.y, pos.z) / pos.w;
    }
    updateVertexBuffer();
}

void LayerColor::updateVertexBuffer()
{
    _customCommand.updateVertexBuffer(_vertexData, sizeof(_vertexData));
}

//
// LayerGradient
//
LayerGradient::LayerGradient() {}

LayerGradient::~LayerGradient() {}

LayerGradient* LayerGradient::create(const Color4B& start, const Color4B& end)
{
    LayerGradient* ret = new LayerGradient();
    if (ret->initWithColor(start, end))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

LayerGradient* LayerGradient::create(const Color4B& start, const Color4B& end, const Vec2& v)
{
    LayerGradient* ret = new LayerGradient();
    if (ret->initWithColor(start, end, v))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

LayerGradient* LayerGradient::create()
{
    LayerGradient* ret = new LayerGradient();
    if (ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

bool LayerGradient::init()
{
    return initWithColor(Color4B(0, 0, 0, 255), Color4B(0, 0, 0, 255));
}

bool LayerGradient::initWithColor(const Color4B& start, const Color4B& end)
{
    return initWithColor(start, end, Vec2(0, -1));
}

bool LayerGradient::initWithColor(const Color4B& start, const Color4B& end, const Vec2& v)
{
    _endColor.r = end.r;
    _endColor.g = end.g;
    _endColor.b = end.b;

    _endOpacity   = end.a;
    _startOpacity = start.a;
    _alongVector  = v;

    _compressedInterpolation = true;

    return LayerColor::initWithColor(Color4B(start.r, start.g, start.b, 255));
}

void LayerGradient::updateColor()
{
    LayerColor::updateColor();

    float h = _alongVector.getLength();
    if (h == 0)
        return;

    float c = sqrtf(2.0f);
    Vec2 u(_alongVector.x / h, _alongVector.y / h);

    // Compressed Interpolation mode
    if (_compressedInterpolation)
    {
        float h2 = 1 / (fabsf(u.x) + fabsf(u.y));
        u        = u * (h2 * (float)c);
    }

    float opacityf = (float)_displayedOpacity / 255.0f;

    Color4F S(_displayedColor.r / 255.0f, _displayedColor.g / 255.0f, _displayedColor.b / 255.0f,
              _startOpacity * opacityf / 255.0f);

    Color4F E(_endColor.r / 255.0f, _endColor.g / 255.0f, _endColor.b / 255.0f, _endOpacity * opacityf / 255.0f);

    // (-1, -1)
    _vertexData[0].colors.r = E.r + (S.r - E.r) * ((c + u.x + u.y) / (2.0f * c));
    _vertexData[0].colors.g = E.g + (S.g - E.g) * ((c + u.x + u.y) / (2.0f * c));
    _vertexData[0].colors.b = E.b + (S.b - E.b) * ((c + u.x + u.y) / (2.0f * c));
    _vertexData[0].colors.a = E.a + (S.a - E.a) * ((c + u.x + u.y) / (2.0f * c));
    // (1, -1)
    _vertexData[1].colors.r = E.r + (S.r - E.r) * ((c - u.x + u.y) / (2.0f * c));
    _vertexData[1].colors.g = E.g + (S.g - E.g) * ((c - u.x + u.y) / (2.0f * c));
    _vertexData[1].colors.b = E.b + (S.b - E.b) * ((c - u.x + u.y) / (2.0f * c));
    _vertexData[1].colors.a = E.a + (S.a - E.a) * ((c - u.x + u.y) / (2.0f * c));
    // (-1, 1)
    _vertexData[2].colors.r = E.r + (S.r - E.r) * ((c + u.x - u.y) / (2.0f * c));
    _vertexData[2].colors.g = E.g + (S.g - E.g) * ((c + u.x - u.y) / (2.0f * c));
    _vertexData[2].colors.b = E.b + (S.b - E.b) * ((c + u.x - u.y) / (2.0f * c));
    _vertexData[2].colors.a = E.a + (S.a - E.a) * ((c + u.x - u.y) / (2.0f * c));
    // (1, 1)
    _vertexData[3].colors.r = E.r + (S.r - E.r) * ((c - u.x - u.y) / (2.0f * c));
    _vertexData[3].colors.g = E.g + (S.g - E.g) * ((c - u.x - u.y) / (2.0f * c));
    _vertexData[3].colors.b = E.b + (S.b - E.b) * ((c - u.x - u.y) / (2.0f * c));
    _vertexData[3].colors.a = E.a + (S.a - E.a) * ((c - u.x - u.y) / (2.0f * c));
}

const Color3B& LayerGradient::getStartColor() const
{
    return _realColor;
}

void LayerGradient::setStartColor(const Color3B& color)
{
    setColor(color);
}

void LayerGradient::setEndColor(const Color3B& color)
{
    _endColor = color;
    updateColor();
}

const Color3B& LayerGradient::getEndColor() const
{
    return _endColor;
}

void LayerGradient::setStartOpacity(uint8_t o)
{
    _startOpacity = o;
    updateColor();
}

uint8_t LayerGradient::getStartOpacity() const
{
    return _startOpacity;
}

void LayerGradient::setEndOpacity(uint8_t o)
{
    _endOpacity = o;
    updateColor();
}

uint8_t LayerGradient::getEndOpacity() const
{
    return _endOpacity;
}

void LayerGradient::setVector(const Vec2& var)
{
    _alongVector = var;
    updateColor();
}

const Vec2& LayerGradient::getVector() const
{
    return _alongVector;
}

bool LayerGradient::isCompressedInterpolation() const
{
    return _compressedInterpolation;
}

void LayerGradient::setCompressedInterpolation(bool compress)
{
    _compressedInterpolation = compress;
    updateColor();
}

std::string LayerGradient::getDescription() const
{
    return StringUtils::format("<LayerGradient | Tag = %d>", _tag);
}

/**
 * LayerRadialGradient
 */
LayerRadialGradient* LayerRadialGradient::create(const Color4B& startColor,
                                                 const Color4B& endColor,
                                                 float radius,
                                                 const Vec2& center,
                                                 float expand)
{
    auto layerGradient = new LayerRadialGradient();
    if (layerGradient && layerGradient->initWithColor(startColor, endColor, radius, center, expand))
    {
        layerGradient->autorelease();
        return layerGradient;
    }

    delete layerGradient;
    return nullptr;
}

LayerRadialGradient* LayerRadialGradient::create()
{
    auto layerGradient = new LayerRadialGradient();
    if (layerGradient && layerGradient->initWithColor(Color4B::BLACK, Color4B::BLACK, 0, Vec2(0, 0), 0))
    {
        layerGradient->autorelease();
        return layerGradient;
    }

    delete layerGradient;
    return nullptr;
}

LayerRadialGradient::LayerRadialGradient()
{
    auto& pipelinePS = _customCommand.getPipelineDescriptor().programState;
    auto* program    = backend::Program::getBuiltinProgram(backend::ProgramType::LAYER_RADIA_GRADIENT);
    //!!! LayerRadialGradient private programState don't want affect by Node::_programState, so store at _customCommand
    pipelinePS          = new backend::ProgramState(program);
    _mvpMatrixLocation  = pipelinePS->getUniformLocation("u_MVPMatrix");
    _startColorLocation = pipelinePS->getUniformLocation("u_startColor");
    _endColorLocation   = pipelinePS->getUniformLocation("u_endColor");
    _centerLocation     = pipelinePS->getUniformLocation("u_center");
    _radiusLocation     = pipelinePS->getUniformLocation("u_radius");
    _expandLocation     = pipelinePS->getUniformLocation("u_expand");

    auto vertexLayout         = pipelinePS->getVertexLayout();
    const auto& attributeInfo = pipelinePS->getProgram()->getActiveAttributes();
    auto iter                 = attributeInfo.find("a_position");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_position", iter->second.location, backend::VertexFormat::FLOAT2, 0, false);
    }
    vertexLayout->setLayout(sizeof(_vertices[0]));

    _customCommand.createVertexBuffer(sizeof(_vertices[0]), sizeof(_vertices) / sizeof(_vertices[0]),
                                      CustomCommand::BufferUsage::STATIC);
    _customCommand.setDrawType(CustomCommand::DrawType::ARRAY);
    _customCommand.setPrimitiveType(CustomCommand::PrimitiveType::TRIANGLE_STRIP);
}

LayerRadialGradient::~LayerRadialGradient()
{
    CC_SAFE_RELEASE_NULL(_customCommand.getPipelineDescriptor().programState);
}

bool LayerRadialGradient::initWithColor(const cocos2d::Color4B& startColor,
                                        const cocos2d::Color4B& endColor,
                                        float radius,
                                        const Vec2& center,
                                        float expand)
{
    // should do it before Layer::init()
    for (int i = 0; i < 4; ++i)
        _vertices[i] = {0.0f, 0.0f};

    if (Layer::init())
    {
        convertColor4B24F(_startColorRend, startColor);
        _startColor = startColor;

        convertColor4B24F(_endColorRend, endColor);
        _endColor = endColor;

        _expand = expand;

        setRadius(radius);
        setCenter(center);

        return true;
    }

    return false;
}

void LayerRadialGradient::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder, _blendFunc);
    renderer->addCommand(&_customCommand);

    const auto& projectionMat = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    auto programState         = _customCommand.getPipelineDescriptor().programState;
    Mat4 finalMat             = projectionMat * transform;
    programState->setUniform(_mvpMatrixLocation, finalMat.m, sizeof(finalMat.m));

    programState->setUniform(_startColorLocation, &_startColorRend, sizeof(_startColorRend));
    programState->setUniform(_endColorLocation, &_endColorRend, sizeof(_endColorRend));
    programState->setUniform(_centerLocation, &_center, sizeof(_center));
    programState->setUniform(_radiusLocation, &_radius, sizeof(_radius));
    programState->setUniform(_expandLocation, &_expand, sizeof(_expand));
}

void LayerRadialGradient::setContentSize(const Vec2& size)
{
    _vertices[1].x = size.width;
    _vertices[2].y = size.height;
    _vertices[3].x = size.width;
    _vertices[3].y = size.height;
    Layer::setContentSize(size);

    _customCommand.updateVertexBuffer(_vertices, sizeof(_vertices));
}

void LayerRadialGradient::setStartOpacity(uint8_t opacity)
{
    _startColorRend.a = opacity / 255.0f;
    _startColor.a     = opacity;
}

uint8_t LayerRadialGradient::getStartOpacity() const
{
    return _startColor.a;
}

void LayerRadialGradient::setEndOpacity(uint8_t opacity)
{
    _endColorRend.a = opacity / 255.0f;
    _endColor.a     = opacity;
}

uint8_t LayerRadialGradient::getEndOpacity() const
{
    return _endColor.a;
}

void LayerRadialGradient::setRadius(float radius)
{
    _radius = radius;
}

float LayerRadialGradient::getRadius() const
{
    return _radius;
}

void LayerRadialGradient::setCenter(const Vec2& center)
{
    _center = center;
}

Vec2 LayerRadialGradient::getCenter() const
{
    return _center;
}

void LayerRadialGradient::setExpand(float expand)
{
    _expand = expand;
}

float LayerRadialGradient::getExpand() const
{
    return _expand;
}

void LayerRadialGradient::setStartColor(const Color3B& color)
{
    setStartColor(Color4B(color));
}

void LayerRadialGradient::setStartColor(const cocos2d::Color4B& color)
{
    _startColor = color;
    convertColor4B24F(_startColorRend, _startColor);
}

Color4B LayerRadialGradient::getStartColor() const
{
    return _startColor;
}

Color3B LayerRadialGradient::getStartColor3B() const
{
    return Color3B(_startColor);
}

void LayerRadialGradient::setEndColor(const Color3B& color)
{
    setEndColor(Color4B(color));
}

void LayerRadialGradient::setEndColor(const cocos2d::Color4B& color)
{
    _endColor = color;
    convertColor4B24F(_endColorRend, _endColor);
}

Color4B LayerRadialGradient::getEndColor() const
{
    return _endColor;
}

Color3B LayerRadialGradient::getEndColor3B() const
{
    return Color3B(_endColor);
}

void LayerRadialGradient::setBlendFunc(const BlendFunc& blendFunc)
{
    _blendFunc = blendFunc;
}

BlendFunc LayerRadialGradient::getBlendFunc() const
{
    return _blendFunc;
}

void LayerRadialGradient::convertColor4B24F(Color4F& outColor, const Color4B& inColor)
{
    outColor.r = inColor.r / 255.0f;
    outColor.g = inColor.g / 255.0f;
    outColor.b = inColor.b / 255.0f;
    outColor.a = inColor.a / 255.0f;
}

/// MultiplexLayer

LayerMultiplex::LayerMultiplex() : _enabledLayer(0) {}

LayerMultiplex::~LayerMultiplex()
{
    for (const auto& layer : _layers)
    {
        layer->cleanup();
    }
}

LayerMultiplex* LayerMultiplex::create(Node* layer, ...)
{
    va_list args;
    va_start(args, layer);

    LayerMultiplex* ret = new LayerMultiplex();
    if (ret->initWithLayers(layer, args))
    {
        ret->autorelease();
        va_end(args);
        return ret;
    }
    va_end(args);
    CC_SAFE_DELETE(ret);
    return nullptr;
}

LayerMultiplex* LayerMultiplex::createWithLayer(Node* layer)
{
    return LayerMultiplex::create(layer, nullptr);
}

LayerMultiplex* LayerMultiplex::create()
{
    LayerMultiplex* ret = new LayerMultiplex();
    if (ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

LayerMultiplex* LayerMultiplex::createWithArray(const Vector<Node*>& arrayOfLayers)
{
    LayerMultiplex* ret = new LayerMultiplex();
    if (ret->initWithArray(arrayOfLayers))
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

void LayerMultiplex::addLayer(Node* layer)
{
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
    auto sEngine = ScriptEngineManager::getInstance()->getScriptEngine();
    if (sEngine)
    {
        sEngine->retainScriptObject(this, layer);
    }
#endif  // CC_ENABLE_GC_FOR_NATIVE_OBJECTS
    _layers.pushBack(layer);
}

bool LayerMultiplex::init()
{
    if (Layer::init())
    {
        _enabledLayer = 0;
        return true;
    }
    return false;
}

bool LayerMultiplex::initWithLayers(Node* layer, va_list params)
{
    if (Layer::init())
    {
        _layers.reserve(5);
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
        auto sEngine = ScriptEngineManager::getInstance()->getScriptEngine();
        if (sEngine)
        {
            sEngine->retainScriptObject(this, layer);
        }
#endif  // CC_ENABLE_GC_FOR_NATIVE_OBJECTS
        _layers.pushBack(layer);

        Layer* l = va_arg(params, Layer*);
        while (l)
        {
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
            if (sEngine)
            {
                sEngine->retainScriptObject(this, l);
            }
#endif  // CC_ENABLE_GC_FOR_NATIVE_OBJECTS
            _layers.pushBack(l);
            l = va_arg(params, Layer*);
        }

        _enabledLayer = 0;
        this->addChild(_layers.at(_enabledLayer));
        return true;
    }

    return false;
}

bool LayerMultiplex::initWithArray(const Vector<Node*>& arrayOfLayers)
{
    if (Layer::init())
    {
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
        auto sEngine = ScriptEngineManager::getInstance()->getScriptEngine();
        if (sEngine)
        {
            for (const auto& layer : arrayOfLayers)
            {
                if (layer)
                {
                    sEngine->retainScriptObject(this, layer);
                }
            }
        }
#endif  // CC_ENABLE_GC_FOR_NATIVE_OBJECTS
        _layers.reserve(arrayOfLayers.size());
        _layers.pushBack(arrayOfLayers);

        _enabledLayer = 0;
        this->addChild(_layers.at(_enabledLayer));
        return true;
    }
    return false;
}

void LayerMultiplex::switchTo(int n)
{

    switchTo(n, true);
}

void LayerMultiplex::switchTo(int n, bool cleanup)
{
    CCASSERT(n < _layers.size(), "Invalid index in MultiplexLayer switchTo message");

    this->removeChild(_layers.at(_enabledLayer), cleanup);

    _enabledLayer = n;

    this->addChild(_layers.at(n));
}

void LayerMultiplex::switchToAndReleaseMe(int n)
{
    CCASSERT(n < _layers.size(), "Invalid index in MultiplexLayer switchTo message");

    this->removeChild(_layers.at(_enabledLayer), true);
#if CC_ENABLE_GC_FOR_NATIVE_OBJECTS
    auto sEngine = ScriptEngineManager::getInstance()->getScriptEngine();
    if (sEngine)
    {
        sEngine->releaseScriptObject(this, _layers.at(_enabledLayer));
    }
#endif  // CC_ENABLE_GC_FOR_NATIVE_OBJECTS

    _layers.replace(_enabledLayer, nullptr);

    _enabledLayer = n;

    this->addChild(_layers.at(n));
}

std::string LayerMultiplex::getDescription() const
{
    return StringUtils::format("<LayerMultiplex | Tag = %d, Layers = %d", _tag, static_cast<int>(_children.size()));
}

NS_CC_END
