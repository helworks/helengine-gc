#ifdef DrawText
#undef DrawText
#endif
#include "ShaderBindingPolicies.hpp"
#include "ShaderBindingPolicy.hpp"

::ShaderBindingPolicy* ShaderBindingPolicies::get_Default()
{
return new ::ShaderBindingPolicy(0, 0, 100, 200, 300);}

