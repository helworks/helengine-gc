#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeGraphicsRendererManifest.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

::DepthPrepassMode RuntimeGraphicsRendererManifest::get_DepthPrepassMode()
{
return this->DepthPrepassMode;
}

bool RuntimeGraphicsRendererManifest::get_HdrEnabled()
{
return this->HdrEnabled;
}

::PostProcessTier RuntimeGraphicsRendererManifest::get_PostProcessTier()
{
return this->PostProcessTier;
}

::Ps2DepthHandlerMode RuntimeGraphicsRendererManifest::get_Ps2DepthHandlerMode()
{
return this->Ps2DepthHandlerMode;
}

std::string RuntimeGraphicsRendererManifest::get_ShadowQualityTier()
{
return this->ShadowQualityTier;
}

RuntimeGraphicsRendererManifest::RuntimeGraphicsRendererManifest(::DepthPrepassMode depthPrepassMode, std::string shadowQualityTier, bool hdrEnabled, ::PostProcessTier postProcessTier, ::Ps2DepthHandlerMode ps2DepthHandlerMode) : DepthPrepassMode(), HdrEnabled(), PostProcessTier(), Ps2DepthHandlerMode(), ShadowQualityTier()
{
    if (String::IsNullOrWhiteSpace(shadowQualityTier))
    {
throw ([&]() {
auto __ctor_arg_00000129 = "Shadow quality tier must be provided.";
auto __ctor_arg_0000012A = "shadowQualityTier";
return new ArgumentException(__ctor_arg_00000129, __ctor_arg_0000012A);
})();
    }
this->DepthPrepassMode = depthPrepassMode;
this->ShadowQualityTier = shadowQualityTier;
this->HdrEnabled = hdrEnabled;
this->PostProcessTier = postProcessTier;
this->Ps2DepthHandlerMode = ps2DepthHandlerMode;
}

