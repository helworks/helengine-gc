#ifdef DrawText
#undef DrawText
#endif
#include "FontAssetBinarySerializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryHeaderSerializer.hpp"
#include "FontAsset.hpp"
#include "EngineBinaryReader.hpp"
#include "FontInfo.hpp"
#include "TextureAsset.hpp"
#include "runtime/native_dictionary.hpp"
#include "RuntimeTexture.hpp"
#include "Core.hpp"
#include "RenderManager2D.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "FontChar.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"

uint8_t FontAssetBinarySerializer::CurrentVersion = 1;

uint16_t FontAssetBinarySerializer::FormatId = 1;

::EditorBinaryRecordKind FontAssetBinarySerializer::RecordKind = EditorBinaryRecordKind::FontAsset;

::FontAsset* FontAssetBinarySerializer::Deserialize(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
::EngineBinaryHeader *header = EngineBinaryHeaderSerializer::Read(stream);
return Deserialize(stream, header);}

::FontAsset* FontAssetBinarySerializer::Deserialize(::Stream* stream, ::EngineBinaryHeader* header)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
    if (header == nullptr)
    {
throw new ArgumentNullException("header");
    }
ValidateHeader(header);
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager2D() == nullptr)
    {
throw new InvalidOperationException("Font assets require an initialized core renderer before deserialization.");
    }
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, header->get_Endianness(), true);
::FontInfo *fontInfo = ([&]() {
auto __ctor_arg_0000007D = reader->ReadString();
auto __ctor_arg_0000007E = reader->ReadInt32();
auto __ctor_arg_0000007F = reader->ReadSingle();
return new ::FontInfo(__ctor_arg_0000007D, __ctor_arg_0000007E, __ctor_arg_0000007F);
})();
const float lineHeight = reader->ReadSingle();
const int32_t atlasWidth = reader->ReadInt32();
const int32_t atlasHeight = reader->ReadInt32();
::TextureAsset *sourceTexture = ([&]() {
auto __object_00000080 = new ::TextureAsset();
__object_00000080->Width = reader->ReadUInt16();
__object_00000080->Height = reader->ReadUInt16();
__object_00000080->Colors = reader->ReadByteArray();
return __object_00000080;
})();
const int32_t characterCount = reader->ReadInt32();
Dictionary<char, ::FontChar> *characters = new Dictionary<char, ::FontChar>(characterCount);
for (int32_t index = 0; index < characterCount; index++) {
const char character = static_cast<char>(reader->ReadUInt16());
::FontChar fontChar = ([&]() {
auto __ctor_arg_00000081 = reader->ReadFloat4();
auto __ctor_arg_00000082 = reader->ReadSingle();
auto __ctor_arg_00000083 = reader->ReadSingle();
auto __ctor_arg_00000084 = reader->ReadSingle();
auto __ctor_arg_00000085 = reader->ReadSingle();
return ::FontChar(__ctor_arg_00000081, __ctor_arg_00000082, __ctor_arg_00000083, __ctor_arg_00000084, __ctor_arg_00000085);
})();
characters->Add(character, fontChar);
}
::RuntimeTexture *texture = Core::get_Instance()->get_RenderManager2D()->BuildTextureFromRaw(sourceTexture);
::FontAsset *asset = ([&]() {
auto __object_00000086 = new ::FontAsset(fontInfo, texture, characters, lineHeight, atlasWidth, atlasHeight);
__object_00000086->set_SourceTextureAsset(sourceTexture);
return __object_00000086;
})();
return asset;}
}

void FontAssetBinarySerializer::ValidateHeader(::EngineBinaryHeader* header)
{
    if (header->get_FormatId() != FormatId)
    {
throw new InvalidOperationException(std::string("Unsupported font binary format id '") + std::to_string(header->get_FormatId()) + std::string("'."));
    }
    if (header->get_RecordKind() != static_cast<uint16_t>(RecordKind))
    {
throw new InvalidOperationException(std::string("Unexpected font record kind '") + std::to_string(header->get_RecordKind()) + std::string("'."));
    }
    if (header->get_Version() != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported font binary version '") + std::to_string(header->get_Version()) + std::string("'."));
    }
}

