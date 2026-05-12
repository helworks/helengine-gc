#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameLightSubmission.hpp"
#include "runtime/native_exceptions.hpp"

int32_t RenderFrameLightSubmission::get_Importance()
{
return this->Importance;
}

::LightComponent* RenderFrameLightSubmission::get_Light()
{
return this->Light;
}

::LightType RenderFrameLightSubmission::get_LightType()
{
return this->Light->get_LightType();
}

RenderFrameLightSubmission::RenderFrameLightSubmission(::LightComponent* light) : Importance(0), Light()
{
    if (light == nullptr)
    {
throw new ArgumentNullException("light");
    }
this->Light = light;
}

RenderFrameLightSubmission::RenderFrameLightSubmission(::LightComponent* light, int32_t importance) : Importance(0), Light()
{
this->Light = (light != nullptr ? light : throw new ArgumentNullException("light"));
this->Importance = importance;
}

