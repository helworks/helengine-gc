#pragma once

#include <cstddef>

namespace helengine::gamecube {
    /// Performs synchronous byte-range reads through an owned GameCube DI completion bridge that compatible loaders can redirect alongside retail hardware support.
    class GameCubeDiscReader {
    public:
        /// Installs the packaged-disc DI completion handler without mounting or resetting the drive.
        static bool Initialize();

        /// Reads an arbitrary byte range from the physical GameCube disc into a 32-byte-aligned destination buffer.
        static bool ReadBytes(void* destination, std::size_t offset, std::size_t length);
    };
}
