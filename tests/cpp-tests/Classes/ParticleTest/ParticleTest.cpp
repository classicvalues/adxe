/****************************************************************************
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

#include "ParticleTest.h"
#include "../testResource.h"
#include "cocostudio/CocosStudioExtension.h"

USING_NS_CC;

enum
{
    kTagParticleCount = 1,
};

//------------------------------------------------------------------
//
// DemoFirework
//
//------------------------------------------------------------------
void DemoFirework::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleFireworks::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));

    setEmitterPosition();
}

std::string DemoFirework::subtitle() const
{
    return "ParticleFireworks";
}

//------------------------------------------------------------------
//
// DemoFire
//
//------------------------------------------------------------------
void DemoFire::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleFire::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));  //.pvr");
    auto p = _emitter->getPosition();
    _emitter->setPosition(Vec2(p.x, 100.0f));

    setEmitterPosition();
}

std::string DemoFire::subtitle() const
{
    return "ParticleFire";
}

//------------------------------------------------------------------
//
// DemoSun
//
//------------------------------------------------------------------
void DemoSun::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSun::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
}

std::string DemoSun::subtitle() const
{
    return "ParticleSun";
}

//------------------------------------------------------------------
//
// DemoPause
//
//------------------------------------------------------------------
void DemoPause::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSmoke::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
    schedule(CC_SCHEDULE_SELECTOR(DemoPause::pauseEmitter), 2.0f);
}
void DemoPause::pauseEmitter(float time)
{
    if (_emitter->isPaused())
    {
        _emitter->resumeEmissions();
    }
    else
    {
        _emitter->pauseEmissions();
    }
}

std::string DemoPause::subtitle() const
{
    return "Pause Particle";
}

//------------------------------------------------------------------
//
// DemoGalaxy
//
//------------------------------------------------------------------
void DemoGalaxy::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleGalaxy::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
}

std::string DemoGalaxy::subtitle() const
{
    return "ParticleGalaxy";
}

//------------------------------------------------------------------
//
// DemoFlower
//
//------------------------------------------------------------------
void DemoFlower::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleFlower::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));

    setEmitterPosition();
}

std::string DemoFlower::subtitle() const
{
    return "ParticleFlower";
}

//------------------------------------------------------------------
//
// DemoBigFlower
//
//------------------------------------------------------------------
void DemoBigFlower::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSystemQuad::createWithTotalParticles(50);
    _emitter->retain();

    _background->addChild(_emitter, 10);
    ////_emitter->release();    // win32 :  use this line or remove this line and use autorelease()
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));

    _emitter->setDuration(-1);

    // gravity
    _emitter->setGravity(Vec2::ZERO);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(360);

    // speed of particles
    _emitter->setSpeed(160);
    _emitter->setSpeedVar(20);

    // radial
    _emitter->setRadialAccel(-120);
    _emitter->setRadialAccelVar(0);

    // tangential
    _emitter->setTangentialAccel(30);
    _emitter->setTangentialAccelVar(0);

    // emitter position
    _emitter->setPosition(Vec2(160, 240));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(4);
    _emitter->setLifeVar(1);

    // spin of particles
    _emitter->setStartSpin(0);
    _emitter->setStartSizeVar(0);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(0);

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(80.0f);
    _emitter->setStartSizeVar(40.0f);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // additive
    _emitter->setBlendAdditive(true);

    setEmitterPosition();
}

std::string DemoBigFlower::subtitle() const
{
    return "ParticleBigFlower";
}

//------------------------------------------------------------------
//
// DemoRotFlower
//
//------------------------------------------------------------------
void DemoRotFlower::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSystemQuad::createWithTotalParticles(300);
    _emitter->retain();

    _background->addChild(_emitter, 10);
    ////_emitter->release();    // win32 : Remove this line
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars2));

    // duration
    _emitter->setDuration(-1);

    // gravity
    _emitter->setGravity(Vec2::ZERO);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(360);

    // speed of particles
    _emitter->setSpeed(160);
    _emitter->setSpeedVar(20);

    // radial
    _emitter->setRadialAccel(-120);
    _emitter->setRadialAccelVar(0);

    // tangential
    _emitter->setTangentialAccel(30);
    _emitter->setTangentialAccelVar(0);

    // emitter position
    _emitter->setPosition(Vec2(160, 240));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(3);
    _emitter->setLifeVar(1);

    // spin of particles
    _emitter->setStartSpin(0);
    _emitter->setStartSpinVar(0);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(2000);

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(30.0f);
    _emitter->setStartSizeVar(00.0f);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // additive
    _emitter->setBlendAdditive(false);

    setEmitterPosition();
}

std::string DemoRotFlower::subtitle() const
{
    return "ParticleRotFlower";
}

//------------------------------------------------------------------
//
// DemoMeteor
//
//------------------------------------------------------------------
void DemoMeteor::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleMeteor::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
}

std::string DemoMeteor::subtitle() const
{
    return "ParticleMeteor";
}

//------------------------------------------------------------------
//
// DemoSpiral
//
//------------------------------------------------------------------
void DemoSpiral::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSpiral::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
}

std::string DemoSpiral::subtitle() const
{
    return "ParticleSpiral";
}

//------------------------------------------------------------------
//
// DemoExplosion
//
//------------------------------------------------------------------
void DemoExplosion::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleExplosion::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));

    _emitter->setAutoRemoveOnFinish(true);

    setEmitterPosition();
}

std::string DemoExplosion::subtitle() const
{
    return "ParticleExplosion";
}

//------------------------------------------------------------------
//
// DemoSmoke
//
//------------------------------------------------------------------
void DemoSmoke::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSmoke::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    auto p = _emitter->getPosition();
    _emitter->setPosition(Vec2(p.x, 100.0f));

    setEmitterPosition();
}

std::string DemoSmoke::subtitle() const
{
    return "ParticleSmoke";
}

//------------------------------------------------------------------
//
// DemoSnow
//
//------------------------------------------------------------------
void DemoSnow::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSnow::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    auto p = _emitter->getPosition();
    _emitter->setPosition(Vec2(p.x, p.y - 110));
    _emitter->setLife(3);
    _emitter->setLifeVar(1);

    // gravity
    _emitter->setGravity(Vec2(0.0f, -10.0f));

    // speed of particles
    _emitter->setSpeed(130);
    _emitter->setSpeedVar(30);

    Color4F startColor = _emitter->getStartColor();
    startColor.r       = 0.9f;
    startColor.g       = 0.9f;
    startColor.b       = 0.9f;
    _emitter->setStartColor(startColor);

    Color4F startColorVar = _emitter->getStartColorVar();
    startColorVar.b       = 0.1f;
    _emitter->setStartColorVar(startColorVar);

    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_snow));

    setEmitterPosition();
}

std::string DemoSnow::subtitle() const
{
    return "ParticleSnow";
}

//------------------------------------------------------------------
//
// DemoRain
//
//------------------------------------------------------------------
void DemoRain::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleRain::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    auto p = _emitter->getPosition();
    _emitter->setPosition(Vec2(p.x, p.y - 100));
    _emitter->setLife(4);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    setEmitterPosition();
}

std::string DemoRain::subtitle() const
{
    return "ParticleRain";
}

//------------------------------------------------------------------
//
// DemoModernArt
//
//------------------------------------------------------------------
void DemoModernArt::onEnter()
{
    ParticleDemo::onEnter();

    // FIXME: If use ParticleSystemPoint, bada 1.0 device will crash.
    //   Crash place: ParticleSystemPoint.cpp Line 149, function: glDrawArrays(GL_POINTS, 0, _particleIdx);
    _emitter = ParticleSystemQuad::createWithTotalParticles(1000);
    _emitter->retain();

    _background->addChild(_emitter, 10);
    ////_emitter->release();

    auto s = Director::getInstance()->getWinSize();

    // duration
    _emitter->setDuration(-1);

    // gravity
    _emitter->setGravity(Vec2(0.0f, 0.0f));

    // angle
    _emitter->setAngle(0);
    _emitter->setAngleVar(360);

    // radial
    _emitter->setRadialAccel(70);
    _emitter->setRadialAccelVar(10);

    // tangential
    _emitter->setTangentialAccel(80);
    _emitter->setTangentialAccelVar(0);

    // speed of particles
    _emitter->setSpeed(50);
    _emitter->setSpeedVar(10);

    // emitter position
    _emitter->setPosition(Vec2(s.width / 2, s.height / 2));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(2.0f);
    _emitter->setLifeVar(0.3f);

    // emits per frame
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(1.0f);
    _emitter->setStartSizeVar(1.0f);
    _emitter->setEndSize(32.0f);
    _emitter->setEndSizeVar(8.0f);

    // texture
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    // additive
    _emitter->setBlendAdditive(false);

    setEmitterPosition();
}

std::string DemoModernArt::subtitle() const
{
    return "Varying size";
}

//------------------------------------------------------------------
//
// DemoRing
//
//------------------------------------------------------------------
void DemoRing::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleFlower::create();
    _emitter->retain();

    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));
    _emitter->setLifeVar(0);
    _emitter->setLife(10);
    _emitter->setSpeed(100);
    _emitter->setSpeedVar(0);
    _emitter->setEmissionRate(10000);

    setEmitterPosition();
}

std::string DemoRing::subtitle() const
{
    return "Ring Demo";
}

//------------------------------------------------------------------
//
// ParallaxParticle
//
//------------------------------------------------------------------
void ParallaxParticle::onEnter()
{
    ParticleDemo::onEnter();

    _background->getParent()->removeChild(_background, true);
    _background = nullptr;

    auto p = ParallaxNode::create();
    addChild(p, 5);

    auto p1 = Sprite::create(s_back3);
    auto p2 = Sprite::create(s_back3);

    p->addChild(p1, 1, Vec2(0.5f, 1.0f), Vec2(0.0f, 250.0f));
    p->addChild(p2, 2, Vec2(1.5f, 1.0f), Vec2(0.0f, 50.0f));

    _emitter = ParticleFlower::create();
    _emitter->retain();
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    p1->addChild(_emitter, 10);
    _emitter->setPosition(Vec2(250, 200));

    auto par = ParticleSun::create();
    p2->addChild(par, 10);
    par->setTexture(Director::getInstance()->getTextureCache()->addImage(s_fire));

    auto move      = MoveBy::create(4, Vec2(300.0f, 0.0f));
    auto move_back = move->reverse();
    auto seq       = Sequence::create(move, move_back, nullptr);
    p->runAction(RepeatForever::create(seq));
}

std::string ParallaxParticle::subtitle() const
{
    return "Parallax + Particles";
}

//------------------------------------------------------------------
//
// RadiusMode1
//
//------------------------------------------------------------------
void RadiusMode1::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::createWithTotalParticles(200);
    _emitter->retain();
    addChild(_emitter, 10);
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("Images/stars-grayscale.png"));

    // duration
    _emitter->setDuration(ParticleSystem::DURATION_INFINITY);

    // radius mode
    _emitter->setEmitterMode(ParticleSystem::Mode::RADIUS);

    // radius mode: start and end radius in pixels
    _emitter->setStartRadius(0);
    _emitter->setStartRadiusVar(0);
    _emitter->setEndRadius(160);
    _emitter->setEndRadiusVar(0);

    // radius mode: degrees per second
    _emitter->setRotatePerSecond(180);
    _emitter->setRotatePerSecondVar(0);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(0);

    // emitter position
    auto size = Director::getInstance()->getWinSize();
    _emitter->setPosition(Vec2(size.width / 2, size.height / 2));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(5);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(0);
    _emitter->setStartSpinVar(0);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(0);

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(32);
    _emitter->setStartSizeVar(0);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // additive
    _emitter->setBlendAdditive(false);
}

std::string RadiusMode1::subtitle() const
{
    return "Radius Mode: Spiral";
}

//------------------------------------------------------------------
//
// RadiusMode2
//
//------------------------------------------------------------------
void RadiusMode2::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::createWithTotalParticles(200);
    _emitter->retain();
    addChild(_emitter, 10);
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("Images/stars-grayscale.png"));

    // duration
    _emitter->setDuration(ParticleSystem::DURATION_INFINITY);

    // radius mode
    _emitter->setEmitterMode(ParticleSystem::Mode::RADIUS);

    // radius mode: start and end radius in pixels
    _emitter->setStartRadius(100);
    _emitter->setStartRadiusVar(0);
    _emitter->setEndRadius(ParticleSystem::START_RADIUS_EQUAL_TO_END_RADIUS);
    _emitter->setEndRadiusVar(0);

    // radius mode: degrees per second
    _emitter->setRotatePerSecond(45);
    _emitter->setRotatePerSecondVar(0);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(0);

    // emitter position
    auto size = Director::getInstance()->getWinSize();
    _emitter->setPosition(Vec2(size.width / 2, size.height / 2));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(4);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(0);
    _emitter->setStartSpinVar(0);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(0);

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(32);
    _emitter->setStartSizeVar(0);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // additive
    _emitter->setBlendAdditive(false);
}

std::string RadiusMode2::subtitle() const
{
    return "Radius Mode: Semi Circle";
}

//------------------------------------------------------------------
//
// Issue704
//
//------------------------------------------------------------------
void Issue704::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::createWithTotalParticles(100);
    _emitter->retain();
    addChild(_emitter, 10);
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("Images/fire.png"));

    // duration
    _emitter->setDuration(ParticleSystem::DURATION_INFINITY);

    // radius mode
    _emitter->setEmitterMode(ParticleSystem::Mode::RADIUS);

    // radius mode: start and end radius in pixels
    _emitter->setStartRadius(50);
    _emitter->setStartRadiusVar(0);
    _emitter->setEndRadius(ParticleSystem::START_RADIUS_EQUAL_TO_END_RADIUS);
    _emitter->setEndRadiusVar(0);

    // radius mode: degrees per second
    _emitter->setRotatePerSecond(0);
    _emitter->setRotatePerSecondVar(0);

    // angle
    _emitter->setAngle(90);
    _emitter->setAngleVar(0);

    // emitter position
    auto size = Director::getInstance()->getWinSize();
    _emitter->setPosition(Vec2(size.width / 2, size.height / 2));
    _emitter->setPosVar(Vec2::ZERO);

    // life of particles
    _emitter->setLife(5);
    _emitter->setLifeVar(0);

    // spin of particles
    _emitter->setStartSpin(0);
    _emitter->setStartSpinVar(0);
    _emitter->setEndSpin(0);
    _emitter->setEndSpinVar(0);

    // color of particles
    Color4F startColor(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColor(startColor);

    Color4F startColorVar(0.5f, 0.5f, 0.5f, 1.0f);
    _emitter->setStartColorVar(startColorVar);

    Color4F endColor(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColor(endColor);

    Color4F endColorVar(0.1f, 0.1f, 0.1f, 0.2f);
    _emitter->setEndColorVar(endColorVar);

    // size, in pixels
    _emitter->setStartSize(16);
    _emitter->setStartSizeVar(0);
    _emitter->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

    // emits per second
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());

    // additive
    _emitter->setBlendAdditive(false);

    auto rot = RotateBy::create(16, 360);
    _emitter->runAction(RepeatForever::create(rot));
}

std::string Issue704::title() const
{
    return "Issue 704. Free + Rot";
}

std::string Issue704::subtitle() const
{
    return "Emitted particles should not rotate";
}

//------------------------------------------------------------------
//
// Issue870
//
//------------------------------------------------------------------
void Issue870::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::create("Particles/SpinningPeas.plist");
    _emitter->setTextureWithRect(Director::getInstance()->getTextureCache()->addImage("Images/particles.png"),
                                 Rect(0, 0, 32, 32));
    addChild(_emitter, 10);
    _emitter->retain();

    _index = 0;
    schedule(CC_SCHEDULE_SELECTOR(Issue870::updateQuads), 2.0f);
}

void Issue870::updateQuads(float dt)
{
    _index      = (_index + 1) % 4;
    auto rect   = Rect(_index * 32, 0, 32, 32);
    auto system = (ParticleSystemQuad*)_emitter;
    system->setTextureWithRect(_emitter->getTexture(), rect);
}

std::string Issue870::title() const
{
    return "Issue 870. SubRect";
}

std::string Issue870::subtitle() const
{
    return "Every 2 seconds the particle should change";
}

//------------------------------------------------------------------
//
// DemoParticleFromFile
//
//------------------------------------------------------------------
void DemoParticleFromFile::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    std::string filename = "Particles/" + _title + ".plist";
    _emitter             = ParticleSystemQuad::create(filename);
    _emitter->retain();
    addChild(_emitter, 10);

    setEmitterPosition();
}

//------------------------------------------------------------------
//
// ParticleDemo
//
//------------------------------------------------------------------

enum
{
    IDC_NEXT = 100,
    IDC_BACK,
    IDC_RESTART,
    IDC_TOGGLE
};

ParticleTests::ParticleTests()
{
    ADD_TEST_CASE(ParticleReorder);
    ADD_TEST_CASE(ParticleBatchHybrid);
    ADD_TEST_CASE(ParticleBatchMultipleEmitters);
    ADD_TEST_CASE(DemoFlower);
    ADD_TEST_CASE(DemoGalaxy);
    ADD_TEST_CASE(DemoFirework);
    ADD_TEST_CASE(DemoSpiral);
    ADD_TEST_CASE(DemoSun);
    ADD_TEST_CASE(DemoMeteor);
    ADD_TEST_CASE(DemoFire);
    ADD_TEST_CASE(DemoSmoke);
    ADD_TEST_CASE(DemoExplosion);
    ADD_TEST_CASE(DemoSnow);
    ADD_TEST_CASE(DemoRain);
    ADD_TEST_CASE(DemoBigFlower);
    ADD_TEST_CASE(DemoRotFlower);
    ADD_TEST_CASE(DemoModernArt);
    ADD_TEST_CASE(DemoRing);
    ADD_TEST_CASE(ParallaxParticle);
    ADD_TEST_CASE(DemoPause);
    addTestCase("BoilingFoam", []() { return DemoParticleFromFile::create("BoilingFoam"); });
    addTestCase("BurstPipe", []() { return DemoParticleFromFile::create("BurstPipe"); });
    addTestCase("Comet", []() { return DemoParticleFromFile::create("Comet"); });
    addTestCase("debian", []() { return DemoParticleFromFile::create("debian"); });
    addTestCase("ExplodingRing", []() { return DemoParticleFromFile::create("ExplodingRing"); });
    addTestCase("LavaFlow", []() { return DemoParticleFromFile::create("LavaFlow"); });
    addTestCase("SpinningPeas", []() { return DemoParticleFromFile::create("SpinningPeas"); });
    addTestCase("SpookyPeas", []() { return DemoParticleFromFile::create("SpookyPeas"); });
    addTestCase("Upsidedown", []() { return DemoParticleFromFile::create("Upsidedown"); });
    addTestCase("Flower", []() { return DemoParticleFromFile::create("Flower"); });
    addTestCase("Spiral", []() { return DemoParticleFromFile::create("Spiral"); });
    addTestCase("Galaxy", []() { return DemoParticleFromFile::create("Galaxy"); });
    addTestCase("Phoenix", []() { return DemoParticleFromFile::create("Phoenix"); });
    addTestCase("lines", []() { return DemoParticleFromFile::create("lines"); });
    addTestCase("ButterFly", []() { return DemoParticleFromFile::create("ButterFly"); });
    addTestCase("ButterFlyYFlipped", []() { return DemoParticleFromFile::create("ButterFlyYFlipped"); });
    ADD_TEST_CASE(RadiusMode1);
    ADD_TEST_CASE(RadiusMode2);
    ADD_TEST_CASE(Issue704);
    ADD_TEST_CASE(Issue870);
    ADD_TEST_CASE(Issue1201);

    ADD_TEST_CASE(MultipleParticleSystems);
    ADD_TEST_CASE(MultipleParticleSystemsBatched);
    ADD_TEST_CASE(AddAndDeleteParticleSystems);
    ADD_TEST_CASE(ReorderParticleSystems);
    ADD_TEST_CASE(PremultipliedAlphaTest);
    ADD_TEST_CASE(PremultipliedAlphaTest2);
    ADD_TEST_CASE(Issue3990);
    ADD_TEST_CASE(ParticleAutoBatching);
    ADD_TEST_CASE(ParticleVisibleTest);
    ADD_TEST_CASE(ParticleResetTotalParticles);

    ADD_TEST_CASE(ParticleIssue12310);
    ADD_TEST_CASE(ParticleSpriteFrame);
}

ParticleDemo::~ParticleDemo()
{
    CC_SAFE_RELEASE(_emitter);
}

void ParticleDemo::onEnter()
{
    TestCase::onEnter();

    MenuItemFont::setFontSize(32);

    _color = LayerColor::create(Color4B(127, 127, 127, 255));
    this->addChild(_color);

    _emitter = nullptr;

    auto listener            = EventListenerTouchAllAtOnce::create();
    listener->onTouchesBegan = CC_CALLBACK_2(ParticleDemo::onTouchesBegan, this);
    listener->onTouchesMoved = CC_CALLBACK_2(ParticleDemo::onTouchesMoved, this);
    listener->onTouchesEnded = CC_CALLBACK_2(ParticleDemo::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    auto s = Director::getInstance()->getWinSize();

    auto item4 = MenuItemToggle::createWithCallback(
        CC_CALLBACK_1(ParticleDemo::toggleCallback, this), MenuItemFont::create("Free Movement"),
        MenuItemFont::create("Relative Movement"), MenuItemFont::create("Grouped Movement"), nullptr);

    auto menu = Menu::create(item4, nullptr);

    menu->setPosition(Vec2::ZERO);
    item4->setPosition(Vec2(VisibleRect::left().x, VisibleRect::bottom().y + 100));
    item4->setAnchorPoint(Vec2(0, 0));

    addChild(menu, 100);

    auto labelAtlas = LabelAtlas::create("0000", "fps_images.png", 12, 32, '.');
    addChild(labelAtlas, 100, kTagParticleCount);
    labelAtlas->setPosition(Vec2(s.width - 66, 50.0f));

    // moving background
    _background = Sprite::create(s_back3);
    addChild(_background, 5);
    _background->setPosition(Vec2(s.width / 2, s.height - 180));

    auto move      = MoveBy::create(4, Vec2(300, 0));
    auto move_back = move->reverse();
    auto seq       = Sequence::create(move, move_back, nullptr);
    _background->runAction(RepeatForever::create(seq));

    scheduleUpdate();
}

std::string ParticleDemo::title() const
{
    return "Particle Demo";
}

std::string ParticleDemo::subtitle() const
{
    return "No title";
}

void ParticleDemo::onTouchesBegan(const std::vector<Touch*>& touches, Event* event)
{
    onTouchesEnded(touches, event);
}

void ParticleDemo::onTouchesMoved(const std::vector<Touch*>& touches, Event* event)
{
    return onTouchesEnded(touches, event);
}

void ParticleDemo::onTouchesEnded(const std::vector<Touch*>& touches, Event* event)
{
    auto touch = touches[0];

    auto location = touch->getLocation();

    auto pos = Vec2::ZERO;
    if (_background)
    {
        pos = _background->convertToWorldSpace(Vec2::ZERO);
    }

    if (_emitter != nullptr)
    {
        _emitter->setPosition(location - pos);
    }
}

void ParticleDemo::update(float dt)
{
    if (_emitter)
    {
        auto atlas  = (LabelAtlas*)getChildByTag(kTagParticleCount);
        char str[5] = {0};
        sprintf(str, "%04d", _emitter->getParticleCount());
        atlas->setString(str);
    }
}

void ParticleDemo::toggleCallback(Ref* sender)
{
    if (_emitter != nullptr)
    {
        if (_emitter->getPositionType() == ParticleSystem::PositionType::GROUPED)
            _emitter->setPositionType(ParticleSystem::PositionType::FREE);
        else if (_emitter->getPositionType() == ParticleSystem::PositionType::FREE)
            _emitter->setPositionType(ParticleSystem::PositionType::RELATIVE);
        else if (_emitter->getPositionType() == ParticleSystem::PositionType::RELATIVE)
            _emitter->setPositionType(ParticleSystem::PositionType::GROUPED);
    }
}

void ParticleDemo::setEmitterPosition()
{
    auto s = Director::getInstance()->getWinSize();
    if (_emitter != nullptr)
    {
        _emitter->setPosition(Vec2(s.width / 2, s.height / 2));
    }
}

// ParticleBatchHybrid

void ParticleBatchHybrid::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::create("Particles/LavaFlow.plist");
    _emitter->retain();
    auto batch = ParticleBatchNode::createWithTexture(_emitter->getTexture());

    batch->addChild(_emitter);

    addChild(batch, 10);

    schedule(CC_SCHEDULE_SELECTOR(ParticleBatchHybrid::switchRender), 2.0f);

    auto node = Node::create();
    addChild(node);

    _parent1 = batch;
    _parent2 = node;
}

void ParticleBatchHybrid::switchRender(float dt)
{
    bool usingBatch = (_emitter->getBatchNode() != nullptr);
    _emitter->removeFromParentAndCleanup(false);

    auto newParent = (usingBatch ? _parent2 : _parent1);
    newParent->addChild(_emitter);

    log("Particle: Using new parent: %s", usingBatch ? "CCNode" : "CCParticleBatchNode");
}

std::string ParticleBatchHybrid::title() const
{
    return "Particle Batch";
}

std::string ParticleBatchHybrid::subtitle() const
{
    return "Hybrid: batched and non batched every 2 seconds";
}

// ParticleBatchMultipleEmitters

void ParticleBatchMultipleEmitters::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    auto emitter1 = ParticleSystemQuad::create("Particles/LavaFlow.plist");
    emitter1->setStartColor(Color4F(1, 0, 0, 1));
    auto emitter2 = ParticleSystemQuad::create("Particles/LavaFlow.plist");
    emitter2->setStartColor(Color4F(0, 1, 0, 1));
    auto emitter3 = ParticleSystemQuad::create("Particles/LavaFlow.plist");
    emitter3->setStartColor(Color4F(0, 0, 1, 1));

    auto s = Director::getInstance()->getWinSize();

    emitter1->setPosition(Vec2(s.width / 1.25f, s.height / 1.25f));
    emitter2->setPosition(Vec2(s.width / 2, s.height / 2));
    emitter3->setPosition(Vec2(s.width / 4, s.height / 4));

    auto batch = ParticleBatchNode::createWithTexture(emitter1->getTexture());

    batch->addChild(emitter1, 0);
    batch->addChild(emitter2, 0);
    batch->addChild(emitter3, 0);

    addChild(batch, 10);
}

std::string ParticleBatchMultipleEmitters::title() const
{
    return "Particle Batch";
}

std::string ParticleBatchMultipleEmitters::subtitle() const
{
    return "Multiple emitters. One Batch";
}

// ParticleReorder

void ParticleReorder::onEnter()
{
    ParticleDemo::onEnter();

    _order = 0;
    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    auto ignore  = ParticleSystemQuad::create("Particles/SmallSun.plist");
    auto parent1 = Node::create();
    auto parent2 = ParticleBatchNode::createWithTexture(ignore->getTexture());
    ignore->unscheduleUpdate();

    for (unsigned int i = 0; i < 2; i++)
    {
        auto parent = (i == 0 ? parent1 : parent2);

        auto emitter1 = ParticleSystemQuad::create("Particles/SmallSun.plist");
        emitter1->setStartColor(Color4F(1, 0, 0, 1));
        emitter1->setBlendAdditive(false);
        auto emitter2 = ParticleSystemQuad::create("Particles/SmallSun.plist");
        emitter2->setStartColor(Color4F(0, 1, 0, 1));
        emitter2->setBlendAdditive(false);
        auto emitter3 = ParticleSystemQuad::create("Particles/SmallSun.plist");
        emitter3->setStartColor(Color4F(0, 0, 1, 1));
        emitter3->setBlendAdditive(false);

        auto s = Director::getInstance()->getWinSize();

        int neg = (i == 0 ? 1 : -1);

        emitter1->setPosition(Vec2(s.width / 2 - 30, s.height / 2 + 60 * neg));
        emitter2->setPosition(Vec2(s.width / 2, s.height / 2 + 60 * neg));
        emitter3->setPosition(Vec2(s.width / 2 + 30, s.height / 2 + 60 * neg));

        parent->addChild(emitter1, 0, 1);
        parent->addChild(emitter2, 0, 2);
        parent->addChild(emitter3, 0, 3);

        addChild(parent, 10, 1000 + i);
    }

    schedule(CC_SCHEDULE_SELECTOR(ParticleReorder::reorderParticles), 1.0f);
}

std::string ParticleReorder::title() const
{
    return "Reordering particles";
}

std::string ParticleReorder::subtitle() const
{
    return "Reordering particles with and without batches batches";
}

void ParticleReorder::reorderParticles(float dt)
{
    for (int i = 0; i < 2; i++)
    {
        auto parent = getChildByTag(1000 + i);

        auto child1 = parent->getChildByTag(1);
        auto child2 = parent->getChildByTag(2);
        auto child3 = parent->getChildByTag(3);

        if (_order % 3 == 0)
        {
            parent->reorderChild(child1, 1);
            parent->reorderChild(child2, 2);
            parent->reorderChild(child3, 3);
        }
        else if (_order % 3 == 1)
        {
            parent->reorderChild(child1, 3);
            parent->reorderChild(child2, 1);
            parent->reorderChild(child3, 2);
        }
        else if (_order % 3 == 2)
        {
            parent->reorderChild(child1, 2);
            parent->reorderChild(child2, 3);
            parent->reorderChild(child3, 1);
        }
    }

    _order++;
}

class RainbowEffect : public ParticleSystemQuad
{
public:
    bool init() override;
    virtual bool initWithTotalParticles(int numberOfParticles) override;
    virtual void update(float dt) override;
};

bool RainbowEffect::init()
{
    return initWithTotalParticles(150);
}

bool RainbowEffect::initWithTotalParticles(int numberOfParticles)
{
    if (ParticleSystemQuad::initWithTotalParticles(numberOfParticles))
    {
        // additive
        setBlendAdditive(false);

        // duration
        setDuration(ParticleSystem::DURATION_INFINITY);

        // Gravity Mode
        setEmitterMode(ParticleSystem::Mode::GRAVITY);

        // Gravity Mode: gravity
        setGravity(Vec2(0.0f, 0.0f));

        // Gravity mode: radial acceleration
        setRadialAccel(0);
        setRadialAccelVar(0);

        // Gravity mode: speed of particles
        setSpeed(120);
        setSpeedVar(0);

        // angle
        setAngle(180);
        setAngleVar(0);

        // emitter position
        auto winSize = Director::getInstance()->getWinSize();
        setPosition(Vec2(winSize.width / 2, winSize.height / 2));
        setPosVar(Vec2::ZERO);

        // life of particles
        setLife(0.5f);
        setLifeVar(0);

        // size, in pixels
        setStartSize(25.0f);
        setStartSizeVar(0);
        setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

        // emits per seconds
        setEmissionRate(getTotalParticles() / getLife());

        // color of particles
        setStartColor(Color4F(Color4B(50, 50, 50, 50)));
        setEndColor(Color4F(Color4B(0, 0, 0, 0)));

        _startColorVar.r = 0.0f;
        _startColorVar.g = 0.0f;
        _startColorVar.b = 0.0f;
        _startColorVar.a = 0.0f;
        _endColorVar.r   = 0.0f;
        _endColorVar.g   = 0.0f;
        _endColorVar.b   = 0.0f;
        _endColorVar.a   = 0.0f;

        setTexture(Director::getInstance()->getTextureCache()->addImage("Images/particles.png"));
        return true;
    }

    return false;
}

void RainbowEffect::update(float dt)
{
    _emitCounter = 0;
    ParticleSystemQuad::update(dt);
}

void Issue1201::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    RainbowEffect* particle = new RainbowEffect();
    particle->initWithTotalParticles(50);

    addChild(particle);

    auto s = Director::getInstance()->getWinSize();

    particle->setPosition(Vec2(s.width / 2, s.height / 2));

    _emitter = particle;
}

std::string Issue1201::title() const
{
    return "Issue 1201. Unfinished";
}

std::string Issue1201::subtitle() const
{
    return "Unfinished test. Ignore it";
}

void MultipleParticleSystems::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    Director::getInstance()->getTextureCache()->addImage("Images/particles.png");

    for (int i = 0; i < 5; i++)
    {
        auto particleSystem = ParticleSystemQuad::create("Particles/SpinningPeas.plist");

        particleSystem->setPosition(Vec2(i * 50.0f, i * 50.0f));

        particleSystem->setPositionType(ParticleSystem::PositionType::GROUPED);
        addChild(particleSystem);
    }

    _emitter = nullptr;
}

std::string MultipleParticleSystems::title() const
{
    return "Multiple particle systems";
}

std::string MultipleParticleSystems::subtitle() const
{
    return "v1.1 test: FPS should be lower than next test";
}

void MultipleParticleSystems::update(float dt)
{
    auto atlas = (LabelAtlas*)getChildByTag(kTagParticleCount);

    unsigned int count = 0;

    for (const auto& child : _children)
    {
        auto item = dynamic_cast<ParticleSystem*>(child);
        if (item != nullptr)
        {
            count += item->getParticleCount();
        }
    }

    char str[100] = {0};
    sprintf(str, "%4d", count);
    atlas->setString(str);
}

// MultipleParticleSystemsBatched

void MultipleParticleSystemsBatched::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    ParticleBatchNode* batchNode = ParticleBatchNode::createWithTexture(nullptr, 3000);

    addChild(batchNode, 1, 2);

    for (int i = 0; i < 5; i++)
    {

        auto particleSystem = ParticleSystemQuad::create("Particles/SpinningPeas.plist");

        particleSystem->setPositionType(ParticleSystem::PositionType::GROUPED);
        particleSystem->setPosition(Vec2(i * 50.0f, i * 50.0f));

        batchNode->setTexture(particleSystem->getTexture());
        batchNode->addChild(particleSystem);
    }

    _emitter = nullptr;
}

void MultipleParticleSystemsBatched::update(float dt)
{
    auto atlas = (LabelAtlas*)getChildByTag(kTagParticleCount);

    int count = 0;

    auto batchNode = getChildByTag(2);
    for (const auto& child : batchNode->getChildren())
    {
        auto item = dynamic_cast<ParticleSystem*>(child);
        if (item != nullptr)
        {
            count += item->getParticleCount();
        }
    }

    char str[50] = {0};
    sprintf(str, "%4d", count);
    atlas->setString(str);
}

std::string MultipleParticleSystemsBatched::title() const
{
    return "Multiple particle systems batched";
}

std::string MultipleParticleSystemsBatched::subtitle() const
{
    return "v1.1 test: should perform better than previous test";
}

// AddAndDeleteParticleSystems

void AddAndDeleteParticleSystems::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    // adds the texture inside the plist to the texture cache
    _batchNode = ParticleBatchNode::createWithTexture((Texture2D*)nullptr, 16000);

    addChild(_batchNode, 1, 2);

    for (int i = 0; i < 6; i++)
    {

        auto particleSystem = ParticleSystemQuad::create("Particles/Spiral.plist");
        _batchNode->setTexture(particleSystem->getTexture());

        particleSystem->setPositionType(ParticleSystem::PositionType::GROUPED);
        particleSystem->setTotalParticles(200);

        particleSystem->setPosition(Vec2(i * 15 + 100.0f, i * 15 + 100.0f));

        unsigned int randZ = rand() % 100;
        _batchNode->addChild(particleSystem, randZ, -1);
    }

    schedule(CC_SCHEDULE_SELECTOR(AddAndDeleteParticleSystems::removeSystem), 0.5f);
    _emitter = nullptr;
}

void AddAndDeleteParticleSystems::removeSystem(float dt)
{
    ssize_t nChildrenCount = _batchNode->getChildren().size();
    if (nChildrenCount > 0)
    {
        CCLOG("remove random system");
        unsigned int uRand = rand() % (nChildrenCount - 1);
        _batchNode->removeChild(_batchNode->getChildren().at(uRand), true);

        auto particleSystem = ParticleSystemQuad::create("Particles/Spiral.plist");
        // add new

        particleSystem->setPositionType(ParticleSystem::PositionType::GROUPED);
        particleSystem->setTotalParticles(200);

        particleSystem->setPosition(Vec2(rand() % 300, rand() % 400));

        CCLOG("add a new system");
        unsigned int randZ = rand() % 100;
        _batchNode->addChild(particleSystem, randZ, -1);
    }
}

void AddAndDeleteParticleSystems::update(float dt)
{
    auto atlas = (LabelAtlas*)getChildByTag(kTagParticleCount);

    int count = 0;

    auto batchNode = getChildByTag(2);
    for (const auto& child : batchNode->getChildren())
    {
        auto item = dynamic_cast<ParticleSystem*>(child);
        if (item != nullptr)
        {
            count += item->getParticleCount();
        }
    }

    char str[100] = {0};
    sprintf(str, "%4d", count);
    atlas->setString(str);
}

std::string AddAndDeleteParticleSystems::title() const
{
    return "Add and remove Particle System";
}

std::string AddAndDeleteParticleSystems::subtitle() const
{
    return "v1.1 test: every 2 sec 1 system disappear, 1 appears";
}

// ReorderParticleSystems

void ReorderParticleSystems::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    _batchNode = ParticleBatchNode::create("Images/stars-grayscale.png", 3000);

    addChild(_batchNode, 1, 2);

    for (int i = 0; i < 3; i++)
    {

        auto particleSystem = ParticleSystemQuad::createWithTotalParticles(200);
        particleSystem->retain();
        particleSystem->setTexture(_batchNode->getTexture());

        // duration
        particleSystem->setDuration(ParticleSystem::DURATION_INFINITY);

        // radius mode
        particleSystem->setEmitterMode(ParticleSystem::Mode::RADIUS);

        // radius mode: 100 pixels from center
        particleSystem->setStartRadius(100);
        particleSystem->setStartRadiusVar(0);
        particleSystem->setEndRadius(ParticleSystem::START_RADIUS_EQUAL_TO_END_RADIUS);
        particleSystem->setEndRadiusVar(0);  // not used when start == end

        // radius mode: degrees per second
        // 45 * 4 seconds of life = 180 degrees
        particleSystem->setRotatePerSecond(45);
        particleSystem->setRotatePerSecondVar(0);

        // angle
        particleSystem->setAngle(90);
        particleSystem->setAngleVar(0);

        // emitter position
        particleSystem->setPosVar(Vec2::ZERO);

        // life of particles
        particleSystem->setLife(4);
        particleSystem->setLifeVar(0);

        // spin of particles
        particleSystem->setStartSpin(0);
        particleSystem->setStartSpinVar(0);
        particleSystem->setEndSpin(0);
        particleSystem->setEndSpinVar(0);

        // color of particles
        float color[3] = {0, 0, 0};
        color[i]       = 1;
        Color4F startColor(color[0], color[1], color[2], 1.0f);
        particleSystem->setStartColor(startColor);

        Color4F startColorVar(0, 0, 0, 0);
        particleSystem->setStartColorVar(startColorVar);

        Color4F endColor = startColor;
        particleSystem->setEndColor(endColor);

        Color4F endColorVar = startColorVar;
        particleSystem->setEndColorVar(endColorVar);

        // size, in pixels
        particleSystem->setStartSize(32);
        particleSystem->setStartSizeVar(0);
        particleSystem->setEndSize(ParticleSystem::START_SIZE_EQUAL_TO_END_SIZE);

        // emits per second
        particleSystem->setEmissionRate(particleSystem->getTotalParticles() / particleSystem->getLife());

        // additive

        particleSystem->setPosition(Vec2(i * 10 + 120.0f, 200.0f));

        _batchNode->addChild(particleSystem);
        particleSystem->setPositionType(ParticleSystem::PositionType::FREE);

        particleSystem->release();

        //[pBNode addChild:particleSystem z:10 tag:0);
    }

    schedule(CC_SCHEDULE_SELECTOR(ReorderParticleSystems::reorderSystem), 2.0f);
    _emitter = nullptr;
}

void ReorderParticleSystems::reorderSystem(float time)
{
    auto system = static_cast<ParticleSystem*>(_batchNode->getChildren().at(1));
    _batchNode->reorderChild(system, system->getLocalZOrder() - 1);
}

void ReorderParticleSystems::update(float dt)
{
    auto atlas = static_cast<LabelAtlas*>(getChildByTag(kTagParticleCount));

    int count = 0;

    auto batchNode = getChildByTag(2);
    for (const auto& child : batchNode->getChildren())
    {
        auto item = dynamic_cast<ParticleSystem*>(child);
        if (item != nullptr)
        {
            count += item->getParticleCount();
        }
    }
    char str[100] = {0};
    sprintf(str, "%4d", count);
    atlas->setString(str);
}

std::string ReorderParticleSystems::title() const
{
    return "reorder systems";
}

std::string ReorderParticleSystems::subtitle() const
{
    return "changes every 2 seconds";
}

// PremultipliedAlphaTest

std::string PremultipliedAlphaTest::title() const
{
    return "premultiplied alpha and readd child test";
}

std::string PremultipliedAlphaTest::subtitle() const
{
    return "no black halo, particles should fade out\n animation should be normal";
}

void PremultipliedAlphaTest::readdParticle(float delta)
{
    if (_hasEmitter)
    {
        _emitter->removeFromParent();
    }
    else
    {
        this->addChild(_emitter);
    }

    _hasEmitter = !_hasEmitter;
}

void PremultipliedAlphaTest::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLUE);
    this->removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::create("Particles/BoilingFoam.plist");
    _emitter->retain();
    // Particle Designer "normal" blend func causes black halo on premul textures (ignores multiplication)
    // this->emitter.blendFunc = (BlendFunc){ BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA };

    // Cocos2d "normal" blend func for premul causes alpha to be ignored (oversaturates colors)
    _emitter->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);

    CCASSERT(_emitter->isOpacityModifyRGB(), "Particle texture does not have premultiplied alpha, test is useless");

    // Toggle next line to see old behavior
    //	this->emitter.opacityModifyRGB = NO;

    _emitter->setStartColor(Color4F(1.0f, 1.0f, 1.0f, 1.0f));
    _emitter->setEndColor(Color4F(1.0f, 1.0f, 1.0f, 0.0f));
    _emitter->setStartColorVar(Color4F(0.0f, 0.0f, 0.0f, 0.0f));
    _emitter->setEndColorVar(Color4F(0.0f, 0.0f, 0.0f, 0.0f));

    this->addChild(_emitter, 10);
    _hasEmitter = true;

    schedule(CC_SCHEDULE_SELECTOR(PremultipliedAlphaTest::readdParticle), 1.0f);
}

// PremultipliedAlphaTest2

void PremultipliedAlphaTest2::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    this->removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::create("Particles/TestPremultipliedAlpha.plist");
    _emitter->retain();
    this->addChild(_emitter, 10);
}

std::string PremultipliedAlphaTest2::title() const
{
    return "premultiplied alpha 2";
}

std::string PremultipliedAlphaTest2::subtitle() const
{
    return "Arrows should be faded";
}

// Issue3990

void Issue3990::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    this->removeChild(_background, true);
    _background = nullptr;

    _emitter = ParticleSystemQuad::create("Particles/Spiral.plist");

    _emitter->setPositionType(ParticleSystem::PositionType::GROUPED);
    _emitter->setTotalParticles(1000);
    _emitter->setEmissionRate(_emitter->getTotalParticles() / _emitter->getLife());
    _emitter->setPosition(VisibleRect::center());

    _emitter->retain();
    this->addChild(_emitter, 10);
}

std::string Issue3990::title() const
{
    return "Issue3990, setTotalParticle should work";
}

std::string Issue3990::subtitle() const
{
    return "Show '998' or '999' at bottom right side";
}

// ParticleVisibleTest
void ParticleVisibleTest::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleFireworks::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage(s_stars1));

    schedule(CC_SCHEDULE_SELECTOR(ParticleVisibleTest::callback), 1);

    setEmitterPosition();
}

std::string ParticleVisibleTest::title() const
{
    return "Issue4573";
}

std::string ParticleVisibleTest::subtitle() const
{
    return "Visible enable/disable";
}

void ParticleVisibleTest::callback(float delta)
{
    _emitter->setVisible(!_emitter->isVisible());
}

//
// ParticleAutoBatching
//
void ParticleAutoBatching::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    this->removeChild(_background, true);
    _background = nullptr;

    Size s = Director::getInstance()->getWinSize();

    for (int i = 0; i < 10; i++)
    {
        auto particle = ParticleSystemQuad::create("Particles/SmallSun.plist");
        particle->setTotalParticles(100);
        particle->setPosition(Vec2(i * s.width / 11, s.height / 2));
        this->addChild(particle, 10);
    }
}

std::string ParticleAutoBatching::title() const
{
    return "AutoBatching";
}

std::string ParticleAutoBatching::subtitle() const
{
    return "All 10 particles should be drawin in one batch";
}

//
// ParticleResetTotalParticles
//
void ParticleResetTotalParticles::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    auto p = ParticleFire::createWithTotalParticles(10);
    this->addChild(p);

    auto add = MenuItemFont::create("add 10 particles",
                                    [p](Ref*) -> void { p->setTotalParticles(p->getTotalParticles() + 10); });
    add->setFontSizeObj(20);
    add->setPosition(Vec2(0, 25));

    auto remove = MenuItemFont::create("remove 10 particles", [p](Ref*) -> void {
        int count = p->getTotalParticles() - 10;
        if (count < 0)
        {
            count = 0;
        }
        p->setTotalParticles(count);
    });
    remove->setPosition(Vec2(0, -25));
    remove->setFontSizeObj(20);

    auto menu = Menu::create(add, remove, nullptr);
    menu->setPosition(Vec2(VisibleRect::center()));
    this->addChild(menu);
}

std::string ParticleResetTotalParticles::title() const
{
    return "reset total particles";
}

std::string ParticleResetTotalParticles::subtitle() const
{
    return "it should work as well";
}

void ParticleIssue12310::onEnter()
{
    ParticleDemo::onEnter();

    _color->setColor(Color3B::BLACK);
    removeChild(_background, true);
    _background = nullptr;

    auto winSize = Director::getInstance()->getWinSize();

    auto particle = ParticleSystemQuad::create("Particles/BoilingFoam.plist");
    particle->setPosition(Vec2(winSize.width * 0.35f, winSize.height * 0.5f));
    addChild(particle);

    _emitter = particle;
    _emitter->retain();

    auto particle2 = ParticleSystemQuad::create("Particles/BoilingFoamStar.plist");
    particle2->setPosition(Vec2(winSize.width * 0.65f, winSize.height * 0.5f));
    addChild(particle2);
}

std::string ParticleIssue12310::subtitle() const
{
    return "You should see two Particle Emitters using different texture.";
}

//------------------------------------------------------------------
//
// ParticleSpriteFrame
//
//------------------------------------------------------------------
void ParticleSpriteFrame::onEnter()
{
    ParticleDemo::onEnter();

    _emitter = ParticleSmoke::create();
    _emitter->retain();
    _background->addChild(_emitter, 10);

    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Particles/SpriteFrame.plist");

    _emitter->setDisplayFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("dot.png"));

    setEmitterPosition();
}

std::string ParticleSpriteFrame::title() const
{
    return "Particle from SpriteFrame";
}

std::string ParticleSpriteFrame::subtitle() const
{
    return "Should not use entire texture atlas";
}
