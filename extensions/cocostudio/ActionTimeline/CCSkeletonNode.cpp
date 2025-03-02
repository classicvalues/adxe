/****************************************************************************
Copyright (c) 2015-2017 Chukong Technologies Inc.

http://www.cocos2d-x.org

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
#include "ActionTimeline/CCSkeletonNode.h"

#include "base/CCDirector.h"
#include "math/TransformUtils.h"
#include "renderer/CCRenderer.h"
#include "renderer/ccShaders.h"
#include "base/ccUtils.h"
#include <stack>

NS_TIMELINE_BEGIN

SkeletonNode* SkeletonNode::create()
{
    SkeletonNode* skeletonNode = new SkeletonNode();
    if (skeletonNode->init())
    {
        skeletonNode->autorelease();
        return skeletonNode;
    }
    CC_SAFE_DELETE(skeletonNode);
    return nullptr;
}

bool SkeletonNode::init()
{
    _rackLength = _rackWidth = 20;
    updateVertices();

    // init _customCommand
    auto& pipelineDescriptor = _customCommand.getPipelineDescriptor();
    auto* program =
        cocos2d::backend::Program::getBuiltinProgram(cocos2d::backend::ProgramType::POSITION_COLOR);  // TODO: noMVP?
    setProgramState(new cocos2d::backend::ProgramState(program), false);
    pipelineDescriptor.programState = _programState;

    _mvpLocation = _programState->getUniformLocation("u_MVPMatrix");

    auto vertexLayout         = _programState->getVertexLayout();
    const auto& attributeInfo = _programState->getProgram()->getActiveAttributes();
    auto iter                 = attributeInfo.find("a_position");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_position", iter->second.location, cocos2d::backend::VertexFormat::FLOAT3, 0,
                                   false);
    }
    iter = attributeInfo.find("a_color");
    if (iter != attributeInfo.end())
    {
        vertexLayout->setAttribute("a_color", iter->second.location, cocos2d::backend::VertexFormat::FLOAT4,
                                   3 * sizeof(float), false);
    }
    vertexLayout->setLayout(7 * sizeof(float));

    _customCommand.createVertexBuffer(sizeof(_vertexData[0]), 8, cocos2d::CustomCommand::BufferUsage::DYNAMIC);
    _customCommand.createIndexBuffer(cocos2d::CustomCommand::IndexFormat::U_SHORT, 12,
                                     cocos2d::CustomCommand::BufferUsage::STATIC);
    unsigned short indices[12] = {0, 1, 2, 1, 3, 2, 4, 5, 6, 5, 7, 6};
    _customCommand.updateIndexBuffer(indices, sizeof(indices));

    // init _batchBoneCommand
    _batchBoneCommand.getPipelineDescriptor().programState = _programState;

    _rootSkeleton = this;
    return true;
}

cocos2d::Rect SkeletonNode::getBoundingBox() const
{
    float minx, miny, maxx, maxy = 0;
    minx = miny = maxx        = maxy;
    cocos2d::Rect boundingBox = getVisibleSkinsRect();
    bool first                = true;
    if (!boundingBox.equals(cocos2d::Rect::ZERO))
    {
        minx  = boundingBox.getMinX();
        miny  = boundingBox.getMinY();
        maxx  = boundingBox.getMaxX();
        maxy  = boundingBox.getMaxY();
        first = false;
    }
    auto allbones = getAllSubBones();
    for (const auto& bone : allbones)
    {
        cocos2d::Rect r = RectApplyAffineTransform(bone->getVisibleSkinsRect(),
                                                   bone->getNodeToParentAffineTransform(bone->getRootSkeletonNode()));
        if (r.equals(cocos2d::Rect::ZERO))
            continue;

        if (first)
        {
            minx = r.getMinX();
            miny = r.getMinY();
            maxx = r.getMaxX();
            maxy = r.getMaxY();

            first = false;
        }
        else
        {
            minx = MIN(r.getMinX(), minx);
            miny = MIN(r.getMinY(), miny);
            maxx = MAX(r.getMaxX(), maxx);
            maxy = MAX(r.getMaxY(), maxy);
        }
    }
    boundingBox.setRect(minx, miny, maxx - minx, maxy - miny);
    return RectApplyAffineTransform(boundingBox, this->getNodeToParentAffineTransform());
}

SkeletonNode::SkeletonNode() : BoneNode(), _subBonesDirty(true), _subBonesOrderDirty(true), _batchedVeticesCount(0) {}

SkeletonNode::~SkeletonNode()
{
    for (auto& bonepair : _subBonesMap)
    {
        setRootSkeleton(bonepair.second, nullptr);
    }
}

void SkeletonNode::updateVertices()
{
    if (_rackLength != _squareVertices[6].x - _anchorPointInPoints.x ||
        _rackWidth != _squareVertices[3].y - _anchorPointInPoints.y)
    {
        const float radiusl   = _rackLength * .5f;
        const float radiusw   = _rackWidth * .5f;
        const float radiusl_2 = radiusl * .25f;
        const float radiusw_2 = radiusw * .25f;
        _squareVertices[5].y = _squareVertices[2].y = _squareVertices[1].y = _squareVertices[6].y =
            _squareVertices[0].x = _squareVertices[4].x = _squareVertices[7].x = _squareVertices[3].x = .0f;
        _squareVertices[5].x                                                                          = -radiusl;
        _squareVertices[0].y                                                                          = -radiusw;
        _squareVertices[6].x                                                                          = radiusl;
        _squareVertices[3].y                                                                          = radiusw;
        _squareVertices[1].x                                                                          = radiusl_2;
        _squareVertices[7].y                                                                          = radiusw_2;
        _squareVertices[2].x                                                                          = -radiusl_2;
        _squareVertices[4].y                                                                          = -radiusw_2;

        for (int i = 0; i < 8; i++)
        {
            _squareVertices[i] += _anchorPointInPoints;
        }

        _transformUpdated = _transformDirty = _inverseDirty = _contentSizeDirty = true;
    }
}

void SkeletonNode::updateColor()
{
    for (unsigned int i = 0; i < 8; i++)
    {
        _vertexData[i].color = _rackColor;
    }
    _transformUpdated = _transformDirty = _inverseDirty = _contentSizeDirty = true;
}

void SkeletonNode::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
    // quick return if not visible. children won't be drawn.
    if (!_visible)
    {
        return;
    }

    uint32_t flags = processParentFlags(parentTransform, parentFlags);

    // IMPORTANT:
    // To ease the migration to v3.0, we still support the Mat4 stack,
    // but it is deprecated and your code should not rely on it
    _director->pushMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    _director->loadMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

    int i = 0;
    if (!_children.empty())
    {
        sortAllChildren();
        // draw children zOrder < 0
        for (; i < _children.size(); i++)
        {
            auto node = _children.at(i);

            if (node && node->getLocalZOrder() < 0)
                node->visit(renderer, _modelViewTransform, flags);
            else
                break;
        }

        for (auto it = _children.cbegin() + i; it != _children.cend(); ++it)
            (*it)->visit(renderer, _modelViewTransform, flags);
    }

    checkSubBonesDirty();
    for (const auto& bone : _subOrderedAllBones)
    {
        visitSkins(renderer, bone);
    }

    if (_isRackShow)
    {
        this->draw(renderer, _modelViewTransform, flags);
        // batch draw all sub bones
        _batchBoneCommand.init(_globalZOrder, _blendFunc);
        renderer->addCommand(&_batchBoneCommand);
        batchDrawAllSubBones();
    }
    _director->popMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    // FIX ME: Why need to set _orderOfArrival to 0??
    // Please refer to https://github.com/cocos2d/cocos2d-x/pull/6920
    // reset for next frame
    // _orderOfArrival = 0;
}

void SkeletonNode::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
    _customCommand.init(_globalZOrder, _blendFunc);
    renderer->addCommand(&_customCommand);
    for (int i = 0; i < 8; ++i)
    {
        cocos2d::Vec4 pos;
        pos.x = _squareVertices[i].x;
        pos.y = _squareVertices[i].y;
        pos.z = _positionZ;
        pos.w = 1;
        _modelViewTransform.transformVector(&pos);
        _vertexData[i].vertex = cocos2d::Vec3(pos.x, pos.y, pos.z) / pos.w;
    }
    _customCommand.updateVertexBuffer(_vertexData, sizeof(_vertexData));

    // custom command and batched draw command share the same program state, so just set one time.
    _programState->setUniform(_mvpLocation, transform.m, sizeof(transform.m));
}

void SkeletonNode::batchDrawAllSubBones()
{
    checkSubBonesDirty();

    _batchedVeticesCount = 0;
    for (const auto& bone : _subOrderedAllBones)
    {
        batchBoneDrawToSkeleton(bone);
    }

    _batchBoneCommand.createVertexBuffer(sizeof(VertexData), _batchedVeticesCount,
                                         cocos2d::CustomCommand::BufferUsage::DYNAMIC);
    _batchBoneCommand.updateVertexBuffer(_batchedBoneVertexData.data(), sizeof(VertexData) * _batchedVeticesCount);

#ifdef CC_STUDIO_ENABLED_VIEW
// TODO
//     glLineWidth(1);
//     glEnable(GL_LINE_SMOOTH);
//     glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
//     for (int i = 0; i < _batchedVeticesCount; i += 4 )
//     {
//         glDrawArrays(GL_TRIANGLE_FAN, i, 4);
//         glDrawArrays(GL_LINE_LOOP, i, 4);
//     }
//     CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _batchedVeticesCount);
#else
    unsigned short* indices = (unsigned short*)malloc(sizeof(unsigned short) * _batchedVeticesCount);
    for (int i = 0; i < _batchedVeticesCount; i += 4)
    {
        *indices++ = i;
        *indices++ = i + 1;
        *indices++ = i + 2;

        *indices++ = i;
        *indices++ = i + 2;
        *indices++ = i + 3;
    }
    _batchBoneCommand.createIndexBuffer(cocos2d::CustomCommand::IndexFormat::U_SHORT, _batchedVeticesCount,
                                        cocos2d::CustomCommand::BufferUsage::DYNAMIC);
    _batchBoneCommand.updateIndexBuffer(indices, sizeof(unsigned short) * _batchedVeticesCount);
    free(indices);
#endif  // CC_STUDIO_ENABLED_VIEW
}

void SkeletonNode::changeSkins(const hlookup::string_map<std::string>& boneSkinNameMap)
{
    for (auto& boneskin : boneSkinNameMap)
    {
        auto bone = getBoneNode(boneskin.first);
        if (nullptr != bone)
            bone->displaySkin(boneskin.second, true);
    }
}

void SkeletonNode::changeSkins(std::string_view skinGroupName)
{
    auto suit = _skinGroupMap.find(skinGroupName);
    if (suit != _skinGroupMap.end())
    {
        changeSkins(suit->second);
    }
}

BoneNode* SkeletonNode::getBoneNode(std::string_view boneName)
{
    auto iter = _subBonesMap.find(boneName);
    if (iter != _subBonesMap.end())
    {
        return iter->second;
    }
    return nullptr;
}

const cocos2d::StringMap<BoneNode*>& SkeletonNode::getAllSubBonesMap() const
{
    return _subBonesMap;
}

void SkeletonNode::addSkinGroup(std::string groupName, hlookup::string_map<std::string> boneSkinNameMap)
{
    _skinGroupMap.emplace(groupName, std::move(boneSkinNameMap));
}

void SkeletonNode::checkSubBonesDirty()
{
    if (_subBonesDirty)
    {
        updateOrderedAllbones();
        _subBonesDirty = false;
    }
    if (_subBonesOrderDirty)
    {
        sortOrderedAllBones();
        _subBonesOrderDirty = false;
    }
}

void SkeletonNode::updateOrderedAllbones()
{
    _subOrderedAllBones.clear();
    // update sub bones, get All Visible SubBones
    // get all sub bones as visit with visible
    std::stack<BoneNode*> boneStack;
    for (const auto& bone : _childBones)
    {
        if (bone->isVisible())
            boneStack.push(bone);
    }

    while (boneStack.size() > 0)
    {
        auto top = boneStack.top();
        _subOrderedAllBones.pushBack(top);
        boneStack.pop();
        auto topChildren = top->getChildBones();
        for (const auto& childbone : topChildren)
        {
            if (childbone->isVisible())
                boneStack.push(childbone);
        }
    }
}

void SkeletonNode::sortOrderedAllBones()
{
    sortNodes(this->_subOrderedAllBones);
}

NS_TIMELINE_END
