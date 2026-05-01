#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

#include "helcpp_config.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#endif

/// Resolves executable-relative application context values used by generated runtime initialization.
class AppContext {
public:
    inline static constexpr const char* BaseDirectory = ".";
};

#endif // APP_CONTEXT_HPP
