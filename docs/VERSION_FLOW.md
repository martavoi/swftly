# Version Injection Flow

## Overview

Version information flows from semantic-release through Docker build arguments to CMake, and finally gets compiled into the application binary.

## Complete Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│ GitHub Actions Workflow (.github/workflows/release.yml)            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Job 1: build (runs on every push/PR)                             │
│  ├─ Build Docker image for testing                                 │
│  └─ Uses VERSION=0.0.0-dev (default)                              │
│                                                                     │
│  Job 2: release (runs only on main branch, not PRs)               │
│  ├─ Checkout code                                                  │
│  ├─ Install Node.js & dependencies                                 │
│  └─ Run: npx semantic-release                                      │
│      │                                                              │
│      └─> Triggers semantic-release...                              │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│ semantic-release (.releaserc.json)                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. @semantic-release/commit-analyzer                              │
│     └─> Analyzes commits → Determines version (e.g., 1.2.3)       │
│                                                                     │
│  2. @semantic-release/release-notes-generator                      │
│     └─> Generates release notes                                    │
│                                                                     │
│  3. @semantic-release/changelog                                    │
│     └─> Creates CHANGELOG.md                                       │
│                                                                     │
│  4. @semantic-release/exec                                         │
│     └─> Runs: docker buildx build                                  │
│                --build-arg VERSION=${nextRelease.version}          │
│                --push                                               │
│                -t ghcr.io/org/repo:1.2.3                          │
│                -t ghcr.io/org/repo:latest .                        │
│         │                                                           │
│         └─> nextRelease.version = "1.2.3"                         │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│ Docker Build (Dockerfile)                                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ARG VERSION=0.0.0-dev                                             │
│  │                                                                  │
│  ├─ VERSION is passed from docker build command                    │
│  │                                                                  │
│  RUN cmake -B build \                                              │
│      -DPROJECT_VERSION=${VERSION} \                                │
│      ...                                                            │
│      │                                                              │
│      └─> Passes VERSION to CMake                                   │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│ CMake Configuration (CMakeLists.txt)                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  if(NOT DEFINED PROJECT_VERSION)                                   │
│      set(PROJECT_VERSION "0.0.0-dev")  # Fallback                 │
│  endif()                                                            │
│                                                                     │
│  project(swftly VERSION ${PROJECT_VERSION} LANGUAGES CXX)          │
│  │                                                                  │
│  ├─> CMake parses version: 1.2.3                                  │
│  │   - PROJECT_VERSION = "1.2.3"                                  │
│  │   - PROJECT_VERSION_MAJOR = 1                                  │
│  │   - PROJECT_VERSION_MINOR = 2                                  │
│  │   - PROJECT_VERSION_PATCH = 3                                  │
│  │                                                                  │
│  └─> target_compile_definitions(swftly PRIVATE                     │
│          SWFTLY_VERSION="1.2.3"                                    │
│          SWFTLY_VERSION_MAJOR=1                                    │
│          SWFTLY_VERSION_MINOR=2                                    │
│          SWFTLY_VERSION_PATCH=3                                    │
│          SWFTLY_BUILD_TYPE="Release"                               │
│          SWFTLY_GIT_HASH="a1b2c3d"                                 │
│      )                                                              │
│      │                                                              │
│      └─> Passes as compiler flags: -DSWFTLY_VERSION="1.2.3" ...   │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│ Compiler (g++/clang++)                                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Compiles src/version.h with definitions:                          │
│                                                                     │
│  #define SWFTLY_VERSION "1.2.3"                                    │
│  #define SWFTLY_VERSION_MAJOR 1                                    │
│  #define SWFTLY_VERSION_MINOR 2                                    │
│  #define SWFTLY_VERSION_PATCH 3                                    │
│                                                                     │
│  namespace swftly {                                                │
│      inline constexpr std::string_view VERSION = "1.2.3";         │
│      inline constexpr int VERSION_MAJOR = 1;                       │
│      ...                                                            │
│  }                                                                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│ Application Code                                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  #include "version.h"                                              │
│                                                                     │
│  // In main.cpp:                                                   │
│  BOOST_LOG(...) << "Starting Swftly v" << swftly::VERSION;        │
│                                                                     │
│  // In root_handler.cpp:                                           │
│  body["version"] = swftly::VERSION;                                │
│  body["git_hash"] = swftly::GIT_HASH;                             │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    Compiled into binary
                              │
                              ▼
                    Runtime: v1.2.3 available!
```

## Key Points

1. **No version file in repository**: Version is computed by semantic-release and passed through build pipeline
2. **Build argument**: Docker receives `VERSION` as build arg
3. **CMake variable**: Dockerfile passes `VERSION` to CMake via `-DPROJECT_VERSION`
4. **Preprocessor definitions**: CMake passes version as compiler flags (like `-DSWFTLY_VERSION="1.2.3"`)
5. **Native C++ approach**: Similar to Go's `-ldflags`, uses standard preprocessor macros
6. **Compile-time constants**: Version is compiled into binary as `constexpr` constants
7. **Zero runtime overhead**: All version info is known at compile time
8. **No generated files**: Simple `version.h` with macro defaults, IDE-friendly

## Environment Variables Used

| Variable | Set By | Used In | Purpose |
|----------|--------|---------|---------|
| `GITHUB_TOKEN` | GitHub Actions | semantic-release | Authentication for GitHub API |
| `GITHUB_REPOSITORY` | GitHub Actions | semantic-release exec | Repository name for Docker tags |
| `REGISTRY` | GitHub Actions | semantic-release exec | Container registry URL |

## Local Development

Without semantic-release, the version defaults to `0.0.0-dev`:

```bash
# Default version
docker build -t swftly:dev .

# Custom version
docker build --build-arg VERSION=1.2.3 -t swftly:1.2.3 .

# Direct CMake
cmake -B build -DPROJECT_VERSION=1.2.3
```

## Testing the Flow

To test that version injection works:

```bash
# Build with custom version
docker build --build-arg VERSION=9.9.9 -t swftly:test .

# Run and check version
docker run --rm swftly:test --help
curl http://localhost:8080/  # Should show version: "9.9.9"
```

