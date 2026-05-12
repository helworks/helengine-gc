#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeData.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

RuntimeData::RuntimeData() : Id()
{
}

std::string RuntimeData::get_Id()
{
return this->Id;
}

void RuntimeData::set_Id(std::string value)
{
this->Id = value;
}

void RuntimeData::SetId(std::string id)
{
    if (String::IsNullOrWhiteSpace(id))
    {
throw ([&]() {
auto __ctor_arg_00000127 = "Runtime asset id must be provided.";
auto __ctor_arg_00000128 = "id";
return new ArgumentException(__ctor_arg_00000127, __ctor_arg_00000128);
})();
    }
this->set_Id(id);
}

