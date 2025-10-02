#pragma once

#include <string_view>

namespace swftly
{

/**
 * @brief Version information for the Swftly application.
 *
 * These macros are defined at compile time via CMake:
 * - SWFTLY_VERSION: Full version string (e.g., "1.2.3")
 * - SWFTLY_VERSION_MAJOR: Major version number
 * - SWFTLY_VERSION_MINOR: Minor version number
 * - SWFTLY_VERSION_PATCH: Patch version number
 * - SWFTLY_BUILD_TYPE: Build type (Debug/Release)
 * - SWFTLY_GIT_HASH: Git commit hash
 * - SWFTLY_BUILD_TIMESTAMP: Build timestamp
 */

// Stringify helper macros
#define SWFTLY_STRINGIFY_IMPL(x) #x
#define SWFTLY_STRINGIFY(x) SWFTLY_STRINGIFY_IMPL(x)

// Provide defaults if not defined by CMake
#ifndef SWFTLY_VERSION
#define SWFTLY_VERSION "0.1.0"
#endif

#ifndef SWFTLY_VERSION_MAJOR
#define SWFTLY_VERSION_MAJOR 0
#endif

#ifndef SWFTLY_VERSION_MINOR
#define SWFTLY_VERSION_MINOR 0
#endif

#ifndef SWFTLY_VERSION_PATCH
#define SWFTLY_VERSION_PATCH 0
#endif

#ifndef SWFTLY_BUILD_TYPE
#define SWFTLY_BUILD_TYPE "Unknown"
#endif

#ifndef SWFTLY_GIT_HASH
#define SWFTLY_GIT_HASH "unknown"
#endif

#ifndef SWFTLY_BUILD_TIMESTAMP
#define SWFTLY_BUILD_TIMESTAMP "unknown"
#endif

// Expose as C++ constants
inline constexpr std::string_view VERSION = SWFTLY_VERSION;
inline constexpr int VERSION_MAJOR = SWFTLY_VERSION_MAJOR;
inline constexpr int VERSION_MINOR = SWFTLY_VERSION_MINOR;
inline constexpr int VERSION_PATCH = SWFTLY_VERSION_PATCH;
inline constexpr std::string_view BUILD_TYPE = SWFTLY_BUILD_TYPE;
inline constexpr std::string_view GIT_HASH = SWFTLY_GIT_HASH;
inline constexpr std::string_view BUILD_TIMESTAMP = SWFTLY_BUILD_TIMESTAMP;

} // namespace swftly
