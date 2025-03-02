/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#ifndef __CC_ANIMATION_CACHE_H__
#define __CC_ANIMATION_CACHE_H__

#include "base/CCRef.h"
#include "base/CCMap.h"
#include "base/CCValue.h"
#include "2d/CCAnimation.h"

#include <string>

NS_CC_BEGIN

class Animation;

/**
 * @addtogroup _2d
 * @{
 */

/** Singleton that manages the Animations.
It saves in a cache the animations. You should use this class if you want to save your animations in a cache.

Before v0.99.5, the recommend way was to save them on the Sprite. Since v0.99.5, you should use this class instead.

@since v0.99.5
@js cc.animationCache
*/
class CC_DLL AnimationCache : public Ref
{
public:
    /**
     * @js ctor
     */
    AnimationCache();
    /**
     * @js NA
     * @lua NA
     */
    ~AnimationCache();
    /** Returns the shared instance of the Animation cache
         @js NA
        */
    static AnimationCache* getInstance();

    /** Purges the cache. It releases all the Animation objects and the shared instance.
                @js NA
     */
    static void destroyInstance();

    bool init();

    /** Adds a Animation with a name.
     *
     * @param animation An animation.
     * @param name The name of animation.
     */
    void addAnimation(Animation* animation, std::string_view name);

    /** Deletes a Animation from the cache.
     *
     * @param name The name of animation.
     */
    void removeAnimation(std::string_view name);

    /** Returns a Animation that was previously added.
     * If the name is not found it will return nil.
     * You should retain the returned copy if you are going to use it.
     *
     * @return A Animation that was previously added. If the name is not found it will return nil.
     */
    Animation* getAnimation(std::string_view name);

    /** Adds an animation from an NSDictionary.
     * Make sure that the frames were previously loaded in the SpriteFrameCache.
     * @param dictionary An NSDictionary.
     * @param plist The path of the relative file,it use to find the plist path for load SpriteFrames.
     * @since v1.1
         @js NA
     */
    void addAnimationsWithDictionary(const ValueMap& dictionary, std::string_view plist);

    /** Adds an animation from a plist file.
     * Make sure that the frames were previously loaded in the SpriteFrameCache.
     * @since v1.1
     * @js addAnimations
     * @lua addAnimations
     * @param plist An animation from a plist file.
     */
    void addAnimationsWithFile(std::string_view plist);

private:
    void parseVersion1(const ValueMap& animations);
    void parseVersion2(const ValueMap& animations);

private:
    StringMap<Animation*> _animations;
    static AnimationCache* s_sharedAnimationCache;
};

// end of sprite_nodes group
/// @}

NS_CC_END

#endif  // __CC_ANIMATION_CACHE_H__
