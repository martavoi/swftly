cmake_minimum_required(VERSION 3.16)

# Project name and version
project(swftly VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
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

# Clang-specific configuration
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Use libc++ with Clang for better C++23 support and modern features
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
    
    # Clang-optimized compiler flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto=thin")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -gline-tables-only")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Oz -DNDEBUG -flto=thin")
else()
    # Fallback flags for other compilers (GCC, etc.)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
endif()

# Platform-specific settings
if(WIN32)
    message(STATUS "Configuring for Windows")
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux")
    # Linux needs threading support for modern C++ and Boost
    find_package(Threads REQUIRED)
endif()

# Automatically find all .cpp files in src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Find Boost libraries using modern target-based approach
# Only include components actually used in the code
# Note: Boost.Redis is header-only, so we don't need to find it as a component
find_package(Boost REQUIRED COMPONENTS system log program_options json)

# Find our custom-built OpenSSL, which Boost.Redis requires
set(OPENSSL_ROOT_DIR "/usr/local/openssl-3.5.1")
find_package(OpenSSL REQUIRED)

# Create the main executable (modular server application)
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories (for header files)
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Link libraries
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

# Optional: Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using libc++ standard library")
endif()
message(STATUS "Found source files: ${SOURCES}")
message(STATUS "Executable will be placed in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")