#ifdef DrawText
#undef DrawText
#endif
#include "ShaderVariantAsset.hpp"
#include "ShaderVariant.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "ShaderVariantAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

ShaderVariantAsset::ShaderVariantAsset() : Defines(), Name()
{
}

::ShaderVariantAsset* ShaderVariantAsset::FromVariant(::ShaderVariant* variant)
{
    if (variant == nullptr)
    {
throw new ArgumentNullException("variant");
    }
Array<std::string> *defines = BuildDefines(variant);
::ShaderVariantAsset *asset = ([&]() {
auto __object_000001F5 = new ::ShaderVariantAsset();
__object_000001F5->Name = variant->get_Name();
__object_000001F5->Defines = defines;
return __object_000001F5;
})();
return asset;}

::ShaderVariant* ShaderVariantAsset::ToVariant()
{
this->Validate();
return new ::ShaderVariant(this->Name, this->Defines);}

Array<std::string>* ShaderVariantAsset::BuildDefines(::ShaderVariant* variant)
{
List<std::string> *defines = variant->get_Defines();
Array<std::string> *result = new Array<std::string>(defines->get_Count());
for (int32_t i = 0; i < defines->get_Count(); i++) {
(*result)[i] = (*defines)[i];
}
return result;}

void ShaderVariantAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Name))
    {
throw new InvalidOperationException("Variant name must be provided.");
    }
else     if (this->Defines == nullptr)
    {
throw new InvalidOperationException("Variant defines must be provided.");
    }
}

