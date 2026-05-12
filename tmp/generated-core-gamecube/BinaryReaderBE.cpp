#ifdef DrawText
#undef DrawText
#endif
#include "BinaryReaderBE.hpp"
#include "runtime/native_span.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/native_span.hpp"

::EngineBinaryEndianness BinaryReaderBE::get_Endianness()
{
return EngineBinaryEndianness::BigEndian;
}

BinaryReaderBE::BinaryReaderBE(::Stream* stream, bool leaveOpen) : EngineBinaryReader(stream, leaveOpen)
{
}

int32_t BinaryReaderBE::ReadInt32()
{
uint8_t buffer[sizeof(int32_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt32BigEndian(buffer);}

int64_t BinaryReaderBE::ReadInt64()
{
uint8_t buffer[sizeof(int64_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt64BigEndian(buffer);}

uint16_t BinaryReaderBE::ReadUInt16()
{
uint8_t buffer[sizeof(uint16_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt16BigEndian(buffer);}

uint32_t BinaryReaderBE::ReadUInt32()
{
uint8_t buffer[sizeof(uint32_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt32BigEndian(buffer);}

