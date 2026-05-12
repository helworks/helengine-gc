#ifdef DrawText
#undef DrawText
#endif
#include "ShaderProgramBinary.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<uint8_t>* ShaderProgramBinary::get_Bytecode()
{
return this->bytecode;}

std::string ShaderProgramBinary::get_Path()
{
return this->Path;
}

std::string ShaderProgramBinary::get_ProgramName()
{
return this->ProgramName;
}

::ShaderStage ShaderProgramBinary::get_Stage()
{
return this->Stage;
}

std::string ShaderProgramBinary::get_Target()
{
return this->Target;
}

std::string ShaderProgramBinary::get_Variant()
{
return this->Variant;
}

ShaderProgramBinary::ShaderProgramBinary(std::string programName, ::ShaderStage stage, std::string target, std::string variant, std::string path) : Path(), ProgramName(), Stage(), Target(), Variant(), bytecode()
{
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_000001D7 = "Program name must be provided.";
auto __ctor_arg_000001D8 = "programName";
return new ArgumentException(__ctor_arg_000001D7, __ctor_arg_000001D8);
})();
    }
    if (String::IsNullOrWhiteSpace(target))
    {
throw ([&]() {
auto __ctor_arg_000001D9 = "Target must be provided.";
auto __ctor_arg_000001DA = "target";
return new ArgumentException(__ctor_arg_000001D9, __ctor_arg_000001DA);
})();
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_000001DB = "Variant must be provided.";
auto __ctor_arg_000001DC = "variant";
return new ArgumentException(__ctor_arg_000001DB, __ctor_arg_000001DC);
})();
    }
    if (String::IsNullOrWhiteSpace(path))
    {
throw ([&]() {
auto __ctor_arg_000001DD = "Path must be provided.";
auto __ctor_arg_000001DE = "path";
return new ArgumentException(__ctor_arg_000001DD, __ctor_arg_000001DE);
})();
    }
this->ProgramName = programName;
this->Stage = stage;
this->Target = target;
this->Variant = variant;
this->Path = path;
this->bytecode = nullptr;
}

ShaderProgramBinary::ShaderProgramBinary(std::string programName, ::ShaderStage stage, std::string target, std::string variant, Array<uint8_t>* bytecode) : Path(), ProgramName(), Stage(), Target(), Variant(), bytecode()
{
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_000001DF = "Program name must be provided.";
auto __ctor_arg_000001E0 = "programName";
return new ArgumentException(__ctor_arg_000001DF, __ctor_arg_000001E0);
})();
    }
    if (String::IsNullOrWhiteSpace(target))
    {
throw ([&]() {
auto __ctor_arg_000001E1 = "Target must be provided.";
auto __ctor_arg_000001E2 = "target";
return new ArgumentException(__ctor_arg_000001E1, __ctor_arg_000001E2);
})();
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_000001E3 = "Variant must be provided.";
auto __ctor_arg_000001E4 = "variant";
return new ArgumentException(__ctor_arg_000001E3, __ctor_arg_000001E4);
})();
    }
    if (bytecode == nullptr)
    {
throw new ArgumentNullException("bytecode");
    }
    if (bytecode->Length == 0)
    {
throw ([&]() {
auto __ctor_arg_000001E5 = "Bytecode payload must be provided.";
auto __ctor_arg_000001E6 = "bytecode";
return new ArgumentException(__ctor_arg_000001E5, __ctor_arg_000001E6);
})();
    }
this->ProgramName = programName;
this->Stage = stage;
this->Target = target;
this->Variant = variant;
this->Path = String::Empty;
this->bytecode = bytecode;
}

