#ifdef DrawText
#undef DrawText
#endif
#include "CameraProjectionUtils.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4x4.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"

float CameraProjectionUtils::MinimumNearPlaneDistance = 0.01f;

float CameraProjectionUtils::MinimumPlaneSeparation = 0.01f;

float CameraProjectionUtils::ClampFarPlaneDistance(float nearPlaneDistance, float farPlaneDistance)
{
const float minimumNearPlaneDistance = Math::Max(MinimumNearPlaneDistance, nearPlaneDistance);
return Math::Max(minimumNearPlaneDistance + MinimumPlaneSeparation, farPlaneDistance);}

float CameraProjectionUtils::ClampNearPlaneDistance(float nearPlaneDistance, float farPlaneDistance)
{
const float minimumFarPlaneDistance = ClampFarPlaneDistance(MinimumNearPlaneDistance, farPlaneDistance);
return Math::Min(Math::Max(MinimumNearPlaneDistance, nearPlaneDistance), minimumFarPlaneDistance - MinimumPlaneSeparation);}

::float4x4 CameraProjectionUtils::CreatePerspectiveProjection(::ICamera* camera, float fieldOfView, float aspectRatio)
{
    if (camera == nullptr)
    {
throw new ArgumentNullException("camera");
    }
const float nearPlaneDistance = ClampNearPlaneDistance(camera->get_NearPlaneDistance(), camera->get_FarPlaneDistance());
const float farPlaneDistance = ClampFarPlaneDistance(nearPlaneDistance, camera->get_FarPlaneDistance());
::float4x4 projection;
float4x4::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance, projection);
return projection;}

