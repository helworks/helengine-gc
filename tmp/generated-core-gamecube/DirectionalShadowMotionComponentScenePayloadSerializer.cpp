#ifdef DrawText
#undef DrawText
#endif
#include "DirectionalShadowMotionComponentScenePayloadSerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "DirectionalShadowCameraOrbitComponent.hpp"
#include "DirectionalShadowOrbitComponent.hpp"
#include "DirectionalShadowSunSweepComponent.hpp"
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "float3.hpp"
#include "runtime/native_exceptions.hpp"

uint8_t DirectionalShadowMotionComponentScenePayloadSerializer::CurrentVersion = 1;

::DirectionalShadowCameraOrbitComponent* DirectionalShadowMotionComponentScenePayloadSerializer::ReadCameraOrbit(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
::DirectionalShadowCameraOrbitComponent *component = new ::DirectionalShadowCameraOrbitComponent();
::float3 orbitCenter;
float orbitRadius;
float orbitHeight;
float baseAngleRadians;
float angularSpeedRadians;
ReadOrbitFields(reader, orbitCenter, orbitRadius, orbitHeight, baseAngleRadians, angularSpeedRadians);
component->set_OrbitCenter(orbitCenter);
component->set_OrbitRadius(orbitRadius);
component->set_OrbitHeight(orbitHeight);
component->set_BaseAngleRadians(baseAngleRadians);
component->set_AngularSpeedRadians(angularSpeedRadians);
component->set_LookDownPitchRadians(reader->ReadSingle());
return component;}

::DirectionalShadowOrbitComponent* DirectionalShadowMotionComponentScenePayloadSerializer::ReadOrbit(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
::DirectionalShadowOrbitComponent *component = new ::DirectionalShadowOrbitComponent();
::float3 orbitCenter;
float orbitRadius;
float orbitHeight;
float baseAngleRadians;
float angularSpeedRadians;
ReadOrbitFields(reader, orbitCenter, orbitRadius, orbitHeight, baseAngleRadians, angularSpeedRadians);
component->set_OrbitCenter(orbitCenter);
component->set_OrbitRadius(orbitRadius);
component->set_OrbitHeight(orbitHeight);
component->set_BaseAngleRadians(baseAngleRadians);
component->set_AngularSpeedRadians(angularSpeedRadians);
return component;}

::DirectionalShadowSunSweepComponent* DirectionalShadowMotionComponentScenePayloadSerializer::ReadSunSweep(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_0000003A = new ::DirectionalShadowSunSweepComponent();
__object_0000003A->set_MinYawRadians(reader->ReadSingle());
__object_0000003A->set_MaxYawRadians(reader->ReadSingle());
__object_0000003A->set_PitchRadians(reader->ReadSingle());
__object_0000003A->set_SweepSpeedRadians(reader->ReadSingle());
return __object_0000003A;
})();}

::DirectionalShadowTowerSpinComponent* DirectionalShadowMotionComponentScenePayloadSerializer::ReadTowerSpin(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_0000003B = new ::DirectionalShadowTowerSpinComponent();
__object_0000003B->set_BaseYawRadians(reader->ReadSingle());
__object_0000003B->set_AngularSpeedRadians(reader->ReadSingle());
return __object_0000003B;
})();}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteCameraOrbit(::EngineBinaryWriter* writer, ::DirectionalShadowCameraOrbitComponent* component)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
WriteOrbitFields(writer, component->get_OrbitCenter(), component->get_OrbitRadius(), component->get_OrbitHeight(), component->get_BaseAngleRadians(), component->get_AngularSpeedRadians());
writer->WriteSingle(component->get_LookDownPitchRadians());
}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteOrbit(::EngineBinaryWriter* writer, ::DirectionalShadowOrbitComponent* component)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
WriteOrbitFields(writer, component->get_OrbitCenter(), component->get_OrbitRadius(), component->get_OrbitHeight(), component->get_BaseAngleRadians(), component->get_AngularSpeedRadians());
}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteSunSweep(::EngineBinaryWriter* writer, ::DirectionalShadowSunSweepComponent* component)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
writer->WriteSingle(component->get_MinYawRadians());
writer->WriteSingle(component->get_MaxYawRadians());
writer->WriteSingle(component->get_PitchRadians());
writer->WriteSingle(component->get_SweepSpeedRadians());
}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteTowerSpin(::EngineBinaryWriter* writer, ::DirectionalShadowTowerSpinComponent* component)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
else     if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
writer->WriteSingle(component->get_BaseYawRadians());
writer->WriteSingle(component->get_AngularSpeedRadians());
}

::float3 DirectionalShadowMotionComponentScenePayloadSerializer::ReadFloat3(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_0000003C = reader->ReadSingle();
auto __ctor_arg_0000003D = reader->ReadSingle();
auto __ctor_arg_0000003E = reader->ReadSingle();
return ::float3(__ctor_arg_0000003C, __ctor_arg_0000003D, __ctor_arg_0000003E);
})();}

void DirectionalShadowMotionComponentScenePayloadSerializer::ReadOrbitFields(::EngineBinaryReader* reader, ::float3& orbitCenter, float& orbitRadius, float& orbitHeight, float& baseAngleRadians, float& angularSpeedRadians)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
orbitCenter = ReadFloat3(reader);
orbitRadius = reader->ReadSingle();
orbitHeight = reader->ReadSingle();
baseAngleRadians = reader->ReadSingle();
angularSpeedRadians = reader->ReadSingle();
}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteFloat3(::EngineBinaryWriter* writer, ::float3 value)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
writer->WriteSingle(value.X);
writer->WriteSingle(value.Y);
writer->WriteSingle(value.Z);
}

void DirectionalShadowMotionComponentScenePayloadSerializer::WriteOrbitFields(::EngineBinaryWriter* writer, ::float3 orbitCenter, float orbitRadius, float orbitHeight, float baseAngleRadians, float angularSpeedRadians)
{
    if (writer == nullptr)
    {
throw new ArgumentNullException("writer");
    }
WriteFloat3(writer, orbitCenter);
writer->WriteSingle(orbitRadius);
writer->WriteSingle(orbitHeight);
writer->WriteSingle(baseAngleRadians);
writer->WriteSingle(angularSpeedRadians);
}

