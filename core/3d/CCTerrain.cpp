/****************************************************************************
Copyright (c) 2015-2016 Chukong Technologies Inc.
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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "3d/CCTerrain.h"

USING_NS_CC;
#include <stdlib.h>
#include <float.h>
#include <set>
#include <stddef.h>  // offsetof
#include "renderer/CCRenderer.h"
#include "renderer/ccShaders.h"
#include "renderer/backend/Device.h"
#include "renderer/backend/Program.h"
#include "renderer/backend/Buffer.h"
#include "base/CCDirector.h"
#include "base/ccTypes.h"
#include "base/CCEventType.h"
#include "2d/CCCamera.h"
#include "platform/CCImage.h"
#include "3d/CC3DProgramInfo.h"
#include "base/ccUtils.h"

NS_CC_BEGIN

namespace
{
// It's used for creating a default texture when lightMap is nullpter
static unsigned char cc_2x2_white_image[] = {
    // RGBA8888
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
}  // namespace

Terrain* Terrain::create(TerrainData& parameter, CrackFixedType fixedType)
{
    Terrain* terrain = new Terrain();
    if (terrain->initWithTerrainData(parameter, fixedType))
    {
        terrain->autorelease();
        return terrain;
    }
    CC_SAFE_DELETE(terrain);
    return terrain;
}
bool Terrain::initWithTerrainData(TerrainData& parameter, CrackFixedType fixedType)
{
    this->setSkirtHeightRatio(parameter._skirtHeightRatio);
    this->_terrainData         = parameter;
    this->_crackFixedType      = fixedType;
    this->_isCameraViewChanged = true;
    // chunksize
    this->_chunkSize = parameter._chunkSize;
    bool initResult  = true;

    // init heightmap
    initResult &= this->initHeightMap(parameter._heightMapSrc);
    // init textures alpha map,detail Maps
    initResult &= this->initTextures();
    initResult &= this->initProperties();

    return initResult;
}

void cocos2d::Terrain::setLightMap(std::string_view fileName)
{
    CC_SAFE_RELEASE(_lightMap);
    auto image = new Image();
    image->initWithImageFile(fileName);
    _lightMap = new Texture2D();
    _lightMap->initWithImage(image);

    Texture2D::TexParams tRepeatParams;  // set texture parameters
    tRepeatParams.magFilter = tRepeatParams.minFilter = backend::SamplerFilter::LINEAR;
    tRepeatParams.sAddressMode                        = backend::SamplerAddressMode::REPEAT;
    tRepeatParams.tAddressMode                        = backend::SamplerAddressMode::REPEAT;
    _lightMap->setTexParameters(tRepeatParams);
}

void cocos2d::Terrain::setLightDir(const Vec3& lightDir)
{
    _lightDir = lightDir;
}

bool Terrain::initProperties()
{
    auto* program = backend::Program::getBuiltinProgram(backend::ProgramType::TERRAIN_3D);
    setProgramState(new backend::ProgramState(program), false);

    _stateBlock.depthWrite = true;
    _stateBlock.depthTest  = true;
    _stateBlock.cullFace   = backend::CullMode::FRONT;

    setDrawWire(false);
    setIsEnableFrustumCull(true);
    setAnchorPoint(Vec2(0, 0));
    return true;
}

void Terrain::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
    auto modelMatrix = getNodeToWorldTransform();
    if (memcmp(&modelMatrix, &_terrainModelMatrix, sizeof(Mat4)) != 0)
    {
        _terrainModelMatrix = modelMatrix;
        _quadRoot->preCalculateAABB(_terrainModelMatrix);
    }

    auto& projectionMatrix = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    auto finalMatrix       = projectionMatrix * transform;
    _programState->setUniform(_mvpMatrixLocation, &finalMatrix.m, sizeof(finalMatrix.m));

    _programState->setUniform(_lightDirLocation, &_lightDir, sizeof(_lightDir));
    if (!_alphaMap)
    {
        _programState->setTexture(_detailMapLocation[0], 0, _detailMapTextures[0]->getBackendTexture());
        int hasAlphaMap = 0;
        _programState->setUniform(_alphaIsHasAlphaMapLocation, &hasAlphaMap, sizeof(hasAlphaMap));
    }
    else
    {
        float detailMapSize[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        for (int i = 0; i < _maxDetailMapValue; ++i)
        {
            _programState->setTexture(_detailMapLocation[i], i, _detailMapTextures[i]->getBackendTexture());
            detailMapSize[i] = _terrainData._detailMaps[i]._detailMapSize;
        }
        _programState->setUniform(_detailMapSizeLocation, detailMapSize, sizeof(detailMapSize));

        int hasAlphaMap = 1;
        _programState->setUniform(_alphaIsHasAlphaMapLocation, &hasAlphaMap, sizeof(hasAlphaMap));
        _programState->setTexture(_alphaMapLocation, 4, _alphaMap->getBackendTexture());
    }
    if (_lightMap)
    {
        int hasLightMap = 1;
        _programState->setUniform(_lightMapCheckLocation, &hasLightMap, sizeof(hasLightMap));
        _programState->setTexture(_lightMapLocation, 5, _lightMap->getBackendTexture());
    }
    else
    {
        int hasLightMap = 0;
        _programState->setUniform(_lightMapCheckLocation, &hasLightMap, sizeof(hasLightMap));
#ifdef CC_USE_METAL
        _programState->setTexture(_lightMapLocation, 5, _dummyTexture->getBackendTexture());
#endif
    }
    auto camera = Camera::getVisitingCamera();

    if (memcmp(&_CameraMatrix, &camera->getViewMatrix(), sizeof(Mat4)) != 0)
    {
        _isCameraViewChanged = true;
        _CameraMatrix        = camera->getViewMatrix();
    }

    if (_isCameraViewChanged)
    {
        auto m = camera->getNodeToWorldTransform();
        // set lod
        setChunksLOD(Vec3(m.m[12], m.m[13], m.m[14]));
    }

    if (_isCameraViewChanged)
    {
        _quadRoot->resetNeedDraw(true);  // reset it
        // camera frustum culling
        if (_isEnableFrustumCull)
        {
            _quadRoot->cullByCamera(camera, _terrainModelMatrix);
        }
    }
    _quadRoot->draw();
    if (_isCameraViewChanged)
    {
        _isCameraViewChanged = false;
    }
}

bool Terrain::initHeightMap(std::string_view heightMap)
{
    _heightMapImage = new Image();
    _heightMapImage->initWithImageFile(heightMap);
    _data        = _heightMapImage->getData();
    _imageWidth  = _heightMapImage->getWidth();
    _imageHeight = _heightMapImage->getHeight();

    // only the image size  is the Powers Of Two(POT) or POT+1
    if ((utils::isPOT(_imageWidth) && utils::isPOT(_imageHeight)) ||
        (utils::isPOT(_imageWidth - 1) && utils::isPOT(_imageHeight - 1)))
    {
        int chunk_amount_y = _imageHeight / _chunkSize.height;
        int chunk_amount_x = _imageWidth / _chunkSize.width;
        loadVertices();
        calculateNormal();
        memset(_chunkesArray, 0, sizeof(_chunkesArray));

        for (int m = 0; m < chunk_amount_y; m++)
        {
            for (int n = 0; n < chunk_amount_x; n++)
            {
                _chunkesArray[m][n]        = new Chunk(this);
                _chunkesArray[m][n]->_size = _chunkSize;
                _chunkesArray[m][n]->generate(_imageWidth, _imageHeight, m, n, _data);
            }
        }

        // calculate the neighbor
        for (int m = 0; m < chunk_amount_y; m++)
        {
            for (int n = 0; n < chunk_amount_x; n++)
            {
                if (n - 1 >= 0)
                    _chunkesArray[m][n]->_left = _chunkesArray[m][n - 1];
                if (n + 1 < chunk_amount_x)
                    _chunkesArray[m][n]->_right = _chunkesArray[m][n + 1];
                if (m - 1 >= 0)
                    _chunkesArray[m][n]->_back = _chunkesArray[m - 1][n];
                if (m + 1 < chunk_amount_y)
                    _chunkesArray[m][n]->_front = _chunkesArray[m + 1][n];
            }
        }
        _quadRoot = new QuadTree(0, 0, _imageWidth, _imageHeight, this);
        setLODDistance(_chunkSize.width, 2 * _chunkSize.width, 3 * _chunkSize.width);
        return true;
    }
    else
    {
        CCLOG("warning: the height map size is not POT or POT + 1");
        return false;
    }
}

Terrain::Terrain()
    : _alphaMap(nullptr)
    , _lightMap(nullptr)
    , _lightDir(-1.f, -1.f, 0.f)
#if CC_ENABLE_CACHE_TEXTURE_DATA
    , _backToForegroundListener(nullptr)
#endif
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    _backToForegroundListener =
        EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom*) { reload(); });
    _director->getEventDispatcher()->addEventListenerWithFixedPriority(_backToForegroundListener, 1);
#endif
#ifdef CC_USE_METAL
    auto image          = new Image();
    bool CC_UNUSED isOK = image->initWithRawData(cc_2x2_white_image, sizeof(cc_2x2_white_image), 2, 2, 8);
    CCASSERT(isOK, "The 2x2 empty texture was created unsuccessfully.");
    _dummyTexture = new Texture2D();
    _dummyTexture->initWithImage(image);
    CC_SAFE_RELEASE(image);
#endif
}

void Terrain::setChunksLOD(const Vec3& cameraPos)
{
    int chunk_amount_y = _imageHeight / _chunkSize.height;
    int chunk_amount_x = _imageWidth / _chunkSize.width;
    for (int m = 0; m < chunk_amount_y; m++)
        for (int n = 0; n < chunk_amount_x; n++)
        {
            AABB aabb                        = _chunkesArray[m][n]->_parent->_worldSpaceAABB;
            auto center                      = aabb.getCenter();
            float dist                       = Vec2(center.x, center.z).distance(Vec2(cameraPos.x, cameraPos.z));
            _chunkesArray[m][n]->_currentLod = 3;
            for (int i = 0; i < 3; ++i)
            {
                if (dist <= _lodDistance[i])
                {
                    _chunkesArray[m][n]->_currentLod = i;
                    break;
                }
            }
        }
}

float Terrain::getHeight(float x, float z, Vec3* normal) const
{
    Vec2 pos(x, z);

    // top-left
    Vec2 tl(-1 * _terrainData._mapScale * _imageWidth / 2, -1 * _terrainData._mapScale * _imageHeight / 2);
    auto mulResult = getNodeToWorldTransform() * Vec4(tl.x, 0.0f, tl.y, 1.0f);
    tl.set(mulResult.x, mulResult.z);

    Vec2 to_tl = pos - tl;

    // real size
    Vec2 size(_imageWidth * _terrainData._mapScale, _imageHeight * _terrainData._mapScale);
    mulResult = getNodeToWorldTransform() * Vec4(size.x, 0.0f, size.y, 0.0f);
    size.set(mulResult.x, mulResult.z);

    float width_ratio  = to_tl.x / size.x;
    float height_ratio = to_tl.y / size.y;

    float image_x = width_ratio * _imageWidth;
    float image_y = height_ratio * _imageHeight;
    float u       = image_x - (int)image_x;
    float v       = image_y - (int)image_y;
    float i       = (int)image_x;
    float j       = (int)image_y;

    if (image_x >= _imageWidth - 1 || image_y >= _imageHeight - 1 || image_x < 0 || image_y < 0)
    {
        if (normal)
        {
            normal->setZero();
        }
        return 0;
    }
    else
    {
        float a = getImageHeight(i, j) * getScaleY();
        float b = getImageHeight(i, j + 1) * getScaleY();
        float c = getImageHeight(i + 1, j) * getScaleY();
        float d = getImageHeight(i + 1, j + 1) * getScaleY();
        if (normal)
        {
            normal->x = c - b;
            normal->y = 2;
            normal->z = d - a;
            normal->normalize();
            //(*normal) = (1-u)*(1-v)*getNormal(i,j)+ (1-u)*v*getNormal(i,j+1) + u*(1-v)*getNormal(i+1,j)+
            // u*v*getNormal(i+1,j+1);
        }
        float result = (1 - u) * (1 - v) * getImageHeight(i, j) * getScaleY() +
                       (1 - u) * v * getImageHeight(i, j + 1) * getScaleY() +
                       u * (1 - v) * getImageHeight(i + 1, j) * getScaleY() +
                       u * v * getImageHeight(i + 1, j + 1) * getScaleY();
        return result;
    }
}

float Terrain::getHeight(const Vec2& pos, Vec3* normal) const
{
    return getHeight(pos.x, pos.y, normal);
}

float Terrain::getImageHeight(int pixel_x, int pixel_y) const
{
    int byte_stride = 1;
    switch (_heightMapImage->getPixelFormat())
    {
    case backend::PixelFormat::BGRA8:
        byte_stride = 4;
        break;
    case backend::PixelFormat::RGB8:
        byte_stride = 3;
        break;
    case backend::PixelFormat::L8:
        byte_stride = 1;
        break;
    default:
        break;
    }
    return _data[(pixel_y * _imageWidth + pixel_x) * byte_stride] * 1.0 / 255 * _terrainData._mapHeight -
           0.5 * _terrainData._mapHeight;
}

void Terrain::loadVertices()
{
    _maxHeight = -99999;
    _minHeight = 99999;
    for (int i = 0; i < _imageHeight; ++i)
    {
        for (int j = 0; j < _imageWidth; j++)
        {
            float height = getImageHeight(j, i);
            TerrainVertexData v;
            v._position = Vec3(j * _terrainData._mapScale - _imageWidth / 2 * _terrainData._mapScale,    // x
                               height,                                                                   // y
                               i * _terrainData._mapScale - _imageHeight / 2 * _terrainData._mapScale);  // z
            v._texcoord = Tex2F(j * 1.0 / _imageWidth, i * 1.0 / _imageHeight);
            _vertices.push_back(v);

            // update the min & max height;
            if (height > _maxHeight)
                _maxHeight = height;
            if (height < _minHeight)
                _minHeight = height;
        }
    }
}

void Terrain::calculateNormal()
{
    _indices.clear();
    // we generate whole terrain indices(global indices) for correct normal calculate
    for (int i = 0; i < _imageHeight - 1; i += 1)
    {
        for (int j = 0; j < _imageWidth - 1; j += 1)
        {

            int nLocIndex = i * _imageWidth + j;
            _indices.push_back(nLocIndex);
            _indices.push_back(nLocIndex + _imageWidth);
            _indices.push_back(nLocIndex + 1);

            _indices.push_back(nLocIndex + 1);
            _indices.push_back(nLocIndex + _imageWidth);
            _indices.push_back(nLocIndex + _imageWidth + 1);
        }
    }
    for (size_t i = 0, size = _indices.size(); i < size; i += 3)
    {
        unsigned int Index0 = _indices[i];
        unsigned int Index1 = _indices[i + 1];
        unsigned int Index2 = _indices[i + 2];
        Vec3 v1             = _vertices[Index1]._position - _vertices[Index0]._position;
        Vec3 v2             = _vertices[Index2]._position - _vertices[Index0]._position;
        Vec3 Normal;
        Vec3::cross(v1, v2, &Normal);
        Normal.normalize();
        _vertices[Index0]._normal += Normal;
        _vertices[Index1]._normal += Normal;
        _vertices[Index2]._normal += Normal;
    }

    for (auto& vertex : _vertices)
    {
        vertex._normal.normalize();
    }
    // global indices no need at all
    _indices.clear();
}

void Terrain::setDrawWire(bool bool_value)
{
    _isDrawWire = bool_value;
}

void Terrain::setLODDistance(float lod_1, float lod_2, float lod_3)
{
    _lodDistance[0] = lod_1;
    _lodDistance[1] = lod_2;
    _lodDistance[2] = lod_3;
}

void Terrain::setIsEnableFrustumCull(bool bool_value)
{
    _isEnableFrustumCull = bool_value;
}

Terrain::~Terrain()
{
    CC_SAFE_RELEASE(_alphaMap);
    CC_SAFE_RELEASE(_lightMap);
    CC_SAFE_RELEASE(_heightMapImage);
    CC_SAFE_RELEASE(_dummyTexture);
    delete _quadRoot;
    for (int i = 0; i < 4; ++i)
    {
        if (_detailMapTextures[i])
        {
            _detailMapTextures[i]->release();
        }
    }
    for (int i = 0; i < MAX_CHUNKES; ++i)
    {
        for (int j = 0; j < MAX_CHUNKES; j++)
        {
            if (_chunkesArray[i][j])
            {
                delete _chunkesArray[i][j];
            }
        }
    }

#if CC_ENABLE_CACHE_TEXTURE_DATA
    _director->getEventDispatcher()->removeEventListener(_backToForegroundListener);
#endif
}

cocos2d::Vec3 Terrain::getNormal(int pixel_x, int pixel_y) const
{
    float a = getImageHeight(pixel_x, pixel_y) * getScaleY();
    float b = getImageHeight(pixel_x, pixel_y + 1) * getScaleY();
    float c = getImageHeight(pixel_x + 1, pixel_y) * getScaleY();
    float d = getImageHeight(pixel_x + 1, pixel_y + 1) * getScaleY();
    Vec3 normal;
    normal.x = c - b;
    normal.y = 2;
    normal.z = d - a;
    normal.normalize();
    return normal;
}

cocos2d::Vec3 Terrain::getIntersectionPoint(const Ray& ray) const
{
    Vec3 collisionPoint;
    if (getIntersectionPoint(ray, collisionPoint))
    {
        return collisionPoint;
    }
    else
    {
        return Vec3(0, 0, 0);
    }
}

bool Terrain::getIntersectionPoint(const Ray& ray_, Vec3& intersectionPoint) const
{
    // convert ray from world space to local space
    Ray ray(ray_);
    getWorldToNodeTransform().transformPoint(&(ray._origin));

    std::set<Chunk*> closeList;
    Vec2 start = Vec2(ray_._origin.x, ray_._origin.z);
    Vec2 dir   = Vec2(ray._direction.x, ray._direction.z);
    start      = convertToTerrainSpace(start);
    start.x /= (_terrainData._chunkSize.width + 1);
    start.y /= (_terrainData._chunkSize.height + 1);
    Vec2 delta             = dir.getNormalized();
    auto width             = float(_imageWidth) / (_terrainData._chunkSize.width + 1);
    auto height            = float(_imageHeight) / (_terrainData._chunkSize.height + 1);
    bool hasIntersect      = false;
    float intersectionDist = FLT_MAX;
    Vec3 tmpIntersectionPoint;
    for (;;)
    {
        int x1 = floorf(start.x);
        int x2 = ceilf(start.x);
        int y1 = floorf(start.y);
        int y2 = ceilf(start.y);
        for (int x = x1; x <= x2; x++)
        {
            for (int y = y1; y <= y2; y++)
            {
                auto chunk = getChunkByIndex(x, y);
                if (chunk)
                {
                    if (closeList.find(chunk) == closeList.end())
                    {
                        if (chunk->getIntersectPointWithRay(ray, tmpIntersectionPoint))
                        {
                            float dist = (ray._origin - tmpIntersectionPoint).length();
                            if (intersectionDist > dist)
                            {
                                hasIntersect      = true;
                                intersectionDist  = dist;
                                intersectionPoint = tmpIntersectionPoint;
                            }
                        }
                        closeList.insert(chunk);
                    }
                }
            }
        }
        if ((delta.x > 0 && start.x > width) || (delta.x < 0 && start.x < 0))
        {
            break;
        }
        if ((delta.y > 0 && start.y > height) || (delta.y < 0 && start.y < 0))
        {
            break;
        }
        start.x += delta.x;
        start.y += delta.y;
    }
    return hasIntersect;
}

void Terrain::setMaxDetailMapAmount(int max_value)
{
    _maxDetailMapValue = max_value;
}

cocos2d::Vec2 Terrain::convertToTerrainSpace(const Vec2& worldSpaceXZ) const
{
    Vec2 pos(worldSpaceXZ.x, worldSpaceXZ.y);

    // top-left
    Vec2 tl(-1 * _terrainData._mapScale * _imageWidth / 2, -1 * _terrainData._mapScale * _imageHeight / 2);
    auto result = getNodeToWorldTransform() * Vec4(tl.x, 0.0f, tl.y, 1.0f);
    tl.set(result.x, result.z);

    Vec2 to_tl = pos - tl;

    // real size
    Vec2 size(_imageWidth * _terrainData._mapScale, _imageHeight * _terrainData._mapScale);
    result = getNodeToWorldTransform() * Vec4(size.x, 0.0f, size.y, 0.0f);
    size.set(result.x, result.z);

    float width_ratio  = to_tl.x / size.x;
    float height_ratio = to_tl.y / size.y;

    float image_x = width_ratio * _imageWidth;
    float image_y = height_ratio * _imageHeight;
    return Vec2(image_x, image_y);
}

void Terrain::resetHeightMap(std::string_view heightMap)
{
    _heightMapImage->release();
    _vertices.clear();
    free(_data);
    for (int i = 0; i < MAX_CHUNKES; ++i)
    {
        for (int j = 0; j < MAX_CHUNKES; j++)
        {
            if (_chunkesArray[i][j])
            {
                delete _chunkesArray[i][j];
            }
        }
    }
    delete _quadRoot;
    initHeightMap(heightMap);
}

float Terrain::getMinHeight()
{
    return _minHeight;
}

float Terrain::getMaxHeight()
{
    return _maxHeight;
}

cocos2d::AABB Terrain::getAABB()
{
    return _quadRoot->_worldSpaceAABB;
}

Terrain::QuadTree* Terrain::getQuadTree()
{
    return _quadRoot;
}

std::vector<float> Terrain::getHeightData() const
{
    std::vector<float> data;
    data.resize(_imageWidth * _imageHeight);
    for (int i = 0; i < _imageHeight; ++i)
    {
        for (int j = 0; j < _imageWidth; j++)
        {
            int idx   = i * _imageWidth + j;
            data[idx] = (_vertices[idx]._position.y);
        }
    }
    return data;
}

Terrain::Chunk* cocos2d::Terrain::getChunkByIndex(int x, int y) const
{
    if (x < 0 || y < 0 || x >= MAX_CHUNKES || y >= MAX_CHUNKES)
        return nullptr;
    return _chunkesArray[y][x];
}

void Terrain::setAlphaMap(cocos2d::Texture2D* newAlphaMapTexture)
{
    CC_SAFE_RETAIN(newAlphaMapTexture);
    CC_SAFE_RELEASE(_alphaMap);
    _alphaMap = newAlphaMapTexture;
}

void Terrain::setDetailMap(unsigned int index, DetailMap detailMap)
{
    if (index > 4)
    {
        CCLOG("invalid DetailMap index %d\n", index);
    }
    _terrainData._detailMaps[index] = detailMap;
    if (_detailMapTextures[index])
    {

        _detailMapTextures[index]->release();
    }
    _detailMapTextures[index] = new Texture2D();
    auto textImage            = new Image();
    textImage->initWithImageFile(detailMap._detailMapSrc);
    _detailMapTextures[index]->initWithImage(textImage);
    delete textImage;
}

Terrain::ChunkIndices Terrain::lookForIndicesLOD(int neighborLod[4], int selfLod, bool* result)
{
    (*result) = false;
    ChunkIndices tmp;
    tmp._indexBuffer = nullptr;
    tmp._size        = 0;
    if (_chunkLodIndicesSet.empty())
    {
        (*result) = false;
        return tmp;
    }
    else
    {
        int test[5];
        memcpy(test, neighborLod, sizeof(int[4]));
        test[4] = selfLod;
        for (size_t i = 0, size = _chunkLodIndicesSet.size(); i < size; ++i)
        {
            if (memcmp(test, _chunkLodIndicesSet[i]._relativeLod, sizeof(test)) == 0)
            {
                (*result) = true;
                return _chunkLodIndicesSet[i]._chunkIndices;
            }
        }
    }
    (*result) = false;
    return tmp;
}

Terrain::ChunkIndices Terrain::insertIndicesLOD(int neighborLod[4], int selfLod, uint16_t* indices, int size)
{
    ChunkLODIndices lodIndices;
    memcpy(lodIndices._relativeLod, neighborLod, sizeof(int[4]));
    lodIndices._relativeLod[4]     = selfLod;
    lodIndices._chunkIndices._size = size;

    auto buffer = backend::Device::getInstance()->newBuffer(sizeof(uint16_t) * size, backend::BufferType::INDEX,
                                                            backend::BufferUsage::STATIC);
    buffer->updateData(indices, sizeof(uint16_t) * size);

    CC_SAFE_RELEASE_NULL(lodIndices._chunkIndices._indexBuffer);
    lodIndices._chunkIndices._indexBuffer = buffer;
    this->_chunkLodIndicesSet.push_back(lodIndices);
    return lodIndices._chunkIndices;
}

Terrain::ChunkIndices Terrain::lookForIndicesLODSkrit(int selfLod, bool* result)
{
    ChunkIndices badResult;
    badResult._indexBuffer = nullptr;
    badResult._size        = 0;
    if (this->_chunkLodIndicesSkirtSet.empty())
    {
        (*result) = false;
        return badResult;
    }
    for (size_t i = 0, size = _chunkLodIndicesSkirtSet.size(); i < size; ++i)
    {
        if (_chunkLodIndicesSkirtSet[i]._selfLod == selfLod)
        {
            (*result) = true;
            return _chunkLodIndicesSkirtSet[i]._chunkIndices;
        }
    }
    (*result) = false;
    return badResult;
}

Terrain::ChunkIndices Terrain::insertIndicesLODSkirt(int selfLod, uint16_t* indices, int size)
{
    ChunkLODIndicesSkirt skirtIndices;
    skirtIndices._selfLod            = selfLod;
    skirtIndices._chunkIndices._size = size;

    auto buffer = backend::Device::getInstance()->newBuffer(sizeof(uint16_t) * size, backend::BufferType::INDEX,
                                                            backend::BufferUsage::STATIC);
    buffer->updateData(indices, sizeof(uint16_t) * size);

    CC_SAFE_RELEASE_NULL(skirtIndices._chunkIndices._indexBuffer);
    skirtIndices._chunkIndices._indexBuffer = buffer;
    this->_chunkLodIndicesSkirtSet.push_back(skirtIndices);
    return skirtIndices._chunkIndices;
}

void Terrain::setSkirtHeightRatio(float ratio)
{
    _skirtRatio = ratio;
}

void Terrain::onEnter()
{
    Node::onEnter();
    _terrainModelMatrix = getNodeToWorldTransform();
    _quadRoot->preCalculateAABB(_terrainModelMatrix);
    cacheUniformAttribLocation();
}

void Terrain::cacheUniformAttribLocation()
{
    auto vertexLayout         = _programState->getVertexLayout();
    const auto& attributeInfo = _programState->getProgram()->getActiveAttributes();
    auto iter                 = attributeInfo.find("a_position");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_position", iter->second.location, backend::VertexFormat::FLOAT3, 0, false);
    }
    iter = attributeInfo.find("a_texCoord");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_texCoord", iter->second.location, backend::VertexFormat::FLOAT2,
                                   offsetof(TerrainVertexData, _texcoord), false);
    }
    iter = attributeInfo.find("a_normal");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_normal", iter->second.location, backend::VertexFormat::FLOAT3,
                                   offsetof(TerrainVertexData, _normal), false);
    }
    vertexLayout->setLayout(sizeof(TerrainVertexData));

    _alphaMapLocation.reset();
    for (int i = 0; i < 4; ++i)
    {
        _detailMapLocation[i].reset();
    }
    _detailMapSizeLocation.reset();

    _alphaIsHasAlphaMapLocation = _programState->getUniformLocation("u_has_alpha");
    _lightMapCheckLocation      = _programState->getUniformLocation("u_has_light_map");
    if (!_alphaMap)
    {
        _detailMapLocation[0] = _programState->getUniformLocation("u_texture0");
    }
    else
    {
        for (int i = 0; i < _maxDetailMapValue; ++i)
        {
            char str[20];
            sprintf(str, "u_texture%d", i);
            _detailMapLocation[i] = _programState->getUniformLocation(str);
        }

        _detailMapSizeLocation = _programState->getUniformLocation("u_detailSize");  // float[4]

        _alphaMapLocation = _programState->getUniformLocation("u_alphaMap");
    }
    _lightMapLocation  = _programState->getUniformLocation("u_lightMap");
    _lightDirLocation  = _programState->getUniformLocation("u_lightDir");
    _mvpMatrixLocation = _programState->getUniformLocation("u_MVPMatrix");
}

bool Terrain::initTextures()
{
    for (int i = 0; i < 4; ++i)
    {
        _detailMapTextures[i] = nullptr;
    }

    Texture2D::TexParams texParam;
    texParam.sAddressMode = backend::SamplerAddressMode::REPEAT;
    texParam.tAddressMode = backend::SamplerAddressMode::REPEAT;
    if (_terrainData._alphaMapSrc.empty())
    {
        auto textImage = new Image();
        textImage->initWithImageFile(_terrainData._detailMaps[0]._detailMapSrc);
        auto texture = new Texture2D();
        texture->initWithImage(textImage);
        texture->generateMipmap();
        _detailMapTextures[0] = texture;
        texParam.minFilter    = backend::SamplerFilter::LINEAR;
        texParam.magFilter    = backend::SamplerFilter::LINEAR;
        texture->setTexParameters(texParam);
        delete textImage;
    }
    else
    {
        // alpha map
        auto image = new Image();
        image->initWithImageFile(_terrainData._alphaMapSrc);
        _alphaMap = new Texture2D();
        _alphaMap->initWithImage(image);
        texParam.sAddressMode = backend::SamplerAddressMode::CLAMP_TO_EDGE;
        texParam.tAddressMode = backend::SamplerAddressMode::CLAMP_TO_EDGE;
        texParam.minFilter    = backend::SamplerFilter::LINEAR;
        texParam.magFilter    = backend::SamplerFilter::LINEAR;
        _alphaMap->setTexParameters(texParam);
        delete image;

        for (int i = 0; i < _terrainData._detailMapAmount; ++i)
        {
            auto textImage = new Image();
            textImage->initWithImageFile(_terrainData._detailMaps[i]._detailMapSrc);
            auto texture = new Texture2D();
            texture->initWithImage(textImage);
            delete textImage;
            texture->generateMipmap();
            _detailMapTextures[i] = texture;

            texParam.sAddressMode = backend::SamplerAddressMode::REPEAT;
            texParam.tAddressMode = backend::SamplerAddressMode::REPEAT;
            texParam.minFilter    = backend::SamplerFilter::LINEAR;
            texParam.magFilter    = backend::SamplerFilter::LINEAR;
            texture->setTexParameters(texParam);
        }
    }
    setMaxDetailMapAmount(_terrainData._detailMapAmount);
    return true;
}

void Terrain::reload()
{
    int chunk_amount_y = _imageHeight / _chunkSize.height;
    int chunk_amount_x = _imageWidth / _chunkSize.width;

    for (int m = 0; m < chunk_amount_y; m++)
    {
        for (int n = 0; n < chunk_amount_x; n++)
        {
            _chunkesArray[m][n]->finish();
        }
    }

    initTextures();
    _chunkLodIndicesSet.clear();
    _chunkLodIndicesSkirtSet.clear();
}

void Terrain::Chunk::finish()
{
    // generate two VBO ,the first for vertices, we just setup datas once ,won't changed at all
    // the second vbo for the indices, because we use level of detail technique to each chunk, so we will modified
    // frequently

    CC_SAFE_RELEASE_NULL(_buffer);
    _buffer = backend::Device::getInstance()->newBuffer(sizeof(TerrainVertexData) * _originalVertices.size(),
                                                        backend::BufferType::VERTEX, backend::BufferUsage::DYNAMIC);

    _buffer->updateData(&_originalVertices[0], sizeof(TerrainVertexData) * _originalVertices.size());

    calculateSlope();

    for (int i = 0; i < 4; ++i)
    {
        int step = 1 << _currentLod;
        // reserve the indices size, the first part is the core part of the chunk, the second part & third part is for
        // fix crack
        int indicesAmount = (_terrain->_chunkSize.width / step + 1) * (_terrain->_chunkSize.height / step + 1) * 6 +
                            (_terrain->_chunkSize.height / step) * 6 + (_terrain->_chunkSize.width / step) * 6;
        _lod[i]._indices.reserve(indicesAmount);
    }
    _oldLod = -1;
}

void Terrain::Chunk::bindAndDraw()
{
    if (_terrain->_isCameraViewChanged || _oldLod < 0)
    {
        switch (_terrain->_crackFixedType)
        {
        case CrackFixedType::SKIRT:

            updateIndicesLODSkirt();
            break;
        case CrackFixedType::INCREASE_LOWER:
            updateVerticesForLOD();
            updateIndicesLOD();
            break;
        default:
            break;
        }
    }

    auto* renderer = Director::getInstance()->getRenderer();
    CCASSERT(_buffer && _chunkIndices._indexBuffer, "buffer should not be nullptr");
    _command.setIndexBuffer(_chunkIndices._indexBuffer, backend::IndexFormat::U_SHORT);
    _command.setVertexBuffer(_buffer);
    _command.getPipelineDescriptor().programState = _terrain->_programState;
    _command.setIndexDrawInfo(0, _chunkIndices._size);
    renderer->addCommand(&_command);
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _chunkIndices._size);
}

void Terrain::Chunk::generate(int imgWidth, int imageHei, int m, int n, const unsigned char* /*data*/)
{
    _posY = m;
    _posX = n;
    switch (_terrain->_crackFixedType)
    {
    case CrackFixedType::SKIRT:
    {
        for (int i = _size.height * m; i <= _size.height * (m + 1); ++i)
        {
            if (i >= imageHei)
                break;
            for (int j = _size.width * n; j <= _size.width * (n + 1); j++)
            {
                if (j >= imgWidth)
                    break;
                auto v = _terrain->_vertices[i * imgWidth + j];
                _originalVertices.push_back(v);
            }
        }
        // add four skirts

        float skirtHeight = _terrain->_skirtRatio * _terrain->_terrainData._mapScale * 8;
        //#1
        _terrain->_skirtVerticesOffset[0] = (int)_originalVertices.size();
        for (int i = _size.height * m; i <= _size.height * (m + 1); ++i)
        {
            auto v = _terrain->_vertices[i * imgWidth + _size.width * (n + 1)];
            v._position.y -= skirtHeight;
            _originalVertices.push_back(v);
        }

        //#2
        _terrain->_skirtVerticesOffset[1] = (int)_originalVertices.size();
        for (int j = _size.width * n; j <= _size.width * (n + 1); j++)
        {
            auto v = _terrain->_vertices[_size.height * (m + 1) * imgWidth + j];
            v._position.y -= skirtHeight;
            _originalVertices.push_back(v);
        }

        //#3
        _terrain->_skirtVerticesOffset[2] = (int)_originalVertices.size();
        for (int i = _size.height * m; i <= _size.height * (m + 1); ++i)
        {
            auto v = _terrain->_vertices[i * imgWidth + _size.width * n];
            v._position.y -= skirtHeight;
            _originalVertices.push_back(v);
        }

        //#4
        _terrain->_skirtVerticesOffset[3] = (int)_originalVertices.size();
        for (int j = _size.width * n; j <= _size.width * (n + 1); j++)
        {
            auto v = _terrain->_vertices[_size.height * m * imgWidth + j];
            v._position.y -= skirtHeight;
            // v.position.y = -5;
            _originalVertices.push_back(v);
        }
    }
    break;
    case CrackFixedType::INCREASE_LOWER:
    {
        for (int i = _size.height * m; i <= _size.height * (m + 1); ++i)
        {
            if (i >= imageHei)
                break;
            for (int j = _size.width * n; j <= _size.width * (n + 1); j++)
            {
                if (j >= imgWidth)
                    break;
                auto v = _terrain->_vertices[i * imgWidth + j];
                _originalVertices.push_back(v);
            }
        }
    }
    break;
    }
    // store triangle:
    for (int i = 0; i < _size.height; ++i)
    {
        for (int j = 0; j < _size.width; j++)
        {
            int nLocIndex = i * (_size.width + 1) + j;
            Triangle a(_originalVertices[nLocIndex]._position,
                       _originalVertices[nLocIndex + 1 * (_size.width + 1)]._position,
                       _originalVertices[nLocIndex + 1]._position);
            Triangle b(_originalVertices[nLocIndex + 1]._position,
                       _originalVertices[nLocIndex + 1 * (_size.width + 1)]._position,
                       _originalVertices[nLocIndex + 1 * (_size.width + 1) + 1]._position);

            _trianglesList.push_back(a);
            _trianglesList.push_back(b);
        }
    }

    calculateAABB();
    finish();
}

