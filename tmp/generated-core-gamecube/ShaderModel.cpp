#ifdef DrawText
#undef DrawText
#endif
#include "ShaderModel.hpp"
#include "runtime/native_string.hpp"
#include "ShaderStage.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

int32_t ShaderModel::get_Major()
{
return this->Major;
}

int32_t ShaderModel::get_Minor()
{
return this->Minor;
}

std::string ShaderModel::GetProfile(::ShaderStage stage)
{
const std::string prefix = this->GetStagePrefix(stage);
return String::Concat(prefix, "_", std::to_string(this->Major), "_", std::to_string(this->Minor));}

ShaderModel::ShaderModel(int32_t major, int32_t minor) : Major(0), Minor(0)
{
    if (major < 0)
    {
throw ([&]() {
auto __ctor_arg_000001B6 = "major";
auto __ctor_arg_000001B7 = "Major version cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B6, __ctor_arg_000001B7);
})();
    }
    if (minor < 0)
    {
throw ([&]() {
auto __ctor_arg_000001B8 = "minor";
auto __ctor_arg_000001B9 = "Minor version cannot be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B8, __ctor_arg_000001B9);
})();
    }
this->Major = major;
this->Minor = minor;
}

std::string ShaderModel::ToString()
{
return String::Concat(std::to_string(this->Major), ".", std::to_string(this->Minor));}

std::string ShaderModel::GetStagePrefix(::ShaderStage stage)
{
switch (stage) {
case ShaderStage::Vertex: {
return "vs";}
case ShaderStage::Pixel: {
return "ps";}
case ShaderStage::Geometry: {
return "gs";}
case ShaderStage::Hull: {
return "hs";}
case ShaderStage::Domain: {
return "ds";}
case ShaderStage::Compute: {
return "cs";}
default:  {
throw ([&]() {
auto __ctor_arg_000001BA = "stage";
auto __ctor_arg_000001BB = "Unsupported shader stage.";
return new ArgumentOutOfRangeException(__ctor_arg_000001BA, __ctor_arg_000001BB);
})();
}
}

}

