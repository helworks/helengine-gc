#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class PositionKeyframeAsset;
class float3;
class RotationKeyframeAsset;
class float4;
class PositionKeyframeTrackAsset;
class PositionOffsetKeyframeTrackAsset;
class ScaleKeyframeTrackAsset;
class RotationKeyframeTrackAsset;

#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "PositionKeyframeAsset.hpp"
#include "float3.hpp"
#include "RotationKeyframeAsset.hpp"
#include "float4.hpp"
#include "float3.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "float4.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"

class AnimationClipEvaluator
{
public:
    virtual ~AnimationClipEvaluator() = default;

    static ::float3 EvaluatePositionTrack(::PositionKeyframeTrackAsset* track, float time);

    static ::float3 EvaluatePositionTrack(::PositionOffsetKeyframeTrackAsset* track, float time);

    static ::float3 EvaluatePositionTrack(::ScaleKeyframeTrackAsset* track, float time);

    static ::float4 EvaluateRotationTrack(::RotationKeyframeTrackAsset* track, float time);
private:
    static ::float3 EvaluatePositionKeyframes(Array<::PositionKeyframeAsset*>* keyframes, float time, std::string emptyTrackMessage);

    static ::float3 InterpolatePositionKeyframes(::PositionKeyframeAsset* startKeyframe, ::PositionKeyframeAsset* endKeyframe, float time);

    static ::float4 InterpolateRotationKeyframes(::RotationKeyframeAsset* startKeyframe, ::RotationKeyframeAsset* endKeyframe, float time);
};
