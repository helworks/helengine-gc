#ifdef DrawText
#undef DrawText
#endif
#include "AnimationClipEvaluator.hpp"
#include "runtime/native_exceptions.hpp"
#include "PositionKeyframeAsset.hpp"
#include "float3.hpp"
#include "RotationKeyframeAsset.hpp"
#include "float4.hpp"
#include "AnimationInterpolationMode.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::PositionKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
else     if (track->get_Keyframes() == nullptr || track->get_Keyframes()->Length == 0)
    {
throw new InvalidOperationException("Animation position tracks must contain at least one keyframe.");
    }
::PositionKeyframeAsset *firstKeyframe = (*track->get_Keyframes())[0];
    if (time <= firstKeyframe->get_Time())
    {
return firstKeyframe->get_Value();    }
::PositionKeyframeAsset *lastKeyframe = (*track->get_Keyframes())[track->get_Keyframes()->Length - 1];
    if (time >= lastKeyframe->get_Time())
    {
return lastKeyframe->get_Value();    }
for (int32_t i = 0; i < track->get_Keyframes()->Length - 1; i++) {
::PositionKeyframeAsset *startKeyframe = (*track->get_Keyframes())[i];
::PositionKeyframeAsset *endKeyframe = (*track->get_Keyframes())[i + 1];
    if (time <= endKeyframe->get_Time())
    {
return InterpolatePositionKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->get_Value();}

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::PositionOffsetKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
return EvaluatePositionKeyframes(track->get_Keyframes(), time, "Animation additive-position tracks must contain at least one keyframe.");}

::float3 AnimationClipEvaluator::EvaluatePositionTrack(::ScaleKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
return EvaluatePositionKeyframes(track->get_Keyframes(), time, "Animation scale tracks must contain at least one keyframe.");}

::float4 AnimationClipEvaluator::EvaluateRotationTrack(::RotationKeyframeTrackAsset* track, float time)
{
    if (track == nullptr)
    {
throw new ArgumentNullException("track");
    }
else     if (track->get_Keyframes() == nullptr || track->get_Keyframes()->Length == 0)
    {
throw new InvalidOperationException("Animation rotation tracks must contain at least one keyframe.");
    }
::RotationKeyframeAsset *firstKeyframe = (*track->get_Keyframes())[0];
    if (time <= firstKeyframe->get_Time())
    {
return firstKeyframe->get_Value();    }
::RotationKeyframeAsset *lastKeyframe = (*track->get_Keyframes())[track->get_Keyframes()->Length - 1];
    if (time >= lastKeyframe->get_Time())
    {
return lastKeyframe->get_Value();    }
for (int32_t i = 0; i < track->get_Keyframes()->Length - 1; i++) {
::RotationKeyframeAsset *startKeyframe = (*track->get_Keyframes())[i];
::RotationKeyframeAsset *endKeyframe = (*track->get_Keyframes())[i + 1];
    if (time <= endKeyframe->get_Time())
    {
return InterpolateRotationKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->get_Value();}

::float3 AnimationClipEvaluator::EvaluatePositionKeyframes(Array<::PositionKeyframeAsset*>* keyframes, float time, std::string emptyTrackMessage)
{
    if (keyframes == nullptr || keyframes->Length == 0)
    {
throw new InvalidOperationException(emptyTrackMessage);
    }
::PositionKeyframeAsset *firstKeyframe = (*keyframes)[0];
    if (time <= firstKeyframe->get_Time())
    {
return firstKeyframe->get_Value();    }
::PositionKeyframeAsset *lastKeyframe = (*keyframes)[keyframes->Length - 1];
    if (time >= lastKeyframe->get_Time())
    {
return lastKeyframe->get_Value();    }
for (int32_t i = 0; i < keyframes->Length - 1; i++) {
::PositionKeyframeAsset *startKeyframe = (*keyframes)[i];
::PositionKeyframeAsset *endKeyframe = (*keyframes)[i + 1];
    if (time <= endKeyframe->get_Time())
    {
return InterpolatePositionKeyframes(startKeyframe, endKeyframe, time);    }
}
return lastKeyframe->get_Value();}

::float3 AnimationClipEvaluator::InterpolatePositionKeyframes(::PositionKeyframeAsset* startKeyframe, ::PositionKeyframeAsset* endKeyframe, float time)
{
    if (endKeyframe->get_Time() <= startKeyframe->get_Time())
    {
return endKeyframe->get_Value();    }
else     if (endKeyframe->get_InterpolationMode() == AnimationInterpolationMode::Step)
    {
return startKeyframe->get_Value();    }
const float amount = (time - startKeyframe->get_Time()) / (endKeyframe->get_Time() - startKeyframe->get_Time());
return float3::Lerp(startKeyframe->get_Value(), endKeyframe->get_Value(), amount);}

::float4 AnimationClipEvaluator::InterpolateRotationKeyframes(::RotationKeyframeAsset* startKeyframe, ::RotationKeyframeAsset* endKeyframe, float time)
{
    if (endKeyframe->get_Time() <= startKeyframe->get_Time())
    {
return endKeyframe->get_Value();    }
else     if (endKeyframe->get_InterpolationMode() == AnimationInterpolationMode::Step)
    {
return startKeyframe->get_Value();    }
const float amount = (time - startKeyframe->get_Time()) / (endKeyframe->get_Time() - startKeyframe->get_Time());
return float4::Lerp(startKeyframe->get_Value(), endKeyframe->get_Value(), amount);}

