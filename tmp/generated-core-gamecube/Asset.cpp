#ifdef DrawText
#undef DrawText
#endif
#include "Asset.hpp"

Asset::Asset() : Id()
{
}

std::string Asset::get_Id()
{
return this->Id;
}

void Asset::set_Id(std::string value)
{
this->Id = value;
}

