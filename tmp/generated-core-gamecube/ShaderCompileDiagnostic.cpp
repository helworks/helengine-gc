#ifdef DrawText
#undef DrawText
#endif
#include "ShaderCompileDiagnostic.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

int32_t ShaderCompileDiagnostic::get_Column()
{
return this->Column;
}

std::string ShaderCompileDiagnostic::get_FilePath()
{
return this->FilePath;
}

int32_t ShaderCompileDiagnostic::get_Line()
{
return this->Line;
}

std::string ShaderCompileDiagnostic::get_Message()
{
return this->Message;
}

::ShaderDiagnosticSeverity ShaderCompileDiagnostic::get_Severity()
{
return this->Severity;
}

ShaderCompileDiagnostic::ShaderCompileDiagnostic(::ShaderDiagnosticSeverity severity, std::string message, std::string filePath, int32_t line, int32_t column) : Column(0), FilePath(), Line(0), Message(), Severity()
{
    if (String::IsNullOrWhiteSpace(message))
    {
throw ([&]() {
auto __ctor_arg_00000180 = "Diagnostic message must be provided.";
auto __ctor_arg_00000181 = "message";
return new ArgumentException(__ctor_arg_00000180, __ctor_arg_00000181);
})();
    }
    if (String::IsNullOrEmpty(filePath))
    {
throw new ArgumentNullException("filePath");
    }
    if (line < 0)
    {
throw ([&]() {
auto __ctor_arg_00000182 = "line";
auto __ctor_arg_00000183 = "Line cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000182, __ctor_arg_00000183);
})();
    }
    if (column < 0)
    {
throw ([&]() {
auto __ctor_arg_00000184 = "column";
auto __ctor_arg_00000185 = "Column cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000184, __ctor_arg_00000185);
})();
    }
this->Severity = severity;
this->Message = message;
this->FilePath = filePath;
this->Line = line;
this->Column = column;
}

