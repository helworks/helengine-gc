#ifdef DrawText
#undef DrawText
#endif
#include "HlslShaderBindingParser.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderConditionalPreprocessor.hpp"
#include "runtime/native_list.hpp"
#include "system/text/regular_expressions/regex.hpp"
#include "HlslShaderBindingParser.hpp"
#include "ShaderConstantMember.hpp"
#include "ShaderResourceType.hpp"
#include "ShaderBinding.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/number.hpp"
#include "system/text/regular_expressions/regex.hpp"

Array<::ShaderBinding*>* HlslShaderBindingParser::ParseBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy)
{
return ParseBindings(source, bindingPolicy, new List<ShaderDefine*>(Array<ShaderDefine*>::Empty()));}

Array<::ShaderBinding*>* HlslShaderBindingParser::ParseBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy, List<::ShaderDefine*>* defines)
{
    if (String::IsNullOrWhiteSpace(source))
    {
throw ([&]() {
auto __ctor_arg_00000087 = "Shader source must be provided.";
auto __ctor_arg_00000088 = "source";
return new ArgumentException(__ctor_arg_00000087, __ctor_arg_00000088);
})();
    }
    if (bindingPolicy == nullptr)
    {
throw new ArgumentNullException("bindingPolicy");
    }
    if (defines == nullptr)
    {
throw new ArgumentNullException("defines");
    }
const std::string preprocessedSource = ShaderConditionalPreprocessor::Preprocess(source, defines);
const std::string normalizedSource = StripComments(preprocessedSource);
List<::ShaderBinding*> *bindings = new List<::ShaderBinding*>();
AddConstantBufferBindings(normalizedSource, bindingPolicy, bindings);
AddResourceBindings(normalizedSource, bindingPolicy, bindings);
return bindings->ToArray();}

Regex HlslShaderBindingParser::BlockCommentPattern = Regex("/\\*.*?\\*/", RegexOptions::Compiled | RegexOptions::Singleline);

Regex HlslShaderBindingParser::ConstantBufferMemberPattern = Regex("(?<type>[A-Za-z_]\\w*(?:<[^;>]+>)?)\\s+(?<name>[A-Za-z_]\\w*)(?:\\s*\\[\\s*(?<count>\\d+)\\s*\\])?\\s*;", RegexOptions::Compiled);

Regex HlslShaderBindingParser::ConstantBufferPattern = Regex("\\bcbuffer\\s+(?<name>[A-Za-z_]\\w*)\\s*:\\s*register\\s*\\(\\s*b(?<slot>\\d+)(?:\\s*,\\s*space(?<space>\\d+))?\\s*\\)\\s*\\{(?<body>.*?)\\}\\s*;?", RegexOptions::Compiled | RegexOptions::Singleline);

Regex HlslShaderBindingParser::LineCommentPattern = Regex("//.*?$", RegexOptions::Compiled | RegexOptions::Multiline);

Regex HlslShaderBindingParser::MatrixTypePattern = Regex("^(?<base>[A-Za-z_]\\w*)(?<rows>\\d)x(?<columns>\\d)$", RegexOptions::Compiled);

int32_t HlslShaderBindingParser::RegisterSizeInBytes = 16;

Regex HlslShaderBindingParser::ResourcePattern = Regex("^\\s*(?<type>[A-Za-z_]\\w*(?:<[^;>]+>)?)\\s+(?<name>[A-Za-z_]\\w*)\\s*:\\s*register\\s*\\(\\s*(?<register>[bstu])(?<slot>\\d+)(?:\\s*,\\s*space(?<space>\\d+))?\\s*\\)\\s*;", RegexOptions::Compiled | RegexOptions::Multiline);

