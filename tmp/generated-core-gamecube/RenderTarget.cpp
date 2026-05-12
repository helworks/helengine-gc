#ifdef DrawText
#undef DrawText
#endif
#include "RenderTarget.hpp"

RenderTarget::RenderTarget() : CanSampleAsTexture(), HasDepthBuffer()
{
}

bool RenderTarget::get_CanSampleAsTexture()
{
return this->CanSampleAsTexture;
}

void RenderTarget::set_CanSampleAsTexture(bool value)
{
this->CanSampleAsTexture = value;
}

bool RenderTarget::get_HasDepthBuffer()
{
return this->HasDepthBuffer;
}

void RenderTarget::set_HasDepthBuffer(bool value)
{
this->HasDepthBuffer = value;
}

int32_t RenderTarget::get_Height()
{
return this->RuntimeTexture::get_Height();
}

void RenderTarget::set_Height(int32_t value)
{
this->RuntimeTexture::set_Height(value);
}

int32_t RenderTarget::get_Width()
{
return this->RuntimeTexture::get_Width();
}

void RenderTarget::set_Width(int32_t value)
{
this->RuntimeTexture::set_Width(value);
}

std::string RenderTarget::get_Id()
{
return this->RuntimeData::get_Id();
}

void RenderTarget::set_Id(std::string value)
{
this->RuntimeData::set_Id(value);
}

