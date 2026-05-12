#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameDrawableClassifier;
class RenderFrameLightClassifier;
class RenderFrameExtractionResult;
class CameraComponent;
class IDrawable3D;
class LightComponent;
class RendererBackendCapabilityProfile;

#include "runtime/native_exceptions.hpp"
#include "RenderFrameDrawableClassifier.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameLightClassifier.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "runtime/native_list.hpp"
#include "CameraComponent.hpp"
#include "runtime/native_list.hpp"
#include "IDrawable3D.hpp"
#include "runtime/native_list.hpp"
#include "LightComponent.hpp"
#include "RendererBackendCapabilityProfile.hpp"

class RenderFrameExtractionService
{
public:
    virtual ~RenderFrameExtractionService() = default;

    ::RenderFrameExtractionResult* Extract(List<::CameraComponent*>* cameras, List<::IDrawable3D*>* drawables, List<::LightComponent*>* lights, ::RendererBackendCapabilityProfile* backendCapabilities);
};