Terrain::Chunk::Chunk(Terrain* terrain)
{
    _terrain    = terrain;
    _currentLod = 0;
    _left       = nullptr;
    _right      = nullptr;
    _back       = nullptr;
    _front      = nullptr;
    _oldLod     = -1;
    for (int i = 0; i < 4; ++i)
    {
        _neighborOldLOD[i] = -1;
    }
    _command.init(_terrain->_globalZOrder);
    _command.setTransparent(false);
    _command.set3D(true);
    _command.setPrimitiveType(MeshCommand::PrimitiveType::TRIANGLE);
    _command.setDrawType(MeshCommand::DrawType::ELEMENT);

    _command.setBeforeCallback(CC_CALLBACK_0(Terrain::onBeforeDraw, terrain));
    _command.setAfterCallback(CC_CALLBACK_0(Terrain::onAfterDraw, terrain));

    auto& pipelineDescriptor                        = _command.getPipelineDescriptor();
    pipelineDescriptor.blendDescriptor.blendEnabled = false;
}

void Terrain::Chunk::updateIndicesLOD()
{
    int currentNeighborLOD[4];
    if (_left)
    {
        currentNeighborLOD[0] = _left->_currentLod;
    }
    else
    {
        currentNeighborLOD[0] = -1;
    }
    if (_right)
    {
        currentNeighborLOD[1] = _right->_currentLod;
    }
    else
    {
        currentNeighborLOD[1] = -1;
    }
    if (_back)
    {
        currentNeighborLOD[2] = _back->_currentLod;
    }
    else
    {
        currentNeighborLOD[2] = -1;
    }
    if (_front)
    {
        currentNeighborLOD[3] = _front->_currentLod;
    }
    else
    {
        currentNeighborLOD[3] = -1;
    }

    if (_oldLod == _currentLod && (memcmp(currentNeighborLOD, _neighborOldLOD, sizeof(currentNeighborLOD)) == 0))
    {
        return;  // no need to update
    }
    bool isOk;
    _chunkIndices = _terrain->lookForIndicesLOD(currentNeighborLOD, _currentLod, &isOk);
    if (isOk)
    {
        return;
    }
    memcpy(_neighborOldLOD, currentNeighborLOD, sizeof(currentNeighborLOD));
    _oldLod   = _currentLod;
    int gridY = static_cast<int>(_size.height);
    int gridX = static_cast<int>(_size.width);

    int step = 1 << _currentLod;
    if ((_left && _left->_currentLod > _currentLod) || (_right && _right->_currentLod > _currentLod) ||
        (_back && _back->_currentLod > _currentLod) || (_front && _front->_currentLod > _currentLod))
    // need update indices.
    {
        // t-junction inner
        _lod[_currentLod]._indices.clear();
        for (int i = step; i < gridY - step; i += step)
        {
            for (int j = step; j < gridX - step; j += step)
            {
                int nLocIndex = i * (gridX + 1) + j;
                _lod[_currentLod]._indices.push_back(nLocIndex);
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
                _lod[_currentLod]._indices.push_back(nLocIndex + step);

                _lod[_currentLod]._indices.push_back(nLocIndex + step);
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1) + step);
            }
        }
        // fix T-crack
        int next_step = 1 << (_currentLod + 1);
        if (_left && _left->_currentLod > _currentLod)  // left
        {
            for (int i = 0; i < gridY; i += next_step)
            {
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + step);
                _lod[_currentLod]._indices.push_back(i * (gridX + 1));
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1));

                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + step);
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1));
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + step);

                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + step);
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1));
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1) + step);
            }
        }
        else
        {
            int start = 0;
            int end   = gridY;
            if (_front && _front->_currentLod > _currentLod)
                end -= step;
            if (_back && _back->_currentLod > _currentLod)
                start += step;
            for (int i = start; i < end; i += step)
            {
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + step);
                _lod[_currentLod]._indices.push_back(i * (gridX + 1));
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1));

                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + step);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1));
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + step);
            }
        }

        if (_right && _right->_currentLod > _currentLod)  // LEFT
        {
            for (int i = 0; i < gridY; i += next_step)
            {
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX);
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX - step);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + gridX - step);

                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + gridX - step);
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1) + gridX - step);

                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX);
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1) + gridX - step);
                _lod[_currentLod]._indices.push_back((i + next_step) * (gridX + 1) + gridX);
            }
        }
        else
        {
            int start = 0;
            int end   = gridY;
            if (_front && _front->_currentLod > _currentLod)
                end -= step;
            if (_back && _back->_currentLod > _currentLod)
                start += step;
            for (int i = start; i < end; i += step)
            {
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX);
                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX - step);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + gridX - step);

                _lod[_currentLod]._indices.push_back(i * (gridX + 1) + gridX);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + gridX - step);
                _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1) + gridX);
            }
        }
        if (_front && _front->_currentLod > _currentLod)  // front
        {
            for (int i = 0; i < gridX; i += next_step)
            {
                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + step);

                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i + next_step);

                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i + next_step);
                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + next_step);
            }
        }
        else
        {
            for (int i = step; i < gridX - step; i += step)
            {
                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + step);

                _lod[_currentLod]._indices.push_back((gridY - step) * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(gridY * (gridX + 1) + i + step);
            }
        }
        if (_back && _back->_currentLod > _currentLod)  // back
        {
            for (int i = 0; i < gridX; i += next_step)
            {
                _lod[_currentLod]._indices.push_back(i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + step);

                _lod[_currentLod]._indices.push_back(i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(i + next_step);

                _lod[_currentLod]._indices.push_back(i + next_step);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + next_step);
            }
        }
        else
        {
            for (int i = step; i < gridX - step; i += step)
            {
                _lod[_currentLod]._indices.push_back(i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + step);

                _lod[_currentLod]._indices.push_back(i);
                _lod[_currentLod]._indices.push_back(step * (gridX + 1) + i + step);
                _lod[_currentLod]._indices.push_back(i + step);
            }
        }

        _chunkIndices = _terrain->insertIndicesLOD(currentNeighborLOD, _currentLod, &_lod[_currentLod]._indices[0],
                                                   (int)_lod[_currentLod]._indices.size());
    }
    else
    {
        // No lod difference, use simple method
        _lod[_currentLod]._indices.clear();
        for (int i = 0; i < gridY; i += step)
        {
            for (int j = 0; j < gridX; j += step)
            {

                int nLocIndex = i * (gridX + 1) + j;
                _lod[_currentLod]._indices.push_back(nLocIndex);
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
                _lod[_currentLod]._indices.push_back(nLocIndex + step);

                _lod[_currentLod]._indices.push_back(nLocIndex + step);
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
                _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1) + step);
            }
        }
        _chunkIndices = _terrain->insertIndicesLOD(currentNeighborLOD, _currentLod, &_lod[_currentLod]._indices[0],
                                                   (int)_lod[_currentLod]._indices.size());
    }
}

