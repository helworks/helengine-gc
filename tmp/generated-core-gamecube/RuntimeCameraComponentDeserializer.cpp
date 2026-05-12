#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeCameraComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "CameraComponent.hpp"
#include "Component.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeCameraComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeCameraComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Camera component deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000021F = ([&]() {
Array<uint8_t>* __coalesce_00000220 = record->get_Payload();
return __coalesce_00000220 != nullptr ? __coalesce_00000220 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000221 = false;
return new ::MemoryStream(__ctor_arg_0000021F, __ctor_arg_00000221);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported camera component payload version '") + std::to_string(version) + std::string("'."));
    }
::CameraComponent *cameraComponent = ([&]() {
auto __object_00000222 = new ::CameraComponent();
__object_00000222->set_CameraDrawOrder(reader->ReadByte());
__object_00000222->set_LayerMask(reader->ReadUInt16());
__object_00000222->set_Viewport(this->ReadFloat4(reader));
return __object_00000222;
})();
cameraComponent->set_NearPlaneDistance(reader->ReadSingle());
cameraComponent->set_FarPlaneDistance(reader->ReadSingle());
cameraComponent->set_ClearSettings(this->ReadClearSettings(reader));
cameraComponent->set_RenderSettings(this->ReadRenderSettings(reader));
return cameraComponent;}
}
}

std::string RuntimeCameraComponentDeserializer::ComponentType = "helengine.CameraComponent";

uint8_t RuntimeCameraComponentDeserializer::CurrentVersion = 3;

::CameraClearSettings RuntimeCameraComponentDeserializer::ReadClearSettings(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_00000223 = reader->ReadByte() != 0;
auto __ctor_arg_00000224 = ReadFloat4(reader);
auto __ctor_arg_00000225 = reader->ReadByte() != 0;
auto __ctor_arg_00000226 = reader->ReadSingle();
auto __ctor_arg_00000227 = reader->ReadByte() != 0;
auto __ctor_arg_00000228 = reader->ReadByte();
return ::CameraClearSettings(__ctor_arg_00000223, __ctor_arg_00000224, __ctor_arg_00000225, __ctor_arg_00000226, __ctor_arg_00000227, __ctor_arg_00000228);
})();}

::float4 RuntimeCameraComponentDeserializer::ReadFloat4(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __ctor_arg_00000229 = reader->ReadSingle();
auto __ctor_arg_0000022A = reader->ReadSingle();
auto __ctor_arg_0000022B = reader->ReadSingle();
auto __ctor_arg_0000022C = reader->ReadSingle();
return ::float4(__ctor_arg_00000229, __ctor_arg_0000022A, __ctor_arg_0000022B, __ctor_arg_0000022C);
})();}

::CameraRenderSettings* RuntimeCameraComponentDeserializer::ReadRenderSettings(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
return ([&]() {
auto __object_0000022D = new ::CameraRenderSettings();
__object_0000022D->set_DepthPrepassMode(static_cast<DepthPrepassMode>(reader->ReadByte()));
__object_0000022D->set_ShadowDistance(reader->ReadSingle());
__object_0000022D->set_PostProcessTier(static_cast<PostProcessTier>(reader->ReadByte()));
return __object_0000022D;
})();}

