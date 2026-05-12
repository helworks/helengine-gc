#ifdef DrawText
#undef DrawText
#endif
#include "BinaryWriterBE.hpp"
#include "runtime/native_span.hpp"
#include "system/binary_primitives.hpp"
#include "system/io/stream.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/binary_primitives.hpp"
#include "runtime/native_span.hpp"
#include "system/io/stream.hpp"

::EngineBinaryEndianness BinaryWriterBE::get_Endianness()
{
return EngineBinaryEndianness::BigEndian;
}

BinaryWriterBE::BinaryWriterBE(::Stream* stream, bool leaveOpen) : EngineBinaryWriter(stream, leaveOpen)
{
}

void BinaryWriterBE::WriteInt32(int32_t value)
{
uint8_t buffer[sizeof(int32_t)];
BinaryPrimitives::WriteInt32BigEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteInt64(int64_t value)
{
uint8_t buffer[sizeof(int64_t)];
BinaryPrimitives::WriteInt64BigEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteUInt16(uint16_t value)
{
uint8_t buffer[sizeof(uint16_t)];
BinaryPrimitives::WriteUInt16BigEndian(buffer, value);
BaseStream->Write(buffer);
}

void BinaryWriterBE::WriteUInt32(uint32_t value)
{
uint8_t buffer[sizeof(uint32_t)];
BinaryPrimitives::WriteUInt32BigEndian(buffer, value);
BaseStream->Write(buffer);
}

