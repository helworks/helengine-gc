#ifdef DrawText
#undef DrawText
#endif
#include "ModelAsset.hpp"
#include "runtime/array.hpp"

ModelAsset::ModelAsset() : BoundsMax(), BoundsMin(), Indices16(), Indices32(), Normals(), Positions(), Ps2PackedMeshBytes(), Submeshes(), TexCoords()
{
}

std::string ModelAsset::get_Id()
{
return this->Asset::get_Id();
}

void ModelAsset::set_Id(std::string value)
{
this->Asset::set_Id(value);
}

