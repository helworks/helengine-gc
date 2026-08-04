#include <cstdarg>

/// Absorbs one libogc system report when the GameCube build disables EXI-backed reporting.
extern "C" void __wrap_SYS_Report(const char* format, ...) {
    static_cast<void>(format);
}
