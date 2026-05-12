#ifdef DrawText
#undef DrawText
#endif
#include "SceneComponentAssetRecord.hpp"
#include "runtime/array.hpp"

SceneComponentAssetRecord::SceneComponentAssetRecord() : ComponentIndex(0), ComponentKey(), ComponentTypeId(), Payload(Array<uint8_t>::Empty())
{
}

int32_t SceneComponentAssetRecord::get_ComponentIndex()
{
return this->ComponentIndex;
}

void SceneComponentAssetRecord::set_ComponentIndex(int32_t value)
{
this->ComponentIndex = value;
}

std::string SceneComponentAssetRecord::get_ComponentKey()
{
return this->ComponentKey;
}

void SceneComponentAssetRecord::set_ComponentKey(std::string value)
{
this->ComponentKey = value;
}

std::string SceneComponentAssetRecord::get_ComponentTypeId()
{
return this->ComponentTypeId;
}

void SceneComponentAssetRecord::set_ComponentTypeId(std::string value)
{
this->ComponentTypeId = value;
}

Array<uint8_t>* SceneComponentAssetRecord::get_Payload()
{
return this->Payload;
}

void SceneComponentAssetRecord::set_Payload(Array<uint8_t>* value)
{
this->Payload = value;
}

