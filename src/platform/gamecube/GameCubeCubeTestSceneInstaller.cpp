#include "platform/gamecube/GameCubeCubeTestSceneInstaller.hpp"

#include "CameraClearSettings.hpp"
#include "CameraComponent.hpp"
#include "DirectionalLightComponent.hpp"
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "Entity.hpp"
#include "MeshComponent.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeSubmesh.hpp"
#include "ShadowMapMode.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "platform/gamecube/GameCubeRuntimeModel.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::gamecube {
    namespace {
        constexpr uint16_t SceneObjectsLayerMask = 0b0100000000000000;
        constexpr float CameraNearPlaneDistance = 0.1f;
        constexpr float CameraFarPlaneDistance = 64.0f;
        constexpr float DirectionalLightYawRadians = -0.65f;
        constexpr float DirectionalLightPitchRadians = -0.85f;
        constexpr float CubeAngularSpeedRadians = 0.0f;
        constexpr float CubeHalfExtent = 0.5f;
    }

    /// Creates camera, light, and mesh runtime entities that mirror the city cube-test scene.
    void GameCubeCubeTestSceneInstaller::Install() {
        CreateCameraEntity();
        CreateDirectionalLightEntity();
        CreateCubeEntity();
    }

    /// Creates the runtime camera entity used by the first GameCube cube-test milestone.
    Entity* GameCubeCubeTestSceneInstaller::CreateCameraEntity() {
        Entity* cameraEntity = new Entity();
        cameraEntity->InitComponents();
        cameraEntity->InitChildren();
        cameraEntity->set_LocalPosition(float3(0.0f, 0.0f, 6.0f));

        CameraComponent* camera = new CameraComponent();
        camera->set_CameraDrawOrder(0);
        camera->set_LayerMask(SceneObjectsLayerMask);
        camera->set_Viewport(float4(0.0f, 0.0f, 1.0f, 1.0f));
        camera->set_NearPlaneDistance(CameraNearPlaneDistance);
        camera->set_FarPlaneDistance(CameraFarPlaneDistance);
        camera->set_ClearSettings(CameraClearSettings(
            true,
            float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f),
            true,
            1.0f,
            false,
            0));
        cameraEntity->AddComponent(camera);
        return cameraEntity;
    }

    /// Creates the runtime directional light entity that mirrors the authored cube-test sun.
    Entity* GameCubeCubeTestSceneInstaller::CreateDirectionalLightEntity() {
        Entity* lightEntity = new Entity();
        lightEntity->InitComponents();
        lightEntity->InitChildren();
        lightEntity->set_LocalPosition(float3(0.0f, 4.0f, 0.0f));
        lightEntity->set_LayerMask(SceneObjectsLayerMask);

        float4 lightOrientation;
        float4::CreateFromYawPitchRoll(DirectionalLightYawRadians, DirectionalLightPitchRadians, 0.0f, lightOrientation);
        lightOrientation.Normalize();
        lightEntity->set_LocalOrientation(lightOrientation);

        DirectionalLightComponent* light = new DirectionalLightComponent();
        light->set_Color(float4(1.0f, 1.0f, 1.0f, 1.0f));
        light->set_Intensity(1.0f);
        light->set_ShadowsEnabled(false);
        light->set_ShadowMapMode(ShadowMapMode::Forced);
        light->set_ShadowStrength(1.0f);
        light->set_ShadowDistance(24.0f);
        lightEntity->AddComponent(light);
        return lightEntity;
    }

    /// Creates the rotating cube mesh entity using an in-memory runtime model and material.
    Entity* GameCubeCubeTestSceneInstaller::CreateCubeEntity() {
        Entity* cubeEntity = new Entity();
        cubeEntity->InitComponents();
        cubeEntity->InitChildren();
        cubeEntity->set_LayerMask(SceneObjectsLayerMask);
        cubeEntity->set_LocalScale(float3(2.0f, 2.0f, 2.0f));

        GameCubeRuntimeModel* runtimeModel = new GameCubeRuntimeModel();
        runtimeModel->set_Id("gamecube:runtime:cube");
        runtimeModel->SetBounds(
            float3(-CubeHalfExtent, -CubeHalfExtent, -CubeHalfExtent),
            float3(CubeHalfExtent, CubeHalfExtent, CubeHalfExtent));
        runtimeModel->Positions = new Array<float3>({
            float3(-CubeHalfExtent, -CubeHalfExtent, -CubeHalfExtent),
            float3(CubeHalfExtent, -CubeHalfExtent, -CubeHalfExtent),
            float3(CubeHalfExtent, CubeHalfExtent, -CubeHalfExtent),
            float3(-CubeHalfExtent, CubeHalfExtent, -CubeHalfExtent),
            float3(-CubeHalfExtent, -CubeHalfExtent, CubeHalfExtent),
            float3(CubeHalfExtent, -CubeHalfExtent, CubeHalfExtent),
            float3(CubeHalfExtent, CubeHalfExtent, CubeHalfExtent),
            float3(-CubeHalfExtent, CubeHalfExtent, CubeHalfExtent)
        });
        runtimeModel->Indices16 = new Array<uint16_t>({
            4, 5, 6, 4, 6, 7,
            0, 2, 1, 0, 3, 2,
            0, 4, 7, 0, 7, 3,
            1, 2, 6, 1, 6, 5,
            3, 7, 6, 3, 6, 2,
            0, 1, 5, 0, 5, 4
        });
        runtimeModel->Indices32 = nullptr;
        runtimeModel->Uses32BitIndices = false;
        RuntimeSubmesh* submesh = new RuntimeSubmesh();
        submesh->set_MaterialSlotName("");
        submesh->set_IndexStart(0);
        submesh->set_IndexCount(runtimeModel->Indices16->Length);
        runtimeModel->SetSubmeshes(new Array<RuntimeSubmesh*>({ submesh }));

        RuntimeMaterial* runtimeMaterial = new RuntimeMaterial();
        runtimeMaterial->set_Id("gamecube:runtime:white-unlit");
        runtimeMaterial->set_CastsShadows(false);
        runtimeMaterial->set_ReceivesShadows(false);

        MeshComponent* mesh = new MeshComponent();
        mesh->set_Model(runtimeModel);
        mesh->set_Material(runtimeMaterial);
        mesh->set_RenderOrder3D(0);
        cubeEntity->AddComponent(mesh);

        DirectionalShadowTowerSpinComponent* spin = new DirectionalShadowTowerSpinComponent();
        spin->set_BaseYawRadians(0.0f);
        spin->set_AngularSpeedRadians(CubeAngularSpeedRadians);
        cubeEntity->AddComponent(spin);
        return cubeEntity;
    }
}
