#ifdef DrawText
#undef DrawText
#endif
#include "ShaderProgramDefinition.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

List<::ShaderBinding*>* ShaderProgramDefinition::get_Bindings()
{
return new List<ShaderBinding*>(this->bindings);}

std::string ShaderProgramDefinition::get_EntryPoint()
{
return this->EntryPoint;
}

List<::ShaderVertexElement*>* ShaderProgramDefinition::get_Inputs()
{
return new List<ShaderVertexElement*>(this->inputs);}

std::string ShaderProgramDefinition::get_Name()
{
return this->Name;
}

List<::ShaderVertexElement*>* ShaderProgramDefinition::get_Outputs()
{
return new List<ShaderVertexElement*>(this->outputs);}

::ShaderStage ShaderProgramDefinition::get_Stage()
{
return this->Stage;
}

List<::ShaderVariant*>* ShaderProgramDefinition::get_Variants()
{
return new List<ShaderVariant*>(this->variants);}

ShaderProgramDefinition::ShaderProgramDefinition(std::string name, ::ShaderStage stage, std::string entryPoint, Array<::ShaderBinding*>* bindings, Array<::ShaderVertexElement*>* inputs, Array<::ShaderVertexElement*>* outputs, Array<::ShaderVariant*>* variants) : EntryPoint(), Name(), Stage(), bindings(), inputs(), outputs(), variants()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001E7 = "Program name must be provided.";
auto __ctor_arg_000001E8 = "name";
return new ArgumentException(__ctor_arg_000001E7, __ctor_arg_000001E8);
})();
    }
    if (String::IsNullOrWhiteSpace(entryPoint))
    {
throw ([&]() {
auto __ctor_arg_000001E9 = "Entry point must be provided.";
auto __ctor_arg_000001EA = "entryPoint";
return new ArgumentException(__ctor_arg_000001E9, __ctor_arg_000001EA);
})();
    }
    if (bindings == nullptr)
    {
throw new ArgumentNullException("bindings");
    }
    if (inputs == nullptr)
    {
throw new ArgumentNullException("inputs");
    }
    if (outputs == nullptr)
    {
throw new ArgumentNullException("outputs");
    }
    if (variants == nullptr)
    {
throw new ArgumentNullException("variants");
    }
this->Name = name;
this->Stage = stage;
this->EntryPoint = entryPoint;
this->bindings = bindings;
this->inputs = inputs;
this->outputs = outputs;
this->variants = variants;
}

