#ifdef DrawText
#undef DrawText
#endif
#include "EditorPropertyOrderAttribute.hpp"

int32_t EditorPropertyOrderAttribute::get_Order()
{
return this->Order;
}

EditorPropertyOrderAttribute::EditorPropertyOrderAttribute(int32_t order) : Order(0)
{
this->Order = order;
}

