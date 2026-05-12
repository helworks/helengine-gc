#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;

#include "AnimationInterpolationMode.hpp"
#include "float4.hpp"

class RotationKeyframeAsset
{
public:
    virtual ~RotationKeyframeAsset() = default;

    ::AnimationInterpolationMode InterpolationMode;

    ::AnimationInterpolationMode get_InterpolationMode();
    void set_InterpolationMode(::AnimationInterpolationMode value);

    float Time;

    float get_Time();
    void set_Time(float value);

    ::float4 Value;

    ::float4 get_Value();
    void set_Value(::float4 value);

    RotationKeyframeAsset();

    RotationKeyframeAsset(float time, ::float4 value, ::AnimationInterpolationMode interpolationMode);
};
