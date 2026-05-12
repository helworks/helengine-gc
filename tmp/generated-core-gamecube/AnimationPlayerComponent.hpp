#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class AnimationClipAsset;
class float3;
class float4;

#include "UpdateComponent.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "UpdateComponent.hpp"
#include "AnimationClipAsset.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "AnimationClipAsset.hpp"

class AnimationPlayerComponent : public UpdateComponent
{
public:
    virtual ~AnimationPlayerComponent() = default;

    ::AnimationClipAsset* get_CurrentClip();

    float get_CurrentTime();

    float get_FrameDeltaTime();

    void set_FrameDeltaTime(float value);

    bool get_IsPaused();

    bool get_IsPlaying();

    void Advance(float deltaTime);

    AnimationPlayerComponent();

    void Pause();

    void Play(::AnimationClipAsset* clip, bool shouldLoop);

    void Resume();

    void Seek(float time);

    void Stop();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    ::float4 baseLocalOrientation;

    ::float3 baseLocalPosition;

    ::float3 baseLocalScale;

    ::AnimationClipAsset* currentClip;

    float currentTime;

    float frameDeltaTime;

    bool isPaused;

    bool isPlaying;

    bool loop;

    void ApplyCurrentPose();

    void CompletePlayback();

    float ResolvePlaybackTime(float time);

    void ValidateClip(::AnimationClipAsset* clip);
};
