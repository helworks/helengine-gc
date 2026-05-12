#ifdef DrawText
#undef DrawText
#endif
#include "RawByteContentProcessor.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "system/io/stream.hpp"
#include "RawByteContent.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

Type* RawByteContentProcessor::get_OutputType()
{
return he_cpp_type_of<RawByteContent>("RawByteContent");
}

::RawByteContent* RawByteContentProcessor::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
{
::MemoryStream *memoryStream = new ::MemoryStream();
stream->CopyTo(memoryStream);
return ([&]() {
auto __object_0000021B = new ::RawByteContent();
__object_0000021B->set_Bytes(memoryStream->ToArray());
return __object_0000021B;
})();}
}

void* RawByteContentProcessor::ReadObject(::Stream* stream)
{
return this->Read(stream);}

