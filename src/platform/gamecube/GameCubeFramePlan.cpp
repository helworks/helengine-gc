#include "platform/gamecube/GameCubeFramePlan.hpp"

#include <unordered_set>

#include "IDrawable3D.hpp"
#include "LightComponent.hpp"
#include "RenderFrame.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"

namespace {
    /// Releases the extracted render-frame graph owned by one GameCube frame plan. The extraction
    /// result disposes its frames and their submissions itself; batching metadata is the one graph
    /// node the generated ownership model leaves to the caller, so it is deleted here first.
    void DeleteExtractionResult(RenderFrameExtractionResult*& extractionResult) {
        if (extractionResult == nullptr) {
            return;
        }

        std::unordered_set<RenderFrameBatchingMetadata*> deletedBatchingMetadata;
        IReadOnlyList<RenderFrame*>* frames = extractionResult->get_Frames();
        if (frames != nullptr) {
            for (int32_t frameIndex = 0; frameIndex < frames->get_Count(); frameIndex++) {
                RenderFrame* frame = frames->get_Item(frameIndex);
                if (frame == nullptr) {
                    continue;
                }

                IReadOnlyList<RenderFrameDrawableSubmission*>* drawableSubmissions = frame->get_DrawableSubmissions();
                if (drawableSubmissions == nullptr) {
                    continue;
                }

                for (int32_t submissionIndex = 0; submissionIndex < drawableSubmissions->get_Count(); submissionIndex++) {
                    RenderFrameDrawableSubmission* drawableSubmission = drawableSubmissions->get_Item(submissionIndex);
                    if (drawableSubmission == nullptr) {
                        continue;
                    }

                    RenderFrameBatchingMetadata* batchingMetadata = drawableSubmission->get_BatchingMetadata();
                    if (batchingMetadata != nullptr && deletedBatchingMetadata.insert(batchingMetadata).second) {
                        delete batchingMetadata;
                    }
                }
            }
        }

        extractionResult->Dispose();
        delete extractionResult;
        extractionResult = nullptr;
    }
}

namespace helengine::gamecube {
    /// Releases the temporary scene snapshots used to build this frame plan.
    GameCubeFramePlan::~GameCubeFramePlan() {
        if (Cameras != nullptr) {
            delete Cameras;
            Cameras = nullptr;
        }

        if (Drawables != nullptr) {
            delete Drawables;
            Drawables = nullptr;
        }

        if (Lights != nullptr) {
            delete Lights;
            Lights = nullptr;
        }

        DeleteExtractionResult(ExtractionResult);
        DrawableSubmissions = nullptr;
        LightSubmissions = nullptr;
    }
}
