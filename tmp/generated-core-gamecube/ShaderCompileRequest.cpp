#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileRequest.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

List<::ShaderDefine*>* ShaderCompileRequest::get_Defines()
{
return this->Defines;
}

std::string ShaderCompileRequest::get_EntryPoint()
{
return this->EntryPoint;
}

::ShaderCompileOptions* ShaderCompileRequest::get_Options()
{
return this->Options;
}

std::string ShaderCompileRequest::get_ProgramName()
{
return this->ProgramName;
}

::ShaderModel* ShaderCompileRequest::get_ShaderModel()
{
return this->ShaderModel;
}

::ShaderSourceInfo* ShaderCompileRequest::get_Source()
{
return this->Source;
}

::ShaderStage ShaderCompileRequest::get_Stage()
{
return this->Stage;
}

::ShaderCompileTarget ShaderCompileRequest::get_Target()
{
return this->Target;
}

std::string ShaderCompileRequest::get_Variant()
{
return this->Variant;
}

ShaderCompileRequest::ShaderCompileRequest(::ShaderSourceInfo* source, std::string programName, std::string entryPoint, ::ShaderStage stage, ::ShaderCompileTarget target, ::ShaderModel* shaderModel, std::string variant, List<::ShaderDefine*>* defines, ::ShaderCompileOptions* options) : Defines(), EntryPoint(), Options(), ProgramName(), ShaderModel(), Source(), Stage(), Target(), Variant()
{
    if (source == nullptr)
    {
throw new ArgumentNullException("source");
    }
    if (String::IsNullOrWhiteSpace(programName))
    {
throw ([&]() {
auto __ctor_arg_00000186 = "Program name must be provided.";
auto __ctor_arg_00000187 = "programName";
return new ArgumentException(__ctor_arg_00000186, __ctor_arg_00000187);
})();
    }
    if (String::IsNullOrWhiteSpace(entryPoint))
    {
throw ([&]() {
auto __ctor_arg_00000188 = "Entry point must be provided.";
auto __ctor_arg_00000189 = "entryPoint";
return new ArgumentException(__ctor_arg_00000188, __ctor_arg_00000189);
})();
    }
    if (shaderModel == nullptr)
    {
throw new ArgumentNullException("shaderModel");
    }
    if (String::IsNullOrWhiteSpace(variant))
    {
throw ([&]() {
auto __ctor_arg_0000018A = "Variant name must be provided.";
auto __ctor_arg_0000018B = "variant";
return new ArgumentException(__ctor_arg_0000018A, __ctor_arg_0000018B);
})();
    }
    if (defines == nullptr)
    {
throw new ArgumentNullException("defines");
    }
    if (options == nullptr)
    {
throw new ArgumentNullException("options");
    }
this->Source = source;
this->ProgramName = programName;
this->EntryPoint = entryPoint;
this->Stage = stage;
this->Target = target;
this->ShaderModel = shaderModel;
this->Variant = variant;
this->Defines = defines;
this->Options = options;
}

