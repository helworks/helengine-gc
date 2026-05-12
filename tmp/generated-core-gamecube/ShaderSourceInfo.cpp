#ifdef DrawText
#undef DrawText
#endif
#include "ShaderSourceInfo.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderSourceInfo::get_Path()
{
return this->Path;
}

std::string ShaderSourceInfo::get_Source()
{
return this->Source;
}

ShaderSourceInfo::ShaderSourceInfo(std::string path, std::string source) : Path(), Source()
{
    if (String::IsNullOrWhiteSpace(path))
    {
throw ([&]() {
auto __ctor_arg_000001EB = "Shader path must be provided.";
auto __ctor_arg_000001EC = "path";
return new ArgumentException(__ctor_arg_000001EB, __ctor_arg_000001EC);
})();
    }
    if (String::IsNullOrWhiteSpace(source))
    {
throw ([&]() {
auto __ctor_arg_000001ED = "Shader source must be provided.";
auto __ctor_arg_000001EE = "source";
return new ArgumentException(__ctor_arg_000001ED, __ctor_arg_000001EE);
})();
    }
this->Path = path;
this->Source = source;
}

