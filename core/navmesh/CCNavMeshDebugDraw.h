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

#pragma once

#include "base/ccConfig.h"
#if CC_USE_NAVMESH

#    include "renderer/CCRenderState.h"
#    include "renderer/backend/ProgramState.h"
#    include "renderer/backend/Types.h"
#    include "renderer/backend/Buffer.h"
#    include "math/Vec3.h"
#    include "recast/DebugDraw.h"
#    include "renderer/CCGroupCommand.h"
#    include "renderer/CCCallbackCommand.h"
#    include "renderer/CCCustomCommand.h"
#    include <string>
#    include <vector>

NS_CC_BEGIN

/**
 * @addtogroup 3d
 * @{
 */
class Renderer;
class NavMeshDebugDraw : public duDebugDraw
{
public:
    NavMeshDebugDraw();
    virtual ~NavMeshDebugDraw();

    virtual void depthMask(bool state) override;
    virtual void texture(bool /*state*/) override{};
    virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f) override;

    virtual void vertex(const float* pos, unsigned int color) override;
    virtual void vertex(const float x, const float y, const float z, unsigned int color) override;

    virtual void vertex(const float* pos, unsigned int color, const float* uv) override;
    virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
        override;

    virtual void end() override;

    void draw(Renderer* renderer);

    void clear();

private:
    void initCustomCommand(CustomCommand& command);
    backend::PrimitiveType getPrimitiveType(duDebugDrawPrimitives prim);
    static Vec4 getColor(unsigned int col);

    void onBeforeVisitCmd();
    void onAfterVisitCmd();
    void onBeforeEachCommand(bool enableDepthWrite);

private:
    struct V3F_C4F
    {
        Vec3 position;
        Vec4 color;
    };

    struct Primitive
    {
        backend::PrimitiveType type = backend::PrimitiveType::LINE;
        bool depthMask              = false;
        unsigned short start        = 0;
        unsigned short end          = 0;
        float size                  = 1.0f;
    };

    Primitive* _currentPrimitive         = nullptr;
    backend::ProgramState* _programState = nullptr;
    bool _currentDepthMask               = true;
    bool _dirtyBuffer                    = true;
    backend::Buffer* _vertexBuffer       = nullptr;

    // RenderState::StateBlock     _stateBlock;
    std::vector<V3F_C4F> _vertices;
    std::vector<Primitive*> _primitiveList;
    backend::UniformLocation _locMVP;
    std::vector<CustomCommand> _commands;

    CallbackCommand _beforeCommand;
    CallbackCommand _afterCommand;

    // renderer state cache variables
    bool _rendererDepthTestEnabled                 = true;
    backend::CompareFunction _rendererDepthCmpFunc = backend::CompareFunction::LESS;
    backend::CullMode _rendererCullMode            = backend::CullMode::BACK;
    backend::Winding _rendererWinding              = backend::Winding::COUNTER_CLOCK_WISE;
    bool _rendererDepthWrite                       = false;
};

/** @} */

NS_CC_END

#endif  // CC_USE_NAVMESH
