#ifdef DrawText
#undef DrawText
#endif
#include "BinaryWriterLE.hpp"
#include "runtime/native_span.hpp"
#include "system/binary_primitives.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/native_span.hpp"
#include "system/io/stream.hpp"

::EngineBinaryEndianness BinaryWriterLE::get_Endianness()
{
return EngineBinaryEndianness::LittleEndian;
}

BinaryWriterLE::BinaryWriterLE(::Stream* stream, bool leaveOpen) : EngineBinaryWriter(stream, leaveOpen)
{
}

void BinaryWriterLE::WriteInt32(int32_t value)
{
uint8_t buffer[sizeof(int32_t)];
BinaryPrimitives::WriteInt32LittleEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteInt64(int64_t value)
{
uint8_t buffer[sizeof(int64_t)];
BinaryPrimitives::WriteInt64LittleEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteUInt16(uint16_t value)
{
uint8_t buffer[sizeof(uint16_t)];
BinaryPrimitives::WriteUInt16LittleEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterLE::WriteUInt32(uint32_t value)
{
uint8_t buffer[sizeof(uint32_t)];
BinaryPrimitives::WriteUInt32LittleEndian(buffer, value);
BaseStream->Write(buffer);
}

