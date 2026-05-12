#ifdef DrawText
#undef DrawText
#endif
#include "LightComponentScenePayloadSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "float4.hpp"
#include "runtime/native_exceptions.hpp"

uint8_t LightComponentScenePayloadSerializer::CurrentVersion = 2;

::AmbientLightComponent* LightComponentScenePayloadSerializer::ReadAmbientLight(::EngineBinaryReader* reader)
{
return ReadAmbientLight(reader, CurrentVersion);}

::AmbientLightComponent* LightComponentScenePayloadSerializer::ReadAmbientLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported ambient light payload version '") + std::to_string(version) + std::string("'."));
    }
::AmbientLightComponent *lightComponent = new ::AmbientLightComponent();
ReadCommonLightFields(reader, lightComponent);
return lightComponent;}

::DirectionalLightComponent* LightComponentScenePayloadSerializer::ReadDirectionalLight(::EngineBinaryReader* reader)
{
return ReadDirectionalLight(reader, CurrentVersion);}

::DirectionalLightComponent* LightComponentScenePayloadSerializer::ReadDirectionalLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional light payload version '") + std::to_string(version) + std::string("'."));
    }
::DirectionalLightComponent *lightComponent = new ::DirectionalLightComponent();
ReadCommonLightFields(reader, lightComponent);
lightComponent->set_ShadowDistance(reader->ReadSingle());
return lightComponent;}

::PointLightComponent* LightComponentScenePayloadSerializer::ReadPointLight(::EngineBinaryReader* reader)
{
return ReadPointLight(reader, CurrentVersion);}

::PointLightComponent* LightComponentScenePayloadSerializer::ReadPointLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported point light payload version '") + std::to_string(version) + std::string("'."));
    }
::PointLightComponent *lightComponent = new ::PointLightComponent();
ReadCommonLightFields(reader, lightComponent);
lightComponent->set_Range(reader->ReadSingle());
return lightComponent;}

::SpotLightComponent* LightComponentScenePayloadSerializer::ReadSpotLight(::EngineBinaryReader* reader)
{
return ReadSpotLight(reader, CurrentVersion);}

::SpotLightComponent* LightComponentScenePayloadSerializer::ReadSpotLight(::EngineBinaryReader* reader, uint8_t version)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported spot light payload version '") + std::to_string(version) + std::string("'."));
    }
::SpotLightComponent *lightComponent = new ::SpotLightComponent();
ReadCommonLightFields(reader, lightComponent);
lightComponent->set_Range(reader->ReadSingle());
lightComponent->set_InnerConeAngleDegrees(reader->ReadSingle());
lightComponent->set_OuterConeAngleDegrees(reader->ReadSingle());
return lightComponent;}

void LightComponentScenePayloadSerializer::WriteAmbientLight(::EngineBinaryWriter* writer, ::AmbientLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
WriteCommonLightFields(writer, lightComponent);
}

void LightComponentScenePayloadSerializer::WriteDirectionalLight(::EngineBinaryWriter* writer, ::DirectionalLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->get_ShadowDistance());
}

void LightComponentScenePayloadSerializer::WritePointLight(::EngineBinaryWriter* writer, ::PointLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->get_Range());
}

void LightComponentScenePayloadSerializer::WriteSpotLight(::EngineBinaryWriter* writer, ::SpotLightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
WriteCommonLightFields(writer, lightComponent);
writer->WriteSingle(lightComponent->get_Range());
writer->WriteSingle(lightComponent->get_InnerConeAngleDegrees());
writer->WriteSingle(lightComponent->get_OuterConeAngleDegrees());
}

void LightComponentScenePayloadSerializer::ReadCommonLightFields(::EngineBinaryReader* reader, ::LightComponent* lightComponent)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
lightComponent->set_Color(ReadFloat4(reader));
lightComponent->set_Intensity(reader->ReadSingle());
lightComponent->set_ShadowsEnabled(reader->ReadByte() != 0);
lightComponent->set_ShadowMapMode(static_cast<ShadowMapMode>(reader->ReadByte()));
lightComponent->set_ShadowStrength(reader->ReadSingle());
}

::float4 LightComponentScenePayloadSerializer::ReadFloat4(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_000000BD = reader->ReadSingle();
auto __ctor_arg_000000BE = reader->ReadSingle();
auto __ctor_arg_000000BF = reader->ReadSingle();
auto __ctor_arg_000000C0 = reader->ReadSingle();
return ::float4(__ctor_arg_000000BD, __ctor_arg_000000BE, __ctor_arg_000000BF, __ctor_arg_000000C0);
})();}

void LightComponentScenePayloadSerializer::WriteCommonLightFields(::EngineBinaryWriter* writer, ::LightComponent* lightComponent)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (lightComponent == nullptr)
    {
throw new ArgumentNullException("lightComponent");
    }
WriteFloat4(writer, lightComponent->get_Color());
writer->WriteSingle(lightComponent->get_Intensity());
writer->WriteByte(lightComponent->get_ShadowsEnabled() ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0));
writer->WriteByte(static_cast<uint8_t>(lightComponent->get_ShadowMapMode()));
writer->WriteSingle(lightComponent->get_ShadowStrength());
}

void LightComponentScenePayloadSerializer::WriteFloat4(::EngineBinaryWriter* writer, ::float4 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
writer->WriteSingle(value.W);
}

