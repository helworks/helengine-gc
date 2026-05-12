#ifdef DrawText
#undef DrawText
#endif
#include "ShaderModuleDefinition.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "ShaderProgramBinary.hpp"
#include "ShaderProgramDefinition.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

List<::ShaderProgramBinary*>* ShaderModuleDefinition::get_Binaries()
{
return new List<ShaderProgramBinary*>(this->binaries);}

std::string ShaderModuleDefinition::get_Name()
{
return this->Name;
}

List<::ShaderProgramDefinition*>* ShaderModuleDefinition::get_Programs()
{
return new List<ShaderProgramDefinition*>(this->programs);}

::ShaderProgramBinary* ShaderModuleDefinition::GetBinary(std::string programName, std::string target, std::string variant)
{
::ShaderProgramBinary *binary;
    if (this->TryGetBinary(programName, target, variant, binary))
    {
return binary;    }
throw new InvalidOperationException("No compiled shader binary was found for the requested selection.");
}

::ShaderProgramDefinition* ShaderModuleDefinition::GetProgram(std::string programName)
{
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_000001BC = "Program name must be provided.";
auto __ctor_arg_000001BD = "programName";
return new ArgumentException(__ctor_arg_000001BC, __ctor_arg_000001BD);
})();
    }
for (int32_t i = 0; i < this->programs->Length; i++) {
::ShaderProgramDefinition *program = (*this->programs)[i];
    if (String::Equals(program->get_Name(), programName, StringComparison::Ordinal))
    {
return program;    }
}
throw new InvalidOperationException("No shader program was found for the requested name.");
}

ShaderModuleDefinition::ShaderModuleDefinition(std::string name, Array<::ShaderProgramDefinition*>* programs, Array<::ShaderProgramBinary*>* binaries) : Name(), binaries(), programs()
{
    if (String::IsNullOrWhiteSpace(name))
    {
throw ([&]() {
auto __ctor_arg_000001BE = "Module name must be provided.";
auto __ctor_arg_000001BF = "name";
return new ArgumentException(__ctor_arg_000001BE, __ctor_arg_000001BF);
})();
    }
    if (programs == nullptr)
    {
throw new ArgumentNullException("programs");
    }
    if (programs->Length == 0)
    {
throw ([&]() {
auto __ctor_arg_000001C0 = "At least one program definition is required.";
auto __ctor_arg_000001C1 = "programs";
return new ArgumentException(__ctor_arg_000001C0, __ctor_arg_000001C1);
})();
    }
    if (binaries == nullptr)
    {
throw new ArgumentNullException("binaries");
    }
this->Name = name;
this->programs = programs;
this->binaries = binaries;
}

bool ShaderModuleDefinition::TryGetBinary(std::string programName, std::string target, std::string variant, ::ShaderProgramBinary*& binary)
{
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_000001C2 = "Program name must be provided.";
auto __ctor_arg_000001C3 = "programName";
return new ArgumentException(__ctor_arg_000001C2, __ctor_arg_000001C3);
})();
    }
    if (String::IsNullOrWhiteSpace(target))
    {
throw ([&]() {
auto __ctor_arg_000001C4 = "Target must be provided.";
auto __ctor_arg_000001C5 = "target";
return new ArgumentException(__ctor_arg_000001C4, __ctor_arg_000001C5);
})();
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_000001C6 = "Variant must be provided.";
auto __ctor_arg_000001C7 = "variant";
return new ArgumentException(__ctor_arg_000001C6, __ctor_arg_000001C7);
})();
    }
for (int32_t i = 0; i < this->binaries->Length; i++) {
::ShaderProgramBinary *candidate = (*this->binaries)[i];
    if (!String::Equals(candidate->get_ProgramName(), programName, StringComparison::Ordinal))
    {
continue;
    }
    if (!String::Equals(candidate->get_Target(), target, StringComparison::OrdinalIgnoreCase))
    {
continue;
    }
    if (String::Equals(candidate->get_Variant(), variant, StringComparison::Ordinal))
    {
binary = candidate;
return true;    }
}
binary = nullptr;
return false;}

bool ShaderModuleDefinition::TryGetProgram(std::string programName, ::ShaderProgramDefinition*& program)
{
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_000001C8 = "Program name must be provided.";
auto __ctor_arg_000001C9 = "programName";
return new ArgumentException(__ctor_arg_000001C8, __ctor_arg_000001C9);
})();
    }
for (int32_t i = 0; i < this->programs->Length; i++) {
::ShaderProgramDefinition *candidate = (*this->programs)[i];
    if (String::Equals(candidate->get_Name(), programName, StringComparison::Ordinal))
    {
program = candidate;
return true;    }
}
program = nullptr;
return false;}

