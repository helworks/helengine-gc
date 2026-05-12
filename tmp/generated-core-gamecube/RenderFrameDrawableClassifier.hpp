#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable3D;
class RuntimeMaterial;
class MaterialRenderState;
class RenderFrameDrawableSubmission;
class RuntimeSubmesh;
class RuntimeModel;

#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"
#include "MaterialRenderState.hpp"
#include "runtime/array.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"
#include "RuntimeModel.hpp"

class RenderFrameDrawableClassifier
{
public:
    virtual ~RenderFrameDrawableClassifier() = default;

    Array<::RenderFrameDrawableSubmission*>* Classify(::IDrawable3D* drawable);
private:
    static bool IsTransparent(::RuntimeMaterial* material);

    static ::RuntimeMaterial* ResolveMaterial(::IDrawable3D* drawable, int32_t submeshIndex);

    static Array<::RuntimeSubmesh*>* ResolveSubmeshes(::RuntimeModel* model);
};
