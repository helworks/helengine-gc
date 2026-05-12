#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::float3 RuntimeModel::get_BoundsMax()
{
return this->BoundsMax;
}

void RuntimeModel::set_BoundsMax(::float3 value)
{
this->BoundsMax = value;
}

::float3 RuntimeModel::get_BoundsMin()
{
return this->BoundsMin;
}

void RuntimeModel::set_BoundsMin(::float3 value)
{
this->BoundsMin = value;
}

Array<::RuntimeSubmesh*>* RuntimeModel::get_Submeshes()
{
return this->Submeshes;
}

void RuntimeModel::set_Submeshes(Array<::RuntimeSubmesh*>* value)
{
this->Submeshes = value;
}

void RuntimeModel::SetBounds(::float3 boundsMin, ::float3 boundsMax)
{
this->set_BoundsMin(boundsMin);
this->set_BoundsMax(boundsMax);
}

void RuntimeModel::SetSubmeshes(Array<::RuntimeSubmesh*>* submeshes)
{
    if (submeshes == nullptr)
    {
throw new ArgumentNullException("submeshes");
    }
Array<::RuntimeSubmesh*> *copiedSubmeshes = new Array<RuntimeSubmesh*>(submeshes->Length);
for (int32_t submeshIndex = 0; submeshIndex < submeshes->Length; submeshIndex++) {
::RuntimeSubmesh *submesh = (*submeshes)[submeshIndex];
    if (submesh == nullptr)
    {
throw new InvalidOperationException("Runtime model submesh collections cannot contain null entries.");
    }
(*copiedSubmeshes)[submeshIndex] = submesh;
}
this->set_Submeshes(copiedSubmeshes);
}

std::string RuntimeModel::get_Id()
{
return this->RuntimeData::get_Id();
}

void RuntimeModel::set_Id(std::string value)
{
this->RuntimeData::set_Id(value);
}

RuntimeModel::RuntimeModel() : BoundsMax(), BoundsMin(), Submeshes()
{
this->set_Submeshes(Array<RuntimeSubmesh*>::Empty());
}

