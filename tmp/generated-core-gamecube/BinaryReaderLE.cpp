#ifdef DrawText
#undef DrawText
#endif
#include "BinaryReaderLE.hpp"
#include "runtime/native_span.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/native_span.hpp"

::EngineBinaryEndianness BinaryReaderLE::get_Endianness()
{
return EngineBinaryEndianness::LittleEndian;
}

BinaryReaderLE::BinaryReaderLE(::Stream* stream, bool leaveOpen) : EngineBinaryReader(stream, leaveOpen)
{
}

int32_t BinaryReaderLE::ReadInt32()
{
uint8_t buffer[sizeof(int32_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt32LittleEndian(buffer);}

int64_t BinaryReaderLE::ReadInt64()
{
uint8_t buffer[sizeof(int64_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadInt64LittleEndian(buffer);}

uint16_t BinaryReaderLE::ReadUInt16()
{
uint8_t buffer[sizeof(uint16_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt16LittleEndian(buffer);}

uint32_t BinaryReaderLE::ReadUInt32()
{
uint8_t buffer[sizeof(uint32_t)];
ReadRequiredBytes(buffer);
return BinaryPrimitives::ReadUInt32LittleEndian(buffer);}