void HlslShaderBindingParser::AddConstantBufferBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy, List<::ShaderBinding*>* bindings)
{
    if (String::IsNullOrEmpty(source))
    {
throw new ArgumentNullException("source");
    }
    if (bindingPolicy == nullptr)
    {
throw new ArgumentNullException("bindingPolicy");
    }
    if (bindings == nullptr)
    {
throw new ArgumentNullException("bindings");
    }
MatchCollection matches = ConstantBufferPattern.Matches(source);
for (int32_t matchIndex = 0; matchIndex < matches.Count; matchIndex++) {
Match match = matches[matchIndex];
    if (!match.Success)
    {
continue;
    }
const std::string name = match.Groups["name"].Value;
const int32_t registerIndex = ParseRequiredInt(match.Groups["slot"].Value, "constant-buffer register");
const int32_t set = ParseOptionalInt(match.Groups["space"].Value, bindingPolicy->get_DefaultSpace());
const int32_t slot = bindingPolicy->GetSlot(ShaderResourceType::ConstantBuffer, registerIndex);
const std::string body = match.Groups["body"].Value;
Array<::ShaderConstantMember*> *members = ParseConstantBufferMembers(body);
const int32_t size = ComputeConstantBufferSize(new List<ShaderConstantMember*>(members));
bindings->Add(new ::ShaderBinding(name, ShaderResourceType::ConstantBuffer, set, slot, size, members));
}
}

void HlslShaderBindingParser::AddResourceBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy, List<::ShaderBinding*>* bindings)
{
    if (String::IsNullOrEmpty(source))
    {
throw new ArgumentNullException("source");
    }
    if (bindingPolicy == nullptr)
    {
throw new ArgumentNullException("bindingPolicy");
    }
    if (bindings == nullptr)
    {
throw new ArgumentNullException("bindings");
    }
MatchCollection matches = ResourcePattern.Matches(source);
for (int32_t matchIndex = 0; matchIndex < matches.Count; matchIndex++) {
Match match = matches[matchIndex];
    if (!match.Success)
    {
continue;
    }
const std::string typeText = match.Groups["type"].Value;
::ShaderResourceType resourceType = ResolveResourceType(typeText, match.Groups["register"].Value);
const std::string name = match.Groups["name"].Value;
const int32_t registerIndex = ParseRequiredInt(match.Groups["slot"].Value, "resource register");
const int32_t set = ParseOptionalInt(match.Groups["space"].Value, bindingPolicy->get_DefaultSpace());
const int32_t slot = bindingPolicy->GetSlot(resourceType, registerIndex);
bindings->Add(([&]() {
auto __ctor_arg_00000089 = name;
auto __ctor_arg_0000008A = resourceType;
auto __ctor_arg_0000008B = set;
auto __ctor_arg_0000008C = slot;
auto __ctor_arg_0000008D = 0;
auto __ctor_arg_0000008E = Array<ShaderConstantMember*>::Empty();
return new ::ShaderBinding(__ctor_arg_00000089, __ctor_arg_0000008A, __ctor_arg_0000008B, __ctor_arg_0000008C, __ctor_arg_0000008D, __ctor_arg_0000008E);
})());
}
}

int32_t HlslShaderBindingParser::AlignToRegister(int32_t offset)
{
    if (offset < 0)
    {
throw ([&]() {
auto __ctor_arg_0000008F = "offset";
auto __ctor_arg_00000090 = "Offset cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000008F, __ctor_arg_00000090);
})();
    }
const int32_t remainder = offset % RegisterSizeInBytes;
    if (remainder == 0)
    {
return offset;    }
return offset + (RegisterSizeInBytes - remainder);}

int32_t HlslShaderBindingParser::ComputeArrayStorageSize(std::string type, int32_t arrayCount)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_00000091 = "Element type must be provided.";
auto __ctor_arg_00000092 = "type";
return new ArgumentException(__ctor_arg_00000091, __ctor_arg_00000092);
})();
    }
    if (arrayCount <= 0)
    {
throw ([&]() {
auto __ctor_arg_00000093 = "arrayCount";
auto __ctor_arg_00000094 = "Array count must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000093, __ctor_arg_00000094);
})();
    }
    if (IsMatrixType(type))
    {
return ComputeMatrixStorageSize(type) * arrayCount;    }
return RegisterSizeInBytes * arrayCount;}

int32_t HlslShaderBindingParser::ComputeConstantBufferSize(List<::ShaderConstantMember*>* members)
{
    if (members == nullptr)
    {
throw new ArgumentNullException("members");
    }
    if (members->get_Count() == 0)
    {
return 0;    }
::ShaderConstantMember *lastMember = (*members)[members->get_Count() - 1];
const int32_t endOffset = lastMember->get_Offset() + lastMember->get_Size();
return AlignToRegister(endOffset);}

