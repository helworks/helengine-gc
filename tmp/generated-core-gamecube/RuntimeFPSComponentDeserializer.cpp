#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeFPSComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReference.hpp"
#include "FPSComponent.hpp"
#include "Component.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/bit_converter.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeFPSComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeFPSComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (referenceResolver == nullptr)
    {
throw new ArgumentNullException("referenceResolver");
    }
    if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("FPS component deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000023D = ([&]() {
Array<uint8_t>* __coalesce_0000023E = record->get_Payload();
return __coalesce_0000023E != nullptr ? __coalesce_0000023E : Array<uint8_t>::Empty();
})();
auto __ctor_arg_0000023F = false;
return new ::MemoryStream(__ctor_arg_0000023D, __ctor_arg_0000023F);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported FPS component payload version '") + std::to_string(version) + std::string("'."));
    }
::SceneAssetReference *fontReference = this->ReadOptionalReference(reader);
::FPSComponent *fpsComponent = ([&]() {
auto __object_00000240 = new ::FPSComponent();
__object_00000240->set_RefreshIntervalSeconds(BitConverter::Int64BitsToDouble(reader->ReadInt64()));
__object_00000240->set_Padding(reader->ReadInt2());
__object_00000240->set_RenderOrder2D(reader->ReadByte());
return __object_00000240;
})();
    if (fontReference != nullptr)
    {
fpsComponent->set_Font(referenceResolver->ResolveFont(fontReference));
    }
else {
throw new InvalidOperationException("FPSComponent requires a packaged font reference before deserialization.");
}
return fpsComponent;}
}
}

std::string RuntimeFPSComponentDeserializer::ComponentType = "helengine.FPSComponent";

uint8_t RuntimeFPSComponentDeserializer::CurrentVersion = 2;

::SceneAssetReference* RuntimeFPSComponentDeserializer::ReadOptionalReference(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (reader->ReadByte() == 0)
    {
return nullptr;    }
return ([&]() {
auto __object_00000241 = new ::SceneAssetReference();
__object_00000241->set_SourceKind(static_cast<SceneAssetReferenceSourceKind>(reader->ReadInt32()));
__object_00000241->set_RelativePath(reader->ReadString());
__object_00000241->set_ProviderId(reader->ReadString());
__object_00000241->set_AssetId(reader->ReadString());
return __object_00000241;
})();}

