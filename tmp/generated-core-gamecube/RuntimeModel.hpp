#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeData;
class float3;
class RuntimeSubmesh;

#include "RuntimeData.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"

class RuntimeModel : public RuntimeData
{
public:
    virtual ~RuntimeModel() = default;

    ::float3 BoundsMax;

    ::float3 get_BoundsMax();
    void set_BoundsMax(::float3 value);

    ::float3 BoundsMin;

    ::float3 get_BoundsMin();
    void set_BoundsMin(::float3 value);

    Array<::RuntimeSubmesh*>* Submeshes;

    Array<::RuntimeSubmesh*>* get_Submeshes();
    void set_Submeshes(Array<::RuntimeSubmesh*>* value);

    void SetBounds(::float3 boundsMin, ::float3 boundsMax);

    void SetSubmeshes(Array<::RuntimeSubmesh*>* submeshes);

    std::string get_Id();

    void set_Id(std::string value);
protected:
    RuntimeModel();
};
