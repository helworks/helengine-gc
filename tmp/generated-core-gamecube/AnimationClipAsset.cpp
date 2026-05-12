#ifdef DrawText
#undef DrawText
#endif
#include "AnimationClipAsset.hpp"
#include "runtime/array.hpp"

AnimationClipAsset::AnimationClipAsset() : Duration(), PositionOffsetTracks(Array<PositionOffsetKeyframeTrackAsset*>::Empty()), PositionTracks(Array<PositionKeyframeTrackAsset*>::Empty()), RotationTracks(Array<RotationKeyframeTrackAsset*>::Empty()), ScaleTracks(Array<ScaleKeyframeTrackAsset*>::Empty())
{
}

float AnimationClipAsset::get_Duration()
{
return this->Duration;
}

void AnimationClipAsset::set_Duration(float value)
{
this->Duration = value;
}

Array<::PositionOffsetKeyframeTrackAsset*>* AnimationClipAsset::get_PositionOffsetTracks()
{
return this->PositionOffsetTracks;
}

void AnimationClipAsset::set_PositionOffsetTracks(Array<::PositionOffsetKeyframeTrackAsset*>* value)
{
this->PositionOffsetTracks = value;
}

Array<::PositionKeyframeTrackAsset*>* AnimationClipAsset::get_PositionTracks()
{
return this->PositionTracks;
}

void AnimationClipAsset::set_PositionTracks(Array<::PositionKeyframeTrackAsset*>* value)
{
this->PositionTracks = value;
}

Array<::RotationKeyframeTrackAsset*>* AnimationClipAsset::get_RotationTracks()
{
return this->RotationTracks;
}

void AnimationClipAsset::set_RotationTracks(Array<::RotationKeyframeTrackAsset*>* value)
{
this->RotationTracks = value;
}

Array<::ScaleKeyframeTrackAsset*>* AnimationClipAsset::get_ScaleTracks()
{
return this->ScaleTracks;
}

void AnimationClipAsset::set_ScaleTracks(Array<::ScaleKeyframeTrackAsset*>* value)
{
this->ScaleTracks = value;
}

std::string AnimationClipAsset::get_Id()
{
return this->Asset::get_Id();
}

void AnimationClipAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}

