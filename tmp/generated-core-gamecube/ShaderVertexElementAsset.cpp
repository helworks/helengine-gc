#ifdef DrawText
#undef DrawText
#endif
#include "ShaderVertexElementAsset.hpp"
#include "ShaderVertexElement.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderVertexElementAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

ShaderVertexElementAsset::ShaderVertexElementAsset() : Format(), Index(0), Semantic()
{
}

::ShaderVertexElementAsset* ShaderVertexElementAsset::FromVertexElement(::ShaderVertexElement* element)
{
    if (element == nullptr)
    {
throw new ArgumentNullException("element");
    }
::ShaderVertexElementAsset *asset = ([&]() {
auto __object_000001FC = new ::ShaderVertexElementAsset();
__object_000001FC->Semantic = element->get_Semantic();
__object_000001FC->Index = element->get_Index();
__object_000001FC->Format = element->get_Format();
return __object_000001FC;
})();
return asset;}

::ShaderVertexElement* ShaderVertexElementAsset::ToVertexElement()
{
this->Validate();
return new ::ShaderVertexElement(this->Semantic, this->Index, this->Format);}

void ShaderVertexElementAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Semantic))
    {
throw new InvalidOperationException("Vertex element semantic must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->Format))
    {
throw new InvalidOperationException("Vertex element format must be provided.");
    }
else     if (this->Index < 0)
    {
throw new InvalidOperationException("Vertex element index cannot be negative.");
    }
}