void Terrain::Chunk::calculateAABB()
{
    std::vector<Vec3> pos;
    for (size_t i = 0, size = _originalVertices.size(); i < size; ++i)
    {
        pos.push_back(_originalVertices[i]._position);
    }
    _aabb.updateMinMax(&pos[0], pos.size());
}

void Terrain::Chunk::calculateSlope()
{
    // find max slope
    auto lowest = _originalVertices[0]._position;
    for (size_t i = 0, size = _originalVertices.size(); i < size; ++i)
    {
        if (_originalVertices[i]._position.y < lowest.y)
        {
            lowest = _originalVertices[i]._position;
        }
    }
    auto highest = _originalVertices[0]._position;
    for (size_t i = 0, size = _originalVertices.size(); i < size; ++i)
    {
        if (_originalVertices[i]._position.y > highest.y)
        {
            highest = _originalVertices[i]._position;
        }
    }
    Vec2 a(lowest.x, lowest.z);
    Vec2 b(highest.x, highest.z);
    float dist = a.distance(b);
    _slope     = (highest.y - lowest.y) / dist;
}

bool Terrain::Chunk::getIntersectPointWithRay(const Ray& ray, Vec3& intersectPoint)
{
    if (!ray.intersects(_aabb))
        return false;

    float minDist = FLT_MAX;
    bool isFind   = false;
    for (const auto& triangle : _trianglesList)
    {
        Vec3 p;
        if (triangle.getIntersectPoint(ray, p))
        {
            float dist = ray._origin.distance(p);
            if (dist < minDist)
            {
                intersectPoint = p;
                minDist        = dist;
            }
            isFind = true;
        }
    }

    return isFind;
}

