#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::ShaderCompiledBinary* ShaderCompileResult::get_Binary()
{
return this->Binary;
}

List<::ShaderCompileDiagnostic*>* ShaderCompileResult::get_Diagnostics()
{
return this->Diagnostics;
}

::ShaderProgramDefinition* ShaderCompileResult::get_ProgramDefinition()
{
return this->ProgramDefinition;
}

::ShaderCompileRequest* ShaderCompileResult::get_Request()
{
return this->Request;
}

bool ShaderCompileResult::get_Success()
{
return this->Success;
}

ShaderCompileResult::ShaderCompileResult(::ShaderCompileRequest* request, ::ShaderProgramDefinition* programDefinition, ::ShaderCompiledBinary* binary, List<::ShaderCompileDiagnostic*>* diagnostics, bool success) : Binary(), Diagnostics(), ProgramDefinition(), Request(), Success()
{
    if (request == nullptr)
    {
throw new ArgumentNullException("request");
    }
    if (programDefinition == nullptr)
    {
throw new ArgumentNullException("programDefinition");
    }
    if (binary == nullptr)
    {
throw new ArgumentNullException("binary");
    }
    if (diagnostics == nullptr)
    {
throw new ArgumentNullException("diagnostics");
    }
this->Request = request;
this->ProgramDefinition = programDefinition;
this->Binary = binary;
this->Diagnostics = diagnostics;
this->Success = success;
}

