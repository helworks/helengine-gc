#ifdef DrawText
#undef DrawText
#endif
#include "ShaderAsset.hpp"
#include "runtime/array.hpp"
#include "ShaderModuleDefinition.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "ShaderTargetNames.hpp"
#include "ShaderAsset.hpp"
#include "runtime/native_list.hpp"
#include "ShaderProgramDefinition.hpp"
#include "ShaderProgramBinary.hpp"
#include "ShaderProgramAsset.hpp"
#include "ShaderBinaryAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

ShaderAsset::ShaderAsset() : Binaries(), Name(), Programs(), TargetName()
{
}

::ShaderModuleDefinition* ShaderAsset::BuildDefinition()
{
this->Validate();
Array<::ShaderProgramDefinition*> *programDefinitions = this->BuildProgramDefinitions();
Array<::ShaderProgramBinary*> *binaryDefinitions = this->BuildBinaryDefinitions();
return new ::ShaderModuleDefinition(this->Name, programDefinitions, binaryDefinitions);}

::ShaderAsset* ShaderAsset::FromDefinition(::ShaderModuleDefinition* definition, ::ShaderCompileTarget target)
{
    if (definition == nullptr)
    {
throw new ArgumentNullException("definition");
    }
const std::string targetName = ShaderTargetNames::GetTargetName(target);
Array<::ShaderProgramAsset*> *programs = BuildProgramAssets(definition);
Array<::ShaderBinaryAsset*> *binaries = BuildBinaryAssets(definition, targetName);
::ShaderAsset *asset = ([&]() {
auto __object_00000274 = new ::ShaderAsset();
__object_00000274->set_Id(definition->get_Name());
__object_00000274->Name = definition->get_Name();
__object_00000274->TargetName = targetName;
__object_00000274->Programs = programs;
__object_00000274->Binaries = binaries;
return __object_00000274;
})();
return asset;}

std::string ShaderAsset::get_Id()
{
return this->Asset::get_Id();
}

void ShaderAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}

Array<::ShaderBinaryAsset*>* ShaderAsset::BuildBinaryAssets(::ShaderModuleDefinition* definition, std::string targetName)
{
List<::ShaderProgramBinary*> *binaries = definition->get_Binaries();
List<::ShaderBinaryAsset*> *assets = new List<::ShaderBinaryAsset*>();
for (int32_t i = 0; i < binaries->get_Count(); i++) {
::ShaderProgramBinary *binary = (*binaries)[i];
    if (!String::Equals(binary->get_Target(), targetName, StringComparison::OrdinalIgnoreCase))
    {
continue;
    }
assets->Add(ShaderBinaryAsset::FromBinary(binary));
}
    if (assets->Count() == 0)
    {
throw new InvalidOperationException("Shader module does not contain binaries for the requested target.");
    }
return assets->ToArray();}

Array<::ShaderProgramBinary*>* ShaderAsset::BuildBinaryDefinitions()
{
Array<::ShaderProgramBinary*> *binaries = new Array<ShaderProgramBinary*>(this->Binaries->Length);
for (int32_t i = 0; i < this->Binaries->Length; i++) {
::ShaderBinaryAsset *binary = (*this->Binaries)[i];
    if (binary == nullptr)
    {
throw new InvalidOperationException("Shader asset contains a null binary entry.");
    }
(*binaries)[i] = binary->ToBinary();
}
return binaries;}

Array<::ShaderProgramAsset*>* ShaderAsset::BuildProgramAssets(::ShaderModuleDefinition* definition)
{
List<::ShaderProgramDefinition*> *programs = definition->get_Programs();
Array<::ShaderProgramAsset*> *assets = new Array<ShaderProgramAsset*>(programs->get_Count());
for (int32_t i = 0; i < programs->get_Count(); i++) {
::ShaderProgramDefinition *program = (*programs)[i];
(*assets)[i] = ShaderProgramAsset::FromDefinition(program);
}
return assets;}

Array<::ShaderProgramDefinition*>* ShaderAsset::BuildProgramDefinitions()
{
Array<::ShaderProgramDefinition*> *definitions = new Array<ShaderProgramDefinition*>(this->Programs->Length);
for (int32_t i = 0; i < this->Programs->Length; i++) {
::ShaderProgramAsset *program = (*this->Programs)[i];
    if (program == nullptr)
    {
throw new InvalidOperationException("Shader asset contains a null program entry.");
    }
(*definitions)[i] = program->ToDefinition();
}
return definitions;}

void ShaderAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->Name))
    {
throw new InvalidOperationException("Shader asset name must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->TargetName))
    {
throw new InvalidOperationException("Shader asset target name must be provided.");
    }
else     if (this->Programs == nullptr)
    {
throw new InvalidOperationException("Shader asset programs must be provided.");
    }
else     if (this->Programs->Length == 0)
    {
throw new InvalidOperationException("Shader asset must include at least one program.");
    }
else     if (this->Binaries == nullptr)
    {
throw new InvalidOperationException("Shader asset binaries must be provided.");
    }
else     if (this->Binaries->Length == 0)
    {
throw new InvalidOperationException("Shader asset must include at least one binary.");
    }
}

