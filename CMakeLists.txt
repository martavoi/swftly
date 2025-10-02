cmake_minimum_required(VERSION 3.16)

# Version can be passed via -DPROJECT_VERSION=x.y.z (e.g., from Docker build)
# If not specified, defaults to 0.1.0 for local development
if(NOT DEFINED PROJECT_VERSION)
    set(PROJECT_VERSION "0.1.0")
endif()

project(swftly VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Set C++ standard with fallback support
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clang-tidy and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler detection and configuration
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Portable compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "")

# Base flags for all compilers
set(BASE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set(BASE_DEBUG_FLAGS "-g -O0 -DDEBUG")
set(BASE_RELEASE_FLAGS "-O3 -DNDEBUG")

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Configuring for Clang")
    
    # Clang-specific flags
    set(CLANG_FLAGS "${BASE_CXX_FLAGS} -Wthread-safety")
    set(CLANG_DEBUG_FLAGS "${BASE_DEBUG_FLAGS}")
    set(CLANG_RELEASE_FLAGS "${BASE_RELEASE_FLAGS} -flto=thin")
    
    # Optional: Use libc++ if explicitly requested
    option(USE_LIBCXX "Use libc++ instead of libstdc++" OFF)
    if(USE_LIBCXX)
        message(STATUS "Using libc++ standard library")
        set(CLANG_FLAGS "${CLANG_FLAGS} -stdlib=libc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
    else()
        message(STATUS "Using default standard library")
    endif()
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CLANG_DEBUG_FLAGS} ${CLANG_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CLANG_RELEASE_FLAGS} ${CLANG_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -gline-tables-only ${CLANG_FLAGS}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Oz -DNDEBUG -flto=thin ${CLANG_FLAGS}")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Configuring for GCC")
    
    # GCC-specific flags
    set(GCC_FLAGS "${BASE_CXX_FLAGS}")
    set(GCC_DEBUG_FLAGS "${BASE_DEBUG_FLAGS}")
    set(GCC_RELEASE_FLAGS "${BASE_RELEASE_FLAGS}")
    
    # GCC version-specific features
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13.0")
        message(STATUS "GCC 13+ detected, enabling advanced optimizations")
        set(GCC_RELEASE_FLAGS "${GCC_RELEASE_FLAGS} -flto")
    endif()
    
    set(CMAKE_CXX_FLAGS_DEBUG "${GCC_DEBUG_FLAGS} ${GCC_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${GCC_RELEASE_FLAGS} ${GCC_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${GCC_FLAGS}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG ${GCC_FLAGS}")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Configuring for MSVC")
    
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /RTC1 /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /DNDEBUG")

else()
    message(STATUS "Configuring for unknown compiler with basic flags")
    set(CMAKE_CXX_FLAGS_DEBUG "${BASE_DEBUG_FLAGS} ${BASE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${BASE_RELEASE_FLAGS} ${BASE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${BASE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG ${BASE_CXX_FLAGS}")
endif()

# Platform-specific settings
if(WIN32)
    message(STATUS "Configuring for Windows")
    # Windows-specific settings
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux")
    # Linux needs threading support for modern C++ and Boost
    find_package(Threads REQUIRED)
elseif(APPLE)
    message(STATUS "Configuring for macOS")
    # macOS-specific settings
endif()

# ==============================================================================
# Version Information
# ==============================================================================

# Get git hash if available
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

# Get build timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

message(STATUS "Version information:")
message(STATUS "  - Version: ${PROJECT_VERSION}")
message(STATUS "  - Git Hash: ${GIT_HASH}")
message(STATUS "  - Build Timestamp: ${BUILD_TIMESTAMP}")

# Automatically find all .cpp files in src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Find Boost libraries with better error handling
# Support both system packages and custom installations
find_package(Boost 1.75 REQUIRED COMPONENTS system log program_options json)

if(Boost_FOUND)
    message(STATUS "Found Boost ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library dir: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries:")
    message(STATUS "  - Boost::system: ${Boost_SYSTEM_LIBRARY}")
    message(STATUS "  - Boost::log: ${Boost_LOG_LIBRARY}")
    message(STATUS "  - Boost::program_options: ${Boost_PROGRAM_OPTIONS_LIBRARY}")
    message(STATUS "  - Boost::json: ${Boost_JSON_LIBRARY}")
else()
    message(FATAL_ERROR "Boost not found. Please install Boost development packages or set BOOST_ROOT.")
endif()

find_package(OpenSSL REQUIRED)

if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries:")
    message(STATUS "  - SSL library: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "  - Crypto library: ${OPENSSL_CRYPTO_LIBRARY}")
    if(OPENSSL_ROOT_DIR)
        message(STATUS "Using custom OpenSSL installation from: ${OPENSSL_ROOT_DIR}")
    else()
        message(STATUS "Using system OpenSSL installation")
    endif()
else()
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL development packages or set OPENSSL_ROOT_DIR.")
endif()

# Create the main executable (modular server application)
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories (for header files)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Define version information as compile definitions
target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
    SWFTLY_VERSION="${PROJECT_VERSION}"
    SWFTLY_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    SWFTLY_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    SWFTLY_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    SWFTLY_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    SWFTLY_GIT_HASH="${GIT_HASH}"
    SWFTLY_BUILD_TIMESTAMP="${BUILD_TIMESTAMP}"
)

# Link libraries using modern target-based approach
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Boost::system
    Boost::log
    Boost::program_options
    Boost::json
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 wsock32)
    message(STATUS "Linked Windows socket libraries")
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
    message(STATUS "Linked threading libraries for Linux")
endif()

# Compiler feature requirements
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Print configuration summary
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")

# Standard library information
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(USE_LIBCXX)
        message(STATUS "Standard library: libc++")
    else()
        message(STATUS "Standard library: libstdc++ (default)")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Standard library: libstdc++")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Standard library: MSVCRT")
endif()

# Compiler flags summary
message(STATUS "Compiler flags for ${CMAKE_BUILD_TYPE}:")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "  ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "  ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    message(STATUS "  ${CMAKE_CXX_FLAGS_MINSIZEREL}")
endif()

message(STATUS "Found source files: ${SOURCES}")
message(STATUS "Executable will be placed in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# Library dependencies summary
message(STATUS "=== Library Dependencies Summary ===")
message(STATUS "Boost ${Boost_VERSION} (${Boost_LIBRARY_DIRS})")
message(STATUS "  └─ Components: system, log, program_options, json")
if(OPENSSL_ROOT_DIR)
    message(STATUS "OpenSSL ${OPENSSL_VERSION} (Custom: ${OPENSSL_ROOT_DIR})")
else()
    message(STATUS "OpenSSL ${OPENSSL_VERSION} (System)")
endif()
message(STATUS "  └─ Components: SSL, Crypto")

# Platform-specific libraries
if(WIN32)
    message(STATUS "Windows Socket Libraries: ws2_32, wsock32")
elseif(UNIX AND NOT APPLE)
    message(STATUS "Threading Library: Threads::Threads")
endif()