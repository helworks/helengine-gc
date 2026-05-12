#ifdef DrawText
#undef DrawText
#endif
#include "PositionKeyframeAsset.hpp"

::AnimationInterpolationMode PositionKeyframeAsset::get_InterpolationMode()
{
return this->InterpolationMode;
}

void PositionKeyframeAsset::set_InterpolationMode(::AnimationInterpolationMode value)
{
this->InterpolationMode = value;
}

float PositionKeyframeAsset::get_Time()
{
return this->Time;
}

void PositionKeyframeAsset::set_Time(float value)
{
this->Time = value;
}

::float3 PositionKeyframeAsset::get_Value()
{
return this->Value;
}

void PositionKeyframeAsset::set_Value(::float3 value)
{
this->Value = value;
}

PositionKeyframeAsset::PositionKeyframeAsset() : InterpolationMode(), Time(), Value()
{
}

PositionKeyframeAsset::PositionKeyframeAsset(float time, ::float3 value, ::AnimationInterpolationMode interpolationMode) : InterpolationMode(), Time(), Value()
{
this->set_Time(time);
this->set_Value(value);
this->set_InterpolationMode(interpolationMode);
}