void Terrain::Chunk::updateVerticesForLOD()
{
    if (_oldLod == _currentLod)
    {
        return;
    }  // no need to update vertices
    _currentVertices = _originalVertices;
    int gridY        = _size.height;
    int gridX        = _size.width;

    if (_currentLod >= 2 && std::abs(_slope) > 1.2f)
    {
        int step = 1 << _currentLod;
        for (int i = step; i < gridY - step; i += step)
            for (int j = step; j < gridX - step; j += step)
            {
                // use linear-sample adjust vertices height
                float height = 0;
                float count  = 0;
                for (int n = i - step / 2; n < i + step / 2; n++)
                {
                    for (int m = j - step / 2; m < j + step / 2; m++)
                    {
                        float weight = (step / 2 - std::abs(n - i)) * (step / 2 - std::abs(m - j));
                        height += _originalVertices[m * (gridX + 1) + n]._position.y;
                        count += weight;
                    }
                }
                _currentVertices[i * (gridX + 1) + j]._position.y = height / count;
            }
    }

    _oldLod = _currentLod;
}

Terrain::Chunk::~Chunk()
{
    CC_SAFE_RELEASE_NULL(_buffer);
}

void Terrain::Chunk::updateIndicesLODSkirt()
{
    if (_oldLod == _currentLod)
        return;
    _oldLod = _currentLod;
    bool isOk;
    _chunkIndices = _terrain->lookForIndicesLODSkrit(_currentLod, &isOk);
    if (isOk)
        return;

    int gridY = _size.height;
    int gridX = _size.width;
    int step  = 1 << _currentLod;
    int k     = 0;
    for (int i = 0; i < gridY; i += step, k += step)
    {
        for (int j = 0; j < gridX; j += step)
        {
            int nLocIndex = i * (gridX + 1) + j;
            _lod[_currentLod]._indices.push_back(nLocIndex);
            _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
            _lod[_currentLod]._indices.push_back(nLocIndex + step);

            _lod[_currentLod]._indices.push_back(nLocIndex + step);
            _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
            _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1) + step);
        }
    }
    // add skirt
    //#1
    for (int i = 0; i < gridY; i += step)
    {
        int nLocIndex = i * (gridX + 1) + gridX;
        _lod[_currentLod]._indices.push_back(nLocIndex);
        _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
        _lod[_currentLod]._indices.push_back((gridY + 1) * (gridX + 1) + i);

        _lod[_currentLod]._indices.push_back((gridY + 1) * (gridX + 1) + i);
        _lod[_currentLod]._indices.push_back(nLocIndex + step * (gridX + 1));
        _lod[_currentLod]._indices.push_back((gridY + 1) * (gridX + 1) + i + step);
    }

    //#2
    for (int j = 0; j < gridX; j += step)
    {
        int nLocIndex = (gridY) * (gridX + 1) + j;
        _lod[_currentLod]._indices.push_back(nLocIndex);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[1] + j);
        _lod[_currentLod]._indices.push_back(nLocIndex + step);

        _lod[_currentLod]._indices.push_back(nLocIndex + step);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[1] + j);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[1] + j + step);
    }

    //#3
    for (int i = 0; i < gridY; i += step)
    {
        int nLocIndex = i * (gridX + 1);
        _lod[_currentLod]._indices.push_back(nLocIndex);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[2] + i);
        _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1));

        _lod[_currentLod]._indices.push_back((i + step) * (gridX + 1));
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[2] + i);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[2] + i + step);
    }

    //#4
    for (int j = 0; j < gridX; j += step)
    {
        int nLocIndex = j;
        _lod[_currentLod]._indices.push_back(nLocIndex + step);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[3] + j);
        _lod[_currentLod]._indices.push_back(nLocIndex);

        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[3] + j + step);
        _lod[_currentLod]._indices.push_back(_terrain->_skirtVerticesOffset[3] + j);
        _lod[_currentLod]._indices.push_back(nLocIndex + step);
    }

    _chunkIndices = _terrain->insertIndicesLODSkirt(_currentLod, &_lod[_currentLod]._indices[0],
                                                    (int)_lod[_currentLod]._indices.size());
}

