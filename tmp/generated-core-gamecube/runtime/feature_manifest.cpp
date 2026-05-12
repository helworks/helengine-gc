#include "feature_manifest.hpp"

static const HEFeatureEntry kFeatureEntries[] = {
    { HEFeature::DebugOverlay, false, HEFeatureDecisionOrigin::ForcedDisabled, "DebugOverlay" },
    { HEFeature::HostFileSystem, false, HEFeatureDecisionOrigin::NotIncluded, "HostFileSystem" },
    { HEFeature::ReflectionLikeRuntime, false, HEFeatureDecisionOrigin::NotIncluded, "ReflectionLikeRuntime" },
    { HEFeature::Render2D, true, HEFeatureDecisionOrigin::AutoDetected, "Render2D" },
    { HEFeature::RuntimeJson, false, HEFeatureDecisionOrigin::NotIncluded, "RuntimeJson" },
    { HEFeature::Shaders, true, HEFeatureDecisionOrigin::AutoDetected, "Shaders" },
    { HEFeature::Sprites, true, HEFeatureDecisionOrigin::AutoDetected, "Sprites" },
    { HEFeature::Text2D, true, HEFeatureDecisionOrigin::AutoDetected, "Text2D" },
    { HEFeature::TextProcessing, false, HEFeatureDecisionOrigin::NotIncluded, "TextProcessing" },
};

bool he_feature_enabled(HEFeature feature) {
    for (const HEFeatureEntry& entry : kFeatureEntries) {
        if (entry.Feature == feature) {
            return entry.Enabled;
        }
    }

    return false;
}

const HEFeatureEntry* he_get_feature_entries(std::size_t* count) {
    if (count != nullptr) {
        *count = sizeof(kFeatureEntries) / sizeof(kFeatureEntries[0]);
    }

    return kFeatureEntries;
}
