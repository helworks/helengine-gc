#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "DepthPrepassMode.hpp"
#include "PostProcessTier.hpp"
#include "Ps2DepthHandlerMode.hpp"
#include "runtime/native_string.hpp"

class RuntimeGraphicsRendererManifest
{
public:
    virtual ~RuntimeGraphicsRendererManifest() = default;

    ::DepthPrepassMode DepthPrepassMode;

    ::DepthPrepassMode get_DepthPrepassMode();

    bool HdrEnabled;

    bool get_HdrEnabled();

    ::PostProcessTier PostProcessTier;

    ::PostProcessTier get_PostProcessTier();

    ::Ps2DepthHandlerMode Ps2DepthHandlerMode;

    ::Ps2DepthHandlerMode get_Ps2DepthHandlerMode();

    std::string ShadowQualityTier;

    std::string get_ShadowQualityTier();

    RuntimeGraphicsRendererManifest(::DepthPrepassMode depthPrepassMode, std::string shadowQualityTier, bool hdrEnabled, ::PostProcessTier postProcessTier, ::Ps2DepthHandlerMode ps2DepthHandlerMode);
};
