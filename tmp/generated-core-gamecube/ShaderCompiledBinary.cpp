#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompiledBinary.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<uint8_t>* ShaderCompiledBinary::get_Bytecode()
{
return this->Bytecode;
}

std::string ShaderCompiledBinary::get_EntryPoint()
{
return this->EntryPoint;
}

::ShaderStage ShaderCompiledBinary::get_Stage()
{
return this->Stage;
}

::ShaderCompileTarget ShaderCompiledBinary::get_Target()
{
return this->Target;
}

std::string ShaderCompiledBinary::get_Variant()
{
return this->Variant;
}

ShaderCompiledBinary::ShaderCompiledBinary(::ShaderCompileTarget target, ::ShaderStage stage, std::string entryPoint, std::string variant, Array<uint8_t>* bytecode) : Bytecode(), EntryPoint(), Stage(), Target(), Variant()
{
    if (String::IsNullOrWhiteSpace(entryPoint))
    {
throw ([&]() {
auto __ctor_arg_0000018E = "Entry point must be provided.";
auto __ctor_arg_0000018F = "entryPoint";
return new ArgumentException(__ctor_arg_0000018E, __ctor_arg_0000018F);
})();
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_00000190 = "Variant name must be provided.";
auto __ctor_arg_00000191 = "variant";
return new ArgumentException(__ctor_arg_00000190, __ctor_arg_00000191);
})();
    }
    if (bytecode == nullptr)
    {
throw new ArgumentNullException("bytecode");
    }
    if (bytecode->Length == 0)
    {
throw ([&]() {
auto __ctor_arg_00000192 = "Bytecode payload must be provided.";
auto __ctor_arg_00000193 = "bytecode";
return new ArgumentException(__ctor_arg_00000192, __ctor_arg_00000193);
})();
    }
this->Target = target;
this->Stage = stage;
this->EntryPoint = entryPoint;
this->Variant = variant;
this->Bytecode = bytecode;
}