int32_t HlslShaderBindingParser::ComputeMatrixStorageSize(std::string type)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_00000095 = "Matrix type must be provided.";
auto __ctor_arg_00000096 = "type";
return new ArgumentException(__ctor_arg_00000095, __ctor_arg_00000096);
})();
    }
    if (String::Equals(type, "matrix", StringComparison::Ordinal))
    {
return RegisterSizeInBytes * 4;    }
Match match = MatrixTypePattern.Match(type);
    if (!match.Success)
    {
throw new InvalidOperationException(std::string("Unsupported HLSL matrix type '") + type + std::string("'."));
    }
const int32_t columns = ParseRequiredInt(match.Groups["columns"].Value, "matrix column count");
return RegisterSizeInBytes * columns;}

int32_t HlslShaderBindingParser::ComputeMemberOffset(int32_t currentOffset, std::string type, int32_t arrayCount, int32_t memberSize)
{
    if (currentOffset < 0)
    {
throw ([&]() {
auto __ctor_arg_00000097 = "currentOffset";
auto __ctor_arg_00000098 = "Current offset cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000097, __ctor_arg_00000098);
})();
    }
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_00000099 = "Member type must be provided.";
auto __ctor_arg_0000009A = "type";
return new ArgumentException(__ctor_arg_00000099, __ctor_arg_0000009A);
})();
    }
    if (arrayCount < 0)
    {
throw ([&]() {
auto __ctor_arg_0000009B = "arrayCount";
auto __ctor_arg_0000009C = "Array count cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000009B, __ctor_arg_0000009C);
})();
    }
    if (memberSize < 0)
    {
throw ([&]() {
auto __ctor_arg_0000009D = "memberSize";
auto __ctor_arg_0000009E = "Member size cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_0000009D, __ctor_arg_0000009E);
})();
    }
    if (RequiresRegisterAlignment(type, arrayCount))
    {
return AlignToRegister(currentOffset);    }
const int32_t registerOffset = currentOffset % RegisterSizeInBytes;
    if (registerOffset + memberSize > RegisterSizeInBytes)
    {
return AlignToRegister(currentOffset);    }
return currentOffset;}

int32_t HlslShaderBindingParser::ComputeMemberStorageSize(std::string type, int32_t arrayCount)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_0000009F = "Member type must be provided.";
auto __ctor_arg_000000A0 = "type";
return new ArgumentException(__ctor_arg_0000009F, __ctor_arg_000000A0);
})();
    }
    if (arrayCount < 0)
    {
throw ([&]() {
auto __ctor_arg_000000A1 = "arrayCount";
auto __ctor_arg_000000A2 = "Array count cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000000A1, __ctor_arg_000000A2);
})();
    }
    if (arrayCount > 0)
    {
return ComputeArrayStorageSize(type, arrayCount);    }
else     if (IsMatrixType(type))
    {
return ComputeMatrixStorageSize(type);    }
return ComputeScalarOrVectorStorageSize(type);}

int32_t HlslShaderBindingParser::ComputeScalarOrVectorStorageSize(std::string type)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000A3 = "Member type must be provided.";
auto __ctor_arg_000000A4 = "type";
return new ArgumentException(__ctor_arg_000000A3, __ctor_arg_000000A4);
})();
    }
const std::string normalizedType = String::Trim(type);
    if (String::Equals(normalizedType, "matrix", StringComparison::Ordinal))
    {
return RegisterSizeInBytes * 4;    }
int32_t componentCount;
    if (TryParseVectorComponentCount(normalizedType, componentCount))
    {
return ResolveScalarTypeSize(normalizedType) * componentCount;    }
return ResolveScalarTypeSize(normalizedType);}

std::string HlslShaderBindingParser::ExtractBaseType(std::string type)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000A5 = "Type must be provided.";
auto __ctor_arg_000000A6 = "type";
return new ArgumentException(__ctor_arg_000000A5, __ctor_arg_000000A6);
})();
    }
