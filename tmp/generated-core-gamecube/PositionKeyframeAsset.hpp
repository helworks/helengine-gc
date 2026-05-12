#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float3;

#include "AnimationInterpolationMode.hpp"
#include "float3.hpp"

class PositionKeyframeAsset
{
public:
    virtual ~PositionKeyframeAsset() = default;

    ::AnimationInterpolationMode InterpolationMode;

    ::AnimationInterpolationMode get_InterpolationMode();
    void set_InterpolationMode(::AnimationInterpolationMode value);

    float Time;

    float get_Time();
    void set_Time(float value);

    ::float3 Value;

    ::float3 get_Value();
    void set_Value(::float3 value);

    PositionKeyframeAsset();

    PositionKeyframeAsset(float time, ::float3 value, ::AnimationInterpolationMode interpolationMode);
};
