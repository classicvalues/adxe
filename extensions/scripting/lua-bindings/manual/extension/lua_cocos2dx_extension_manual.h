/****************************************************************************
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

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
#ifndef COCOS2DX_SCRIPT_LUA_COCOS2DX_SUPPORT_LUA_COCOS2DX_EXTENSION_MANUAL_H
#define COCOS2DX_SCRIPT_LUA_COCOS2DX_SUPPORT_LUA_COCOS2DX_EXTENSION_MANUAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

/**
 * @addtogroup lua
 * @{
 */

/**
 * Call this function can import the lua bindings for classes in the `cocos2d::extension` namespace like
 * cocos2d::extension::Control, cocos2d::extension::ControlHuePicker, and so on. After registering, we could call the
 * related cocosbuilder code conveniently in the lua.eg,.cc.Control:create(). If you don't want to use the extension
 * module in the lua, you only don't call this registering function. If you don't register the extension module, the
 * package size would become smaller . The current mechanism,this registering function is called in the
 * lua_module_register.h
 */
TOLUA_API int register_extension_module(lua_State* tolua_S);

// end group
/// @}

/// @cond
TOLUA_API int register_all_cocos2dx_extension_manual(lua_State* tolua_S);
/// @endcond

struct LuaAssetsManagerEventData
{
    int value;

    LuaAssetsManagerEventData(int _value = 0) : value(_value) {}
};

struct LuaTableViewEventData
{
    void* value;

    // Constructor
    LuaTableViewEventData(void* _value = nullptr) : value(_value) {}
};

#endif  // #ifndef COCOS2DX_SCRIPT_LUA_COCOS2DX_SUPPORT_LUA_COCOS2DX_EXTENSION_MANUAL_H
