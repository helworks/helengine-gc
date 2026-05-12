#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeTextComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReference.hpp"
#include "TextComponent.hpp"
#include "Component.hpp"
#include "byte4.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeTextComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeTextComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (referenceResolver == nullptr)
    {
throw new ArgumentNullException("referenceResolver");
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000267 = ([&]() {
Array<uint8_t>* __coalesce_00000268 = record->get_Payload();
return __coalesce_00000268 != nullptr ? __coalesce_00000268 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000269 = false;
return new ::MemoryStream(__ctor_arg_00000267, __ctor_arg_00000269);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported text component payload version '") + std::to_string(version) + std::string("'."));
    }
::SceneAssetReference *fontReference = this->ReadOptionalReference(reader);
::TextComponent *textComponent = ([&]() {
auto __object_0000026A = new ::TextComponent();
__object_0000026A->set_Text(reader->ReadString());
__object_0000026A->set_WrapText(reader->ReadByte() != 0);
__object_0000026A->set_Size(reader->ReadInt2());
__object_0000026A->set_Color(this->ReadByte4(reader));
__object_0000026A->set_SourceRect(reader->ReadFloat4());
__object_0000026A->set_Rotation(reader->ReadSingle());
__object_0000026A->set_RenderOrder2D(reader->ReadByte());
__object_0000026A->set_LayerMask(reader->ReadByte());
__object_0000026A->set_SelectionEnabled(reader->ReadByte() != 0);
return __object_0000026A;
})();
    if (fontReference == nullptr)
    {
throw new InvalidOperationException("TextComponent requires a packaged font reference before deserialization.");
    }
textComponent->set_Font(referenceResolver->ResolveFont(fontReference));
return textComponent;}
}
}

std::string RuntimeTextComponentDeserializer::ComponentType = "helengine.TextComponent";

uint8_t RuntimeTextComponentDeserializer::CurrentVersion = 1;

::byte4 RuntimeTextComponentDeserializer::ReadByte4(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_0000026B = reader->ReadByte();
auto __ctor_arg_0000026C = reader->ReadByte();
auto __ctor_arg_0000026D = reader->ReadByte();
auto __ctor_arg_0000026E = reader->ReadByte();
return ::byte4(__ctor_arg_0000026B, __ctor_arg_0000026C, __ctor_arg_0000026D, __ctor_arg_0000026E);
})();}

::SceneAssetReference* RuntimeTextComponentDeserializer::ReadOptionalReference(::EngineBinaryReader* reader)
{
    if (reader->ReadByte() == 0)
    {
return nullptr;    }
return ([&]() {
auto __object_0000026F = new ::SceneAssetReference();
__object_0000026F->set_SourceKind(static_cast<SceneAssetReferenceSourceKind>(reader->ReadInt32()));
__object_0000026F->set_RelativePath(reader->ReadString());
__object_0000026F->set_ProviderId(reader->ReadString());
__object_0000026F->set_AssetId(reader->ReadString());
return __object_0000026F;
})();}

