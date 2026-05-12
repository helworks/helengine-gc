#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4x4;
class ICamera;

#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4x4.hpp"
#include "float4x4.hpp"
#include "ICamera.hpp"

class CameraProjectionUtils
{
public:
    virtual ~CameraProjectionUtils() = default;

    static float MinimumNearPlaneDistance;

    static float MinimumPlaneSeparation;

    static float ClampFarPlaneDistance(float nearPlaneDistance, float farPlaneDistance);

    static float ClampNearPlaneDistance(float nearPlaneDistance, float farPlaneDistance);

    static ::float4x4 CreatePerspectiveProjection(::ICamera* camera, float fieldOfView, float aspectRatio);
};
