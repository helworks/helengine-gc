#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameLightClassifier.hpp"
#include "runtime/native_exceptions.hpp"
#include "LightComponent.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"

::RenderFrameLightSubmission* RenderFrameLightClassifier::Classify(::LightComponent* light)
{
    if (light == nullptr)
    {
throw new ArgumentNullException("light");
    }
int32_t importance = 0;
    if (light->get_ShadowsEnabled())
    {
importance += 1000;
    }
importance += static_cast<int32_t>(Math::Round(light->get_Intensity() * 100.0, MidpointRounding::AwayFromZero));
return new ::RenderFrameLightSubmission(light, importance);}