int32_t numericIndex = static_cast<int32_t>(type.size());
for (int32_t characterIndex = 0; characterIndex < static_cast<int32_t>(type.size()); characterIndex++) {
    if (String::IsDigit(type[characterIndex]))
    {
numericIndex = characterIndex;
break;
    }
}
return String::Substring(type, 0, numericIndex);}

bool HlslShaderBindingParser::IsMatrixType(std::string type)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000A7 = "Type must be provided.";
auto __ctor_arg_000000A8 = "type";
return new ArgumentException(__ctor_arg_000000A7, __ctor_arg_000000A8);
})();
    }
    if (String::Equals(type, "matrix", StringComparison::Ordinal))
    {
return true;    }
return MatrixTypePattern.IsMatch(type);}

Array<::ShaderConstantMember*>* HlslShaderBindingParser::ParseConstantBufferMembers(std::string body)
{
    if (String::IsNullOrEmpty(body))
    {
throw new ArgumentNullException("body");
    }
List<::ShaderConstantMember*> *members = new List<::ShaderConstantMember*>();
MatchCollection matches = ConstantBufferMemberPattern.Matches(body);
int32_t currentOffset = 0;
for (int32_t matchIndex = 0; matchIndex < matches.Count; matchIndex++) {
Match match = matches[matchIndex];
    if (!match.Success)
    {
continue;
    }
const std::string type = match.Groups["type"].Value;
const std::string name = match.Groups["name"].Value;
const int32_t arrayCount = ParseOptionalInt(match.Groups["count"].Value, 0);
const int32_t memberSize = ComputeMemberStorageSize(type, arrayCount);
const int32_t memberOffset = ComputeMemberOffset(currentOffset, type, arrayCount, memberSize);
members->Add(new ::ShaderConstantMember(name, type, memberOffset, memberSize));
currentOffset = memberOffset + memberSize;
}
return members->ToArray();}

int32_t HlslShaderBindingParser::ParseOptionalInt(std::string text, int32_t fallbackValue)
{
    if (String::IsNullOrWhiteSpace(text))
    {
return fallbackValue;    }
return ParseRequiredInt(text, "optional integer");}

int32_t HlslShaderBindingParser::ParseRequiredInt(std::string text, std::string label)
{
    if (String::IsNullOrWhiteSpace(text))
    {
throw ([&]() {
auto __ctor_arg_000000A9 = "Integer text must be provided.";
auto __ctor_arg_000000AA = "text";
return new ArgumentException(__ctor_arg_000000A9, __ctor_arg_000000AA);
})();
    }
    if (String::IsNullOrWhiteSpace(label))
    {
throw ([&]() {
auto __ctor_arg_000000AB = "Label must be provided.";
auto __ctor_arg_000000AC = "label";
return new ArgumentException(__ctor_arg_000000AB, __ctor_arg_000000AC);
})();
    }
int32_t value;
    if (Number::TryParse(text, value))
    {
return value;    }
throw new InvalidOperationException(std::string("The ") + label + std::string(" value '") + text + std::string("' is not a valid integer."));
}

bool HlslShaderBindingParser::RequiresRegisterAlignment(std::string type, int32_t arrayCount)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000AD = "Member type must be provided.";
auto __ctor_arg_000000AE = "type";
return new ArgumentException(__ctor_arg_000000AD, __ctor_arg_000000AE);
})();
    }
    if (arrayCount < 0)
    {
throw ([&]() {
auto __ctor_arg_000000AF = "arrayCount";
auto __ctor_arg_000000B0 = "Array count cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000000AF, __ctor_arg_000000B0);
})();
    }
    if (arrayCount > 0)
    {
return true;    }
return IsMatrixType(type);}

