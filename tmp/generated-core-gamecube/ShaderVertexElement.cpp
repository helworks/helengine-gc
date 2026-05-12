#ifdef DrawText
#undef DrawText
#endif
#include "ShaderVertexElement.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderVertexElement::get_Format()
{
return this->Format;
}

int32_t ShaderVertexElement::get_Index()
{
return this->Index;
}

std::string ShaderVertexElement::get_Semantic()
{
return this->Semantic;
}

ShaderVertexElement::ShaderVertexElement(std::string semantic, int32_t index, std::string format) : Format(), Index(0), Semantic()
{
    if (String::IsNullOrWhiteSpace(semantic))
    {
throw ([&]() {
auto __ctor_arg_000001F6 = "Semantic must be provided.";
auto __ctor_arg_000001F7 = "semantic";
return new ArgumentException(__ctor_arg_000001F6, __ctor_arg_000001F7);
})();
    }
    if (String::IsNullOrWhiteSpace(format))
    {
throw ([&]() {
auto __ctor_arg_000001F8 = "Format must be provided.";
auto __ctor_arg_000001F9 = "format";
return new ArgumentException(__ctor_arg_000001F8, __ctor_arg_000001F9);
})();
    }
    if (index < 0)
    {
throw ([&]() {
auto __ctor_arg_000001FA = "index";
auto __ctor_arg_000001FB = "Index cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FA, __ctor_arg_000001FB);
})();
    }
this->Semantic = semantic;
this->Index = index;
this->Format = format;
}

