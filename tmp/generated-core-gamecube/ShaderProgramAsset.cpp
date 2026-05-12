#ifdef DrawText
#undef DrawText
#endif
#include "ShaderProgramAsset.hpp"
#include "runtime/array.hpp"
#include "ShaderProgramDefinition.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderProgramAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "ShaderBinding.hpp"
#include "ShaderVertexElement.hpp"
#include "ShaderVariant.hpp"
#include "ShaderBindingAsset.hpp"
#include "ShaderVertexElementAsset.hpp"
#include "ShaderVariantAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

::ShaderProgramAsset* ShaderProgramAsset::FromDefinition(::ShaderProgramDefinition* definition)
{
    if (definition == nullptr)
    {
throw new ArgumentNullException("definition");
    }
::ShaderProgramAsset *asset = ([&]() {
auto __object_000001D6 = new ::ShaderProgramAsset();
__object_000001D6->Name = definition->get_Name();
__object_000001D6->Stage = definition->get_Stage();
__object_000001D6->EntryPoint = definition->get_EntryPoint();
__object_000001D6->Bindings = BuildBindingAssets(definition);
__object_000001D6->Inputs = BuildInputAssets(definition);
__object_000001D6->Outputs = BuildOutputAssets(definition);
__object_000001D6->Variants = BuildVariantAssets(definition);
return __object_000001D6;
})();
return asset;}

ShaderProgramAsset::ShaderProgramAsset() : Bindings(), EntryPoint(), Inputs(), Name(), Outputs(), Stage(), Variants()
{
this->Bindings = Array<ShaderBindingAsset*>::Empty();
this->Inputs = Array<ShaderVertexElementAsset*>::Empty();
this->Outputs = Array<ShaderVertexElementAsset*>::Empty();
this->Variants = Array<ShaderVariantAsset*>::Empty();
}

::ShaderProgramDefinition* ShaderProgramAsset::ToDefinition()
{
this->Validate();
Array<::ShaderBinding*> *bindingDefinitions = this->BuildBindings();
Array<::ShaderVertexElement*> *inputDefinitions = this->BuildInputs();
Array<::ShaderVertexElement*> *outputDefinitions = this->BuildOutputs();
Array<::ShaderVariant*> *variantDefinitions = this->BuildVariants();
return new ::ShaderProgramDefinition(this->Name, this->Stage, this->EntryPoint, bindingDefinitions, inputDefinitions, outputDefinitions, variantDefinitions);}

Array<::ShaderBindingAsset*>* ShaderProgramAsset::BuildBindingAssets(::ShaderProgramDefinition* definition)
{
List<::ShaderBinding*> *bindings = definition->get_Bindings();
Array<::ShaderBindingAsset*> *assets = new Array<ShaderBindingAsset*>(bindings->get_Count());
for (int32_t i = 0; i < bindings->get_Count(); i++) {
(*assets)[i] = ShaderBindingAsset::FromBinding((*bindings)[i]);
}
return assets;}

Array<::ShaderBinding*>* ShaderProgramAsset::BuildBindings()
{
Array<::ShaderBinding*> *definitions = new Array<ShaderBinding*>(this->Bindings->Length);
for (int32_t i = 0; i < this->Bindings->Length; i++) {
::ShaderBindingAsset *binding = (*this->Bindings)[i];
    if (binding == nullptr)
    {
throw new InvalidOperationException("Program bindings contain a null entry.");
    }
(*definitions)[i] = binding->ToBinding();
}
return definitions;}

Array<::ShaderVertexElementAsset*>* ShaderProgramAsset::BuildInputAssets(::ShaderProgramDefinition* definition)
{
List<::ShaderVertexElement*> *inputs = definition->get_Inputs();
Array<::ShaderVertexElementAsset*> *assets = new Array<ShaderVertexElementAsset*>(inputs->get_Count());
for (int32_t i = 0; i < inputs->get_Count(); i++) {
(*assets)[i] = ShaderVertexElementAsset::FromVertexElement((*inputs)[i]);
}
return assets;}

Array<::ShaderVertexElement*>* ShaderProgramAsset::BuildInputs()
{
Array<::ShaderVertexElement*> *definitions = new Array<ShaderVertexElement*>(this->Inputs->Length);
for (int32_t i = 0; i < this->Inputs->Length; i++) {
::ShaderVertexElementAsset *element = (*this->Inputs)[i];
    if (element == nullptr)
    {
throw new InvalidOperationException("Program inputs contain a null entry.");
    }
(*definitions)[i] = element->ToVertexElement();
}
return definitions;}

Array<::ShaderVertexElementAsset*>* ShaderProgramAsset::BuildOutputAssets(::ShaderProgramDefinition* definition)
{
List<::ShaderVertexElement*> *outputs = definition->get_Outputs();
Array<::ShaderVertexElementAsset*> *assets = new Array<ShaderVertexElementAsset*>(outputs->get_Count());
for (int32_t i = 0; i < outputs->get_Count(); i++) {
(*assets)[i] = ShaderVertexElementAsset::FromVertexElement((*outputs)[i]);
}
return assets;}

Array<::ShaderVertexElement*>* ShaderProgramAsset::BuildOutputs()
{
Array<::ShaderVertexElement*> *definitions = new Array<ShaderVertexElement*>(this->Outputs->Length);
for (int32_t i = 0; i < this->Outputs->Length; i++) {
::ShaderVertexElementAsset *element = (*this->Outputs)[i];
    if (element == nullptr)
    {
throw new InvalidOperationException("Program outputs contain a null entry.");
    }
(*definitions)[i] = element->ToVertexElement();
}
return definitions;}

Array<::ShaderVariantAsset*>* ShaderProgramAsset::BuildVariantAssets(::ShaderProgramDefinition* definition)
{
List<::ShaderVariant*> *variants = definition->get_Variants();
Array<::ShaderVariantAsset*> *assets = new Array<ShaderVariantAsset*>(variants->get_Count());
for (int32_t i = 0; i < variants->get_Count(); i++) {
(*assets)[i] = ShaderVariantAsset::FromVariant((*variants)[i]);
}
return assets;}

Array<::ShaderVariant*>* ShaderProgramAsset::BuildVariants()
{
Array<::ShaderVariant*> *definitions = new Array<ShaderVariant*>(this->Variants->Length);
for (int32_t i = 0; i < this->Variants->Length; i++) {
::ShaderVariantAsset *variant = (*this->Variants)[i];
    if (variant == nullptr)
    {
throw new InvalidOperationException("Program variants contain a null entry.");
    }
(*definitions)[i] = variant->ToVariant();
}
return definitions;}

void ShaderProgramAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Name))
    {
throw new InvalidOperationException("Program name must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->EntryPoint))
    {
throw new InvalidOperationException("Program entry point must be provided.");
    }
else     if (this->Bindings == nullptr)
    {
throw new InvalidOperationException("Program bindings must be provided.");
    }
else     if (this->Inputs == nullptr)
    {
throw new InvalidOperationException("Program inputs must be provided.");
    }
else     if (this->Outputs == nullptr)
    {
throw new InvalidOperationException("Program outputs must be provided.");
    }
else     if (this->Variants == nullptr)
    {
throw new InvalidOperationException("Program variants must be provided.");
    }
}