Terrain::QuadTree::QuadTree(int x, int y, int w, int h, Terrain* terrain)
{
    _terrain      = terrain;
    _needDraw     = true;
    _parent       = nullptr;
    _tl           = nullptr;
    _tr           = nullptr;
    _bl           = nullptr;
    _br           = nullptr;
    _posX         = x;
    _posY         = y;
    this->_height = h;
    this->_width  = w;
    if (_width > terrain->_chunkSize.width && _height > terrain->_chunkSize.height)  // subdivision
    {
        _isTerminal        = false;
        this->_tl          = new QuadTree(x, y, _width / 2, _height / 2, terrain);
        this->_tl->_parent = this;
        this->_tr          = new QuadTree(x + _width / 2, y, _width / 2, _height / 2, terrain);
        this->_tr->_parent = this;
        this->_bl          = new QuadTree(x, y + _height / 2, _width / 2, _height / 2, terrain);
        this->_bl->_parent = this;
        this->_br          = new QuadTree(x + _width / 2, y + _height / 2, _width / 2, _height / 2, terrain);
        this->_br->_parent = this;

        _localAABB.merge(_tl->_localAABB);
        _localAABB.merge(_tr->_localAABB);
        _localAABB.merge(_bl->_localAABB);
        _localAABB.merge(_br->_localAABB);
    }
    else  // is terminal Node
    {
        int m           = _posY / terrain->_chunkSize.height;
        int n           = _posX / terrain->_chunkSize.width;
        _chunk          = terrain->_chunkesArray[m][n];
        _isTerminal     = true;
        _localAABB      = _chunk->_aabb;
        _chunk->_parent = this;

        for (auto& triangle : _chunk->_trianglesList)
        {
            triangle.transform(_terrain->getNodeToWorldTransform());
        }
    }
    _worldSpaceAABB = _localAABB;
    _worldSpaceAABB.transform(_terrain->getNodeToWorldTransform());
}

