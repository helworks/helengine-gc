#ifdef DrawText
#undef DrawText
#endif
#include "RotationKeyframeAsset.hpp"

::AnimationInterpolationMode RotationKeyframeAsset::get_InterpolationMode()
{
return this->InterpolationMode;
}

void RotationKeyframeAsset::set_InterpolationMode(::AnimationInterpolationMode value)
{
this->InterpolationMode = value;
}

float RotationKeyframeAsset::get_Time()
{
return this->Time;
}

void RotationKeyframeAsset::set_Time(float value)
{
this->Time = value;
}

::float4 RotationKeyframeAsset::get_Value()
{
return this->Value;
}

void RotationKeyframeAsset::set_Value(::float4 value)
{
this->Value = value;
}

RotationKeyframeAsset::RotationKeyframeAsset() : InterpolationMode(), Time(), Value()
{
}

RotationKeyframeAsset::RotationKeyframeAsset(float time, ::float4 value, ::AnimationInterpolationMode interpolationMode) : InterpolationMode(), Time(), Value()
{
this->set_Time(time);
this->set_Value(value);
this->set_InterpolationMode(interpolationMode);
}

