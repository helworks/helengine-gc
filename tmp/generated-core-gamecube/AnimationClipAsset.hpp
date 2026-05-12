#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Asset;
class PositionOffsetKeyframeTrackAsset;
class PositionKeyframeTrackAsset;
class RotationKeyframeTrackAsset;
class ScaleKeyframeTrackAsset;

#include "Asset.hpp"
#include "runtime/array.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "runtime/array.hpp"
#include "ScaleKeyframeTrackAsset.hpp"

class AnimationClipAsset : public Asset
{
public:
    virtual ~AnimationClipAsset() = default;

    AnimationClipAsset();

    float Duration;

    float get_Duration();
    void set_Duration(float value);

    Array<::PositionOffsetKeyframeTrackAsset*>* PositionOffsetTracks;

    Array<::PositionOffsetKeyframeTrackAsset*>* get_PositionOffsetTracks();
    void set_PositionOffsetTracks(Array<::PositionOffsetKeyframeTrackAsset*>* value);

    Array<::PositionKeyframeTrackAsset*>* PositionTracks;

    Array<::PositionKeyframeTrackAsset*>* get_PositionTracks();
    void set_PositionTracks(Array<::PositionKeyframeTrackAsset*>* value);

    Array<::RotationKeyframeTrackAsset*>* RotationTracks;

    Array<::RotationKeyframeTrackAsset*>* get_RotationTracks();
    void set_RotationTracks(Array<::RotationKeyframeTrackAsset*>* value);

    Array<::ScaleKeyframeTrackAsset*>* ScaleTracks;

    Array<::ScaleKeyframeTrackAsset*>* get_ScaleTracks();
    void set_ScaleTracks(Array<::ScaleKeyframeTrackAsset*>* value);

    std::string get_Id();

    void set_Id(std::string value);
};
