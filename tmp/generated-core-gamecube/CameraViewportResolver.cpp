#ifdef DrawText
#undef DrawText
#endif
#include "CameraViewportResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "runtime/native_exceptions.hpp"

::float4 CameraViewportResolver::ResolveViewport(::float4 viewport, double targetWidth, double targetHeight)
{
    if (targetWidth <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000022 = "targetWidth";
auto __ctor_arg_00000023 = "Target width must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000022, __ctor_arg_00000023);
})();
    }
    if (targetHeight <= 0.0)
    {
throw ([&]() {
auto __ctor_arg_00000024 = "targetHeight";
auto __ctor_arg_00000025 = "Target height must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_00000024, __ctor_arg_00000025);
})();
    }
double offsetX = viewport.X;
double offsetY = viewport.Y;
double width = viewport.Z;
double height = viewport.W;
    if (width <= 1.0 && height <= 1.0)
    {
offsetX *= targetWidth;
offsetY *= targetHeight;
width *= targetWidth;
height *= targetHeight;
    }
return ::float4(static_cast<float>(offsetX), static_cast<float>(offsetY), static_cast<float>(width), static_cast<float>(height));}

