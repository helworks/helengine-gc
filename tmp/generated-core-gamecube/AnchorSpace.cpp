#ifdef DrawText
#undef DrawText
#endif
#include "AnchorSpace.hpp"

::float2 AnchorSpace::get_Origin()
{
return this->Origin;
}

::int2 AnchorSpace::get_Size()
{
return this->Size;
}

AnchorSpace::AnchorSpace(::int2 size, ::float2 origin) : Origin(), Size()
{
this->Size = size;
this->Origin = origin;
}

