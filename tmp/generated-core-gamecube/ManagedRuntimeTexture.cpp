#ifdef DrawText
#undef DrawText
#endif
#include "ManagedRuntimeTexture.hpp"

int32_t ManagedRuntimeTexture::get_Height()
{
return this->RuntimeTexture::get_Height();
}

void ManagedRuntimeTexture::set_Height(int32_t value)
{
this->RuntimeTexture::set_Height(value);
}

int32_t ManagedRuntimeTexture::get_Width()
{
return this->RuntimeTexture::get_Width();
}

void ManagedRuntimeTexture::set_Width(int32_t value)
{
this->RuntimeTexture::set_Width(value);
}

std::string ManagedRuntimeTexture::get_Id()
{
return this->RuntimeData::get_Id();
}

void ManagedRuntimeTexture::set_Id(std::string value)
{
this->RuntimeData::set_Id(value);
}

