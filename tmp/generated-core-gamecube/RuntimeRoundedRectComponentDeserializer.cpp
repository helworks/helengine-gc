#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeRoundedRectComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "byte4.hpp"
#include "EngineBinaryEndianness.hpp"
#include "RoundedRectComponent.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeRoundedRectComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeRoundedRectComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000025C = ([&]() {
Array<uint8_t>* __coalesce_0000025D = record->get_Payload();
return __coalesce_0000025D != nullptr ? __coalesce_0000025D : Array<uint8_t>::Empty();
})();
auto __ctor_arg_0000025E = false;
return new ::MemoryStream(__ctor_arg_0000025C, __ctor_arg_0000025E);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported rounded rectangle component payload version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_0000025F = new ::RoundedRectComponent();
__object_0000025F->set_RenderOrder2D(reader->ReadByte());
__object_0000025F->set_LayerMask(reader->ReadByte());
__object_0000025F->set_Corners(static_cast<RoundedRectCorners>(reader->ReadInt32()));
__object_0000025F->set_Rotation(reader->ReadSingle());
__object_0000025F->set_Color(this->ReadByte4(reader));
__object_0000025F->set_SourceRect(reader->ReadFloat4());
__object_0000025F->set_Size(reader->ReadInt2());
__object_0000025F->set_Radius(reader->ReadSingle());
__object_0000025F->set_BorderThickness(reader->ReadSingle());
__object_0000025F->set_FillColor(this->ReadByte4(reader));
__object_0000025F->set_BorderColor(this->ReadByte4(reader));
return __object_0000025F;
})();}
}
}

std::string RuntimeRoundedRectComponentDeserializer::ComponentType = "helengine.RoundedRectComponent";

uint8_t RuntimeRoundedRectComponentDeserializer::CurrentVersion = 1;

::byte4 RuntimeRoundedRectComponentDeserializer::ReadByte4(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_00000260 = reader->ReadByte();
auto __ctor_arg_00000261 = reader->ReadByte();
auto __ctor_arg_00000262 = reader->ReadByte();
auto __ctor_arg_00000263 = reader->ReadByte();
return ::byte4(__ctor_arg_00000260, __ctor_arg_00000261, __ctor_arg_00000262, __ctor_arg_00000263);
})();}