void Terrain::QuadTree::draw()
{
    if (!_needDraw)
        return;
    if (_isTerminal)
    {
        this->_chunk->bindAndDraw();
    }
    else
    {
        this->_tl->draw();
        this->_tr->draw();
        this->_br->draw();
        this->_bl->draw();
    }
}

void Terrain::QuadTree::resetNeedDraw(bool value)
{
    this->_needDraw = value;
    if (!_isTerminal)
    {
        _tl->resetNeedDraw(value);
        _tr->resetNeedDraw(value);
        _bl->resetNeedDraw(value);
        _br->resetNeedDraw(value);
    }
}

void Terrain::QuadTree::cullByCamera(const Camera* camera, const Mat4& worldTransform)
{
    if (!camera->isVisibleInFrustum(&_worldSpaceAABB))
    {
        this->resetNeedDraw(false);
    }
    else if (!_isTerminal)
    {
        _tl->cullByCamera(camera, worldTransform);
        _tr->cullByCamera(camera, worldTransform);
        _bl->cullByCamera(camera, worldTransform);
        _br->cullByCamera(camera, worldTransform);
    }
}

void Terrain::QuadTree::preCalculateAABB(const Mat4& worldTransform)
{

    _worldSpaceAABB = _localAABB;
    _worldSpaceAABB.transform(worldTransform);
    if (!_isTerminal)
    {
        _tl->preCalculateAABB(worldTransform);
        _tr->preCalculateAABB(worldTransform);
        _bl->preCalculateAABB(worldTransform);
        _br->preCalculateAABB(worldTransform);
    }
}

