#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeData;
class MaterialPropertyBlock;
class RuntimeTexture;
class MaterialLayout;
class MaterialRenderState;
class MaterialConstantBufferAsset;

#include "RuntimeData.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "MaterialPropertyBlock.hpp"
#include "RuntimeTexture.hpp"
#include "runtime/native_string.hpp"
#include "MaterialLayout.hpp"
#include "runtime/array.hpp"
#include "MaterialLayout.hpp"
#include "MaterialPropertyBlock.hpp"
#include "runtime/native_list.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "MaterialRenderState.hpp"
#include "runtime/native_list.hpp"
#include "runtime/array.hpp"
#include "MaterialConstantBufferAsset.hpp"
#include "RuntimeTexture.hpp"
#include "runtime/array.hpp"

class RuntimeMaterial : public RuntimeData
{
public:
    virtual ~RuntimeMaterial() = default;

    bool CastsShadows;

    bool get_CastsShadows();
    void set_CastsShadows(bool value);

    ::MaterialLayout* Layout;

    ::MaterialLayout* get_Layout();
    void set_Layout(::MaterialLayout* value);

    ::RuntimeMaterialLightingModel LightingModel;

    ::RuntimeMaterialLightingModel get_LightingModel();
    void set_LightingModel(::RuntimeMaterialLightingModel value);

    ::RuntimeMaterial* get_ParentMaterial();

    ::MaterialPropertyBlock* Properties;

    ::MaterialPropertyBlock* get_Properties();
    void set_Properties(::MaterialPropertyBlock* value);

    bool ReceivesShadows;

    bool get_ReceivesShadows();
    void set_ReceivesShadows(bool value);

    ::MaterialRenderState* RenderState;

    ::MaterialRenderState* get_RenderState();
    void set_RenderState(::MaterialRenderState* value);

    bool SupportsEmissive;

    bool get_SupportsEmissive();
    void set_SupportsEmissive(bool value);

    bool SupportsNormalMapping;

    bool get_SupportsNormalMapping();
    void set_SupportsNormalMapping(bool value);

    void ApplyConstantBufferDefaults(Array<::MaterialConstantBufferAsset*>* constantBuffers);

    ::RuntimeMaterial* ResolveRootMaterial();

    ::RuntimeTexture* ResolveTexture();

    RuntimeMaterial();

    void SetLayout(::MaterialLayout* layout);

    void SetParentMaterial(::RuntimeMaterial* parentMaterial);

    void SetRenderState(::MaterialRenderState* renderState);

    bool TryResolveConstantBufferData(std::string bindingName, Array<uint8_t>*& data);

    std::string get_Id();

    void set_Id(std::string value);
private:
    List<::RuntimeMaterial*>* ChildMaterialsValue;

    ::RuntimeMaterial* ParentMaterialValue;

    void ApplyResolvedLayout(::MaterialLayout* layout);

    void RegisterChildMaterial(::RuntimeMaterial* childMaterial);

    void RestoreConstantBufferBindings(::MaterialLayout* previousLayout, ::MaterialPropertyBlock* previousProperties);

    void RestoreTextureBindings(::MaterialLayout* previousLayout, ::MaterialPropertyBlock* previousProperties);

    void SynchronizeChildMaterials();

    void SynchronizeWithParentMaterial();

    void UnregisterChildMaterial(::RuntimeMaterial* childMaterial);

    void ValidateParentMaterial(::RuntimeMaterial* parentMaterial);
};
