#ifdef DrawText
#undef DrawText
#endif
#include "MaterialConstantBufferAsset.hpp"
#include "runtime/array.hpp"

Array<uint8_t>* MaterialConstantBufferAsset::get_Data()
{
return this->Data;
}

void MaterialConstantBufferAsset::set_Data(Array<uint8_t>* value)
{
this->Data = value;
}

std::string MaterialConstantBufferAsset::get_Name()
{
return this->Name;
}

void MaterialConstantBufferAsset::set_Name(std::string value)
{
this->Name = value;
}

MaterialConstantBufferAsset::MaterialConstantBufferAsset() : Data(), Name()
{
this->set_Data(Array<uint8_t>::Empty());
}

