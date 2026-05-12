#ifdef DrawText
#undef DrawText
#endif
#include "ClipRectComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "runtime/native_exceptions.hpp"

ClipRectComponent::ClipRectComponent() : SizeValue()
{
}

::int2 ClipRectComponent::get_Size()
{
return this->SizeValue;}

void ClipRectComponent::set_Size(::int2 value)
{
    if (value.X < 0 || value.Y < 0)
    {
throw ([&]() {
auto __ctor_arg_00000282 = "value";
auto __ctor_arg_00000283 = "Clip rectangle size must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000282, __ctor_arg_00000283);
})();
    }
this->SizeValue = value;
}

::float4 ClipRectComponent::GetClipRect()
{
    if (Parent == nullptr)
    {
throw new InvalidOperationException("Clip rectangles require an attached parent entity.");
    }
return ::float4(Parent->get_Position().X, Parent->get_Position().Y, this->SizeValue.X, this->SizeValue.Y);}

::Entity* ClipRectComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ClipRectComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

