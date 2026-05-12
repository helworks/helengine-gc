#ifdef DrawText
#undef DrawText
#endif
#include "TextContentProcessor.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/stream-reader.hpp"
#include "system/text/encoding.hpp"
#include "TextContent.hpp"
#include "system/text/encoding.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_type.hpp"

Type* TextContentProcessor::get_OutputType()
{
return he_cpp_type_of<TextContent>("TextContent");
}

::TextContent* TextContentProcessor::Read(::Stream* stream)
{
    if (stream == nullptr)
    {
throw new ArgumentNullException("stream");
    }
{
StreamReader *reader = new StreamReader(stream, Encoding::UTF8, true, 1024, true);
return ([&]() {
auto __object_0000027B = new ::TextContent();
__object_0000027B->set_Text(reader->ReadToEnd());
return __object_0000027B;
})();}
}

void* TextContentProcessor::ReadObject(::Stream* stream)
{
return this->Read(stream);}

