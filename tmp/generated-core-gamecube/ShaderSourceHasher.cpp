#ifdef DrawText
#undef DrawText
#endif
#include "ShaderSourceHasher.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "system/text/encoding.hpp"
#include "SHA256.hpp"
#include "runtime/native_string.hpp"
#include "system/text/encoding.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/security/cryptography/sha256.hpp"

std::string ShaderSourceHasher::ComputeHash(std::string source)
{
    if (String::IsNullOrEmpty(source))
    {
throw new ArgumentNullException("source");
    }
Array<uint8_t> *data = Encoding::GetBytes(Encoding::UTF8, source);
Array<uint8_t> *hash = SHA256::HashData(data);
return this->ConvertToHex(hash);}

std::string ShaderSourceHasher::ConvertToHex(Array<uint8_t>* data)
{
Array<char> *buffer = new Array<char>(data->Length * 2);
int32_t outputIndex = 0;
for (int32_t i = 0; i < data->Length; i++) {
const uint8_t value = (*data)[i];
const int32_t high = value >> 4;
const int32_t low = value & 0xF;
(*buffer)[outputIndex++] = this->ToHexChar(high);
(*buffer)[outputIndex++] = this->ToHexChar(low);
}
return std::string(buffer->Data, static_cast<size_t>(buffer->Length));}

char ShaderSourceHasher::ToHexChar(int32_t value)
{
    if (value < 10)
    {
return static_cast<char>(('0' + value));    }
return static_cast<char>(('a' + (value - 10)));}

