#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBinaryAsset.hpp"
#include "ShaderProgramBinary.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderBinaryAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

ShaderBinaryAsset::ShaderBinaryAsset() : Bytecode(), ProgramName(), Stage(), TargetName(), Variant()
{
}

::ShaderBinaryAsset* ShaderBinaryAsset::FromBinary(::ShaderProgramBinary* binary)
{
    if (binary == nullptr)
    {
throw new ArgumentNullException("binary");
    }
    if (binary->get_Bytecode() == nullptr || binary->get_Bytecode()->Length == 0)
    {
throw new InvalidOperationException("Shader binary must include embedded bytecode for packaging.");
    }
::ShaderBinaryAsset *asset = ([&]() {
auto __object_00000160 = new ::ShaderBinaryAsset();
__object_00000160->ProgramName = binary->get_ProgramName();
__object_00000160->Stage = binary->get_Stage();
__object_00000160->TargetName = binary->get_Target();
__object_00000160->Variant = binary->get_Variant();
__object_00000160->Bytecode = binary->get_Bytecode();
return __object_00000160;
})();
return asset;}

::ShaderProgramBinary* ShaderBinaryAsset::ToBinary()
{
this->Validate();
return new ::ShaderProgramBinary(this->ProgramName, this->Stage, this->TargetName, this->Variant, this->Bytecode);}

void ShaderBinaryAsset::Validate()
{
    if (String::IsNullOrWhiteSpace(this->ProgramName))
    {
throw new InvalidOperationException("Binary program name must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->TargetName))
    {
throw new InvalidOperationException("Binary target name must be provided.");
    }
else     if (String::IsNullOrWhiteSpace(this->Variant))
    {
throw new InvalidOperationException("Binary variant name must be provided.");
    }
else     if (this->Bytecode == nullptr || this->Bytecode->Length == 0)
    {
throw new InvalidOperationException("Binary bytecode payload must be provided.");
    }
}

