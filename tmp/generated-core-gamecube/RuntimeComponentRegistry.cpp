#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeComponentRegistry.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_type.hpp"
#include "RuntimeMeshComponentDeserializer.hpp"
#include "RuntimeCameraComponentDeserializer.hpp"
#include "RuntimeFPSComponentDeserializer.hpp"
#include "RuntimeTextComponentDeserializer.hpp"
#include "RuntimeRoundedRectComponentDeserializer.hpp"
#include "RuntimeDirectionalLightComponentDeserializer.hpp"
#include "RuntimeAmbientLightComponentDeserializer.hpp"
#include "RuntimePointLightComponentDeserializer.hpp"
#include "RuntimeSpotLightComponentDeserializer.hpp"
#include "RuntimeDirectionalShadowCameraOrbitComponentDeserializer.hpp"
#include "RuntimeDirectionalShadowOrbitComponentDeserializer.hpp"
#include "RuntimeDirectionalShadowSunSweepComponentDeserializer.hpp"
#include "RuntimeDirectionalShadowTowerSpinComponentDeserializer.hpp"
#include "RuntimeMenuComponentDeserializer.hpp"
#include "RuntimeMenuPanelComponentDeserializer.hpp"
#include "RuntimeMenuItemComponentDeserializer.hpp"
#include "RuntimeMenuSelectedDescriptionComponentDeserializer.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/string_comparer.hpp"

::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault()
{
::RuntimeComponentRegistry *registry = new ::RuntimeComponentRegistry();
registry->Register(new ::RuntimeMeshComponentDeserializer());
registry->Register(new ::RuntimeCameraComponentDeserializer());
registry->Register(new ::RuntimeFPSComponentDeserializer());
registry->Register(new ::RuntimeTextComponentDeserializer());
registry->Register(new ::RuntimeRoundedRectComponentDeserializer());
registry->Register(new ::RuntimeDirectionalLightComponentDeserializer());
registry->Register(new ::RuntimeAmbientLightComponentDeserializer());
registry->Register(new ::RuntimePointLightComponentDeserializer());
registry->Register(new ::RuntimeSpotLightComponentDeserializer());
registry->Register(new ::RuntimeDirectionalShadowCameraOrbitComponentDeserializer());
registry->Register(new ::RuntimeDirectionalShadowOrbitComponentDeserializer());
registry->Register(new ::RuntimeDirectionalShadowSunSweepComponentDeserializer());
registry->Register(new ::RuntimeDirectionalShadowTowerSpinComponentDeserializer());
registry->Register(new ::RuntimeMenuComponentDeserializer());
registry->Register(new ::RuntimeMenuPanelComponentDeserializer());
registry->Register(new ::RuntimeMenuItemComponentDeserializer());
registry->Register(new ::RuntimeMenuSelectedDescriptionComponentDeserializer());
return registry;}

::IRuntimeComponentDeserializer* RuntimeComponentRegistry::GetDeserializer(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
throw ([&]() {
auto __ctor_arg_00000123 = "Component type id must be provided.";
auto __ctor_arg_00000124 = "componentTypeId";
return new ArgumentException(__ctor_arg_00000123, __ctor_arg_00000124);
})();
    }
::IRuntimeComponentDeserializer* deserializer;
    if (!this->DeserializersByTypeId->TryGetValue(componentTypeId, deserializer))
    {
deserializer = this->TryCreateAutomaticComponentDeserializer(componentTypeId);
    if (deserializer == nullptr)
    {
throw new InvalidOperationException(std::string("Player builds do not support serialized component type '") + componentTypeId + std::string("' yet."));
    }
this->DeserializersByTypeId->Add(componentTypeId, deserializer);
    }
return deserializer;}

void RuntimeComponentRegistry::Register(::IRuntimeComponentDeserializer* deserializer)
{
    if (deserializer == nullptr)
    {
throw new ArgumentNullException("deserializer");
    }
    if (String::IsNullOrWhiteSpace(deserializer->get_ComponentTypeId()))
    {
throw new InvalidOperationException("Runtime component deserializers must expose a serialized type id.");
    }
    if (this->DeserializersByTypeId->ContainsKey(deserializer->get_ComponentTypeId()))
    {
throw new InvalidOperationException(std::string("A runtime component deserializer is already registered for '") + deserializer->get_ComponentTypeId() + std::string("'."));
    }
this->DeserializersByTypeId->Add(deserializer->get_ComponentTypeId(), deserializer);
}

RuntimeComponentRegistry::RuntimeComponentRegistry() : DeserializersByTypeId()
{
this->DeserializersByTypeId = new Dictionary<std::string, ::IRuntimeComponentDeserializer*>(StringComparer::OrdinalIgnoreCase);
}

bool RuntimeComponentRegistry::TryGet(std::string componentTypeId, ::IRuntimeComponentDeserializer*& deserializer)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
deserializer = nullptr;
return false;    }
return this->DeserializersByTypeId->TryGetValue(componentTypeId, deserializer);}

::IRuntimeComponentDeserializer* RuntimeComponentRegistry::TryCreateAutomaticComponentDeserializer(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
return nullptr;    }
return nullptr;}

