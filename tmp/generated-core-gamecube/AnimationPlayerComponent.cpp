#ifdef DrawText
#undef DrawText
#endif
#include <cmath>
#include "AnimationPlayerComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "UpdateComponent.hpp"
#include "AnimationClipAsset.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "AnimationClipEvaluator.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::AnimationClipAsset* AnimationPlayerComponent::get_CurrentClip()
{
return this->currentClip;}

float AnimationPlayerComponent::get_CurrentTime()
{
return this->currentTime;}

float AnimationPlayerComponent::get_FrameDeltaTime()
{
return this->frameDeltaTime;}

void AnimationPlayerComponent::set_FrameDeltaTime(float value)
{
this->frameDeltaTime = value;
}

bool AnimationPlayerComponent::get_IsPaused()
{
return this->isPaused;}

bool AnimationPlayerComponent::get_IsPlaying()
{
return this->isPlaying;}

void AnimationPlayerComponent::Advance(float deltaTime)
{
    if (!this->isPlaying || this->isPaused || this->currentClip == nullptr)
    {
return;    }
const float nextTime = this->currentTime + deltaTime;
    if (!this->loop && nextTime >= this->currentClip->get_Duration())
    {
this->currentTime = this->currentClip->get_Duration();
this->ApplyCurrentPose();
this->CompletePlayback();
return;    }
this->currentTime = this->ResolvePlaybackTime(nextTime);
this->ApplyCurrentPose();
}

AnimationPlayerComponent::AnimationPlayerComponent() : baseLocalOrientation(), baseLocalPosition(), baseLocalScale(), currentClip(), currentTime(), frameDeltaTime(), isPaused(), isPlaying(), loop()
{
this->frameDeltaTime = 1.0f / 60.0f;
}

void AnimationPlayerComponent::Pause()
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot pause animation playback when no clip is active.");
    }
this->isPaused = true;
this->isPlaying = false;
}

void AnimationPlayerComponent::Play(::AnimationClipAsset* clip, bool shouldLoop)
{
    if (clip == nullptr)
    {
throw new ArgumentNullException("clip");
    }
else     if (Parent == nullptr)
    {
throw new InvalidOperationException("AnimationPlayerComponent must be added to an entity before playback can begin.");
    }
this->ValidateClip(clip);
this->currentClip = clip;
this->currentTime = 0.0f;
this->loop = shouldLoop;
this->isPlaying = true;
this->isPaused = false;
this->baseLocalPosition = Parent->get_LocalPosition();
this->baseLocalScale = Parent->get_LocalScale();
this->baseLocalOrientation = Parent->get_LocalOrientation();
this->ApplyCurrentPose();
    if (this->currentClip->get_Duration() <= 0.0f)
    {
this->CompletePlayback();
    }
}

void AnimationPlayerComponent::Resume()
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot resume animation playback when no clip is active.");
    }
this->isPaused = false;
this->isPlaying = true;
}

void AnimationPlayerComponent::Seek(float time)
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot seek animation playback when no clip is active.");
    }
this->currentTime = this->ResolvePlaybackTime(time);
this->ApplyCurrentPose();
}

void AnimationPlayerComponent::Stop()
{
    if (Parent != nullptr)
    {
Parent->set_LocalPosition(this->baseLocalPosition);
Parent->set_LocalScale(this->baseLocalScale);
Parent->set_LocalOrientation(this->baseLocalOrientation);
    }
this->currentClip = nullptr;
this->currentTime = 0.0f;
this->isPlaying = false;
this->isPaused = false;
this->loop = false;
}

void AnimationPlayerComponent::Update()
{
UpdateComponent::Update();
this->Advance(this->frameDeltaTime);
}

uint8_t AnimationPlayerComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void AnimationPlayerComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* AnimationPlayerComponent::get_Parent()
{
return this->Component::get_Parent();
}

void AnimationPlayerComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void AnimationPlayerComponent::ApplyCurrentPose()
{
    if (Parent == nullptr || this->currentClip == nullptr)
    {
return;    }
::float3 resolvedPosition = this->baseLocalPosition;
    if (this->currentClip->get_PositionTracks()->Length == 1)
    {
resolvedPosition = AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->get_PositionTracks())[0], this->currentTime);
    }
    if (this->currentClip->get_PositionOffsetTracks()->Length == 1)
    {
resolvedPosition = resolvedPosition + AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->get_PositionOffsetTracks())[0], this->currentTime);
    }
::float3 resolvedScale = this->baseLocalScale;
    if (this->currentClip->get_ScaleTracks()->Length == 1)
    {
resolvedScale = AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->get_ScaleTracks())[0], this->currentTime);
    }
::float4 resolvedOrientation = this->baseLocalOrientation;
    if (this->currentClip->get_RotationTracks()->Length == 1)
    {
resolvedOrientation = AnimationClipEvaluator::EvaluateRotationTrack((*this->currentClip->get_RotationTracks())[0], this->currentTime);
    }
Parent->set_LocalPosition(resolvedPosition);
Parent->set_LocalScale(resolvedScale);
Parent->set_LocalOrientation(resolvedOrientation);
}

void AnimationPlayerComponent::CompletePlayback()
{
this->isPlaying = false;
this->isPaused = false;
}

float AnimationPlayerComponent::ResolvePlaybackTime(float time)
{
    if (this->currentClip == nullptr || this->currentClip->get_Duration() <= 0.0f)
    {
return 0.0f;    }
else     if (this->loop)
    {
const double duration = this->currentClip->get_Duration();
double wrapped = std::fmod(static_cast<double>(time), duration);
    if (wrapped < 0.0)
    {
wrapped += duration;
    }
return static_cast<float>(wrapped);    }
else     if (time <= 0.0f)
    {
return 0.0f;    }
else     if (time >= this->currentClip->get_Duration())
    {
return this->currentClip->get_Duration();    }
return time;}

void AnimationPlayerComponent::ValidateClip(::AnimationClipAsset* clip)
{
    if (clip->get_Duration() < 0.0f)
    {
throw new InvalidOperationException("Animation clips cannot declare a negative duration.");
    }
else     if (clip->get_PositionTracks()->Length > 1 || clip->get_PositionOffsetTracks()->Length > 1 || clip->get_ScaleTracks()->Length > 1 || clip->get_RotationTracks()->Length > 1)
    {
throw new InvalidOperationException("Animation clips can currently bind only one track per transform channel.");
    }
}