::ShaderResourceType HlslShaderBindingParser::ResolveResourceType(std::string typeText, std::string registerClass)
{
    if (String::IsNullOrWhiteSpace(typeText))
    {
throw ([&]() {
auto __ctor_arg_000000B1 = "Resource type text must be provided.";
auto __ctor_arg_000000B2 = "typeText";
return new ArgumentException(__ctor_arg_000000B1, __ctor_arg_000000B2);
})();
    }
    if (String::IsNullOrWhiteSpace(registerClass))
    {
throw ([&]() {
auto __ctor_arg_000000B3 = "Register class must be provided.";
auto __ctor_arg_000000B4 = "registerClass";
return new ArgumentException(__ctor_arg_000000B3, __ctor_arg_000000B4);
})();
    }
const std::string normalizedType = String::Trim(typeText);
    if (String::Equals(registerClass, "s", StringComparison::Ordinal))
    {
return ShaderResourceType::Sampler;    }
else     if (String::StartsWith(normalizedType, "TextureCube", StringComparison::Ordinal))
    {
return ShaderResourceType::TextureCube;    }
else     if (String::StartsWith(normalizedType, "Texture2D", StringComparison::Ordinal))
    {
return ShaderResourceType::Texture2D;    }
else     if (String::StartsWith(normalizedType, "RWTexture2D", StringComparison::Ordinal))
    {
return ShaderResourceType::StorageTexture2D;    }
else     if (String::StartsWith(normalizedType, "RWStructuredBuffer", StringComparison::Ordinal) || String::StartsWith(normalizedType, "RWBuffer", StringComparison::Ordinal) || String::Equals(normalizedType, "ByteAddressBuffer", StringComparison::Ordinal) || String::Equals(normalizedType, "RWByteAddressBuffer", StringComparison::Ordinal) || String::StartsWith(normalizedType, "AppendStructuredBuffer", StringComparison::Ordinal) || String::StartsWith(normalizedType, "ConsumeStructuredBuffer", StringComparison::Ordinal))
    {
return ShaderResourceType::StorageBuffer;    }
else     if (String::StartsWith(normalizedType, "StructuredBuffer", StringComparison::Ordinal) || String::StartsWith(normalizedType, "Buffer", StringComparison::Ordinal))
    {
return ShaderResourceType::Buffer;    }
throw new InvalidOperationException(std::string("Unsupported HLSL resource type '") + typeText + std::string("'."));
}

int32_t HlslShaderBindingParser::ResolveScalarTypeSize(std::string type)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000B5 = "Scalar type must be provided.";
auto __ctor_arg_000000B6 = "type";
return new ArgumentException(__ctor_arg_000000B5, __ctor_arg_000000B6);
})();
    }
const std::string baseType = ExtractBaseType(type);
{
const std::string __switchValue41_1 = baseType;
if (String::Equals(__switchValue41_1, "bool") || String::Equals(__switchValue41_1, "int") || String::Equals(__switchValue41_1, "uint") || String::Equals(__switchValue41_1, "float") || String::Equals(__switchValue41_1, "half") || String::Equals(__switchValue41_1, "min16float") || String::Equals(__switchValue41_1, "min10float") || String::Equals(__switchValue41_1, "min16int") || String::Equals(__switchValue41_1, "min12int") || String::Equals(__switchValue41_1, "min16uint")) {
return 4;}
else {
throw new InvalidOperationException(std::string("Unsupported HLSL scalar type '") + type + std::string("'."));
}
}

}

std::string HlslShaderBindingParser::StripComments(std::string source)
{
    if (String::IsNullOrEmpty(source))
    {
throw new ArgumentNullException("source");
    }
const std::string withoutBlockComments = BlockCommentPattern.Replace(source, String::Empty);
return LineCommentPattern.Replace(withoutBlockComments, String::Empty);}

bool HlslShaderBindingParser::TryParseVectorComponentCount(std::string type, int32_t& componentCount)
{
    if (String::IsNullOrWhiteSpace(type))
    {
throw ([&]() {
auto __ctor_arg_000000B7 = "Type must be provided.";
auto __ctor_arg_000000B8 = "type";
return new ArgumentException(__ctor_arg_000000B7, __ctor_arg_000000B8);
})();
    }
Match match = MatrixTypePattern.Match(type);
    if (match.Success)
    {
componentCount = 0;
return false;    }
const int32_t trailingDigitIndex = static_cast<int32_t>(type.size()) - 1;
    if (trailingDigitIndex < 0 || !String::IsDigit(type[trailingDigitIndex]))
    {
componentCount = 1;
return false;    }
componentCount = ParseRequiredInt(String::Substring(type, trailingDigitIndex, 1), "vector component count");
return true;}

