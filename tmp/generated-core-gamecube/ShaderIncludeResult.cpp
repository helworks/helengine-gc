#ifdef DrawText
#undef DrawText
#endif
#include "ShaderIncludeResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string ShaderIncludeResult::get_Path()
{
return this->Path;
}

std::string ShaderIncludeResult::get_Source()
{
return this->Source;
}

ShaderIncludeResult::ShaderIncludeResult(std::string path, std::string source) : Path(), Source()
{
    if (String::IsNullOrWhiteSpace(path))
    {
throw ([&]() {
auto __ctor_arg_000001B2 = "Include path must be provided.";
auto __ctor_arg_000001B3 = "path";
return new ArgumentException(__ctor_arg_000001B2, __ctor_arg_000001B3);
})();
    }
    if (String::IsNullOrWhiteSpace(source))
    {
throw ([&]() {
auto __ctor_arg_000001B4 = "Include source must be provided.";
auto __ctor_arg_000001B5 = "source";
return new ArgumentException(__ctor_arg_000001B4, __ctor_arg_000001B5);
})();
    }
this->Path = path;
this->Source = source;
}