Terrain::QuadTree::~QuadTree()
{
    if (_tl)
        delete _tl;
    if (_tr)
        delete _tr;
    if (_bl)
        delete _bl;
    if (_br)
        delete _br;
}

Terrain::TerrainData::TerrainData(std::string_view heightMapsrc,
                                  std::string_view textureSrc,
                                  const Vec2& chunksize,
                                  float height,
                                  float scale)
{
    this->_heightMapSrc                = heightMapsrc;
    this->_detailMaps[0]._detailMapSrc = textureSrc;
    this->_alphaMapSrc                 = "";
    this->_chunkSize                   = chunksize;
    this->_mapHeight                   = height;
    this->_mapScale                    = scale;
    _skirtHeightRatio                  = 1;
}

Terrain::TerrainData::TerrainData(std::string_view heightMapsrc,
                                  std::string_view alphamap,
                                  const DetailMap& detail1,
                                  const DetailMap& detail2,
                                  const DetailMap& detail3,
                                  const DetailMap& detail4,
                                  const Vec2& chunksize,
                                  float height,
                                  float scale)
{
    this->_heightMapSrc  = heightMapsrc;
    this->_alphaMapSrc   = alphamap;
    this->_detailMaps[0] = detail1;
    this->_detailMaps[1] = detail2;
    this->_detailMaps[2] = detail3;
    this->_detailMaps[3] = detail4;
    this->_chunkSize     = chunksize;
    this->_mapHeight     = height;
    this->_mapScale      = scale;
    _detailMapAmount     = 4;
    _skirtHeightRatio    = 1;
}

