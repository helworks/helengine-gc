#ifdef DrawText
#undef DrawText
#endif
#include "LightDirectionUtility.hpp"
#include "runtime/native_exceptions.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "LightType.hpp"
#include "runtime/native_exceptions.hpp"

::float3 LightDirectionUtility::get_AuthoredForwardAxis()
{
return ::float3(0.0f, 0.0f, -1.0f);
}

::float3 LightDirectionUtility::GetEntityForwardDirection(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
return float4::RotateVector(get_AuthoredForwardAxis(), entity->get_Orientation());}

::float3 LightDirectionUtility::GetLightDirection(::LightComponent* lightComponent)
{
    if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
else     if (lightComponent->get_LightType() == LightType::Ambient)
    {
throw new InvalidOperationException("Ambient lights do not define a world-space light direction.");
    }
else     if (lightComponent->Parent == nullptr)
    {
throw new InvalidOperationException("Light directions require the light component to be attached to an entity.");
    }
return GetEntityForwardDirection(lightComponent->Parent);}