Terrain::TerrainData::TerrainData(std::string_view heightMapsrc,
                                  std::string_view alphamap,
                                  const DetailMap& detail1,
                                  const DetailMap& detail2,
                                  const DetailMap& detail3,
                                  const Vec2& chunksize /*= Vec2(32,32)*/,
                                  float height /*= 2*/,
                                  float scale /*= 0.1*/)
{
    this->_heightMapSrc  = heightMapsrc;
    this->_alphaMapSrc   = alphamap;
    this->_detailMaps[0] = detail1;
    this->_detailMaps[1] = detail2;
    this->_detailMaps[2] = detail3;
    this->_chunkSize     = chunksize;
    this->_mapHeight     = height;
    this->_mapScale      = scale;
    _detailMapAmount     = 3;
    _skirtHeightRatio    = 1;
}

Terrain::TerrainData::TerrainData() {}

Terrain::ChunkIndices::ChunkIndices(const Terrain::ChunkIndices& other)
{
    _indexBuffer = other._indexBuffer;
    CC_SAFE_RETAIN(_indexBuffer);
    _size = other._size;
}

Terrain::ChunkIndices& Terrain::ChunkIndices::operator=(const Terrain::ChunkIndices& other)
{
    if (other._indexBuffer != _indexBuffer)
    {
        CC_SAFE_RELEASE_NULL(_indexBuffer);
        _indexBuffer = other._indexBuffer;
        CC_SAFE_RETAIN(_indexBuffer);
    }
    _size = other._size;
    return *this;
}

Terrain::ChunkIndices::~ChunkIndices()
{
    CC_SAFE_RELEASE_NULL(_indexBuffer);
}

Terrain::DetailMap::DetailMap(std::string_view detailMapPath, float size /*= 35*/)
{
    this->_detailMapSrc  = detailMapPath;
    this->_detailMapSize = size;
}

Terrain::DetailMap::DetailMap()
{
    _detailMapSrc  = "";
    _detailMapSize = 35;
}

Terrain::Triangle::Triangle(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
    _p1 = p1;
    _p2 = p2;
    _p3 = p3;
}

void Terrain::Triangle::transform(const cocos2d::Mat4& matrix)
{
    matrix.transformPoint(&_p1);
    matrix.transformPoint(&_p2);
    matrix.transformPoint(&_p3);
}

// Please refer to 3D Math Primer for Graphics and Game Development
bool Terrain::Triangle::getIntersectPoint(const Ray& ray, Vec3& intersectPoint) const
{
    // E1
    Vec3 E1 = _p2 - _p1;

    // E2
    Vec3 E2 = _p3 - _p1;

    // P
    Vec3 P;
    Vec3::cross(ray._direction, E2, &P);

    // determinant
    float det = E1.dot(P);

    // keep det > 0, modify T accordingly
    Vec3 T;
    if (det > 0)
    {
        T = ray._origin - _p1;
    }
    else
    {
        T   = _p1 - ray._origin;
        det = -det;
    }

    // If determinant is near zero, ray lies in plane of triangle
    if (det < 0.0001f)
        return false;

    float t;     // ray dist
    float u, v;  // barycentric coordinate
    // Calculate u and make sure u <= 1
    u = T.dot(P);
    if (u < 0.0f || u > det)
        return false;

    // Q
    Vec3 Q;
    Vec3::cross(T, E1, &Q);

    // Calculate v and make sure u + v <= 1
    v = ray._direction.dot(Q);
    if (v < 0.0f || u + v > det)
        return false;

    // Calculate t, scale parameters, ray intersects triangle
    t = E2.dot(Q);

    float fInvDet = 1.0f / det;
    t *= fInvDet;

    intersectPoint = ray._origin + ray._direction * t;
    return true;
}

void Terrain::onBeforeDraw()
{
    _stateBlockOld.save();
    _stateBlock.apply();
}

void Terrain::onAfterDraw()
{
    _stateBlockOld.apply();
}

void Terrain::StateBlock::save()
{
    auto renderer = Director::getInstance()->getRenderer();
    depthWrite    = renderer->getDepthWrite();
    depthTest     = renderer->getDepthTest();
    cullFace      = renderer->getCullMode();
    winding       = renderer->getWinding();
}

void Terrain::StateBlock::apply()
{
    auto renderer = Director::getInstance()->getRenderer();
    renderer->setDepthTest(depthTest);
    renderer->setDepthWrite(depthWrite);
    renderer->setCullMode(cullFace);
    renderer->setWinding(winding);
}

NS_CC_END
