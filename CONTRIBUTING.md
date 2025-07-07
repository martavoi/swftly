# Contributing to Swftly

Thank you for your interest in contributing to Swftly! This document provides detailed setup instructions, development guidelines, and troubleshooting tips.

## Development Environment Setup

### Prerequisites

#### Required Tools
- **CMake** 3.16 or higher
- **Clang** 20+ with C++23 support (or GCC 13+)
- **Boost Libraries** 1.85+ (system, log, program_options, json components)
- **OpenSSL** 3.0+ (for Redis connections)
- **Redis Server** 6+ (for runtime)
- **Git** (for cloning)

### Platform Setup

#### Linux/WSL (Recommended)

##### 1. Install Build Tools
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git redis-server

# Install Clang 20 (latest)
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
```

##### 2. Build Boost Libraries with libc++

**Important**: We compile Boost with Clang and libc++ for optimal C++23 support.

```bash
# Download Boost 1.88.0
cd ~/
wget https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz
tar xzf boost_1_88_0.tar.gz
cd boost_1_88_0

# Bootstrap the build system
./bootstrap.sh --with-toolset=clang

# Build and install with libc++ (for modern C++23 support)
sudo ./b2 clean
sudo ./b2 -j8 toolset=clang variant=release,debug \
    cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++" \
    threading=multi link=shared,static install

# Verify installation
ls -la /usr/local/lib/libboost_* | head -10
```

##### 3. Build OpenSSL with libc++

```bash
# Download OpenSSL 3.5.1
cd ~/
wget https://github.com/openssl/openssl/releases/download/openssl-3.5.1/openssl-3.5.1.tar.gz
tar xzf openssl-3.5.1.tar.gz
cd openssl-3.5.1

# Configure and build with Clang/libc++
export CC=clang-20
export CXX=clang++-20
export CXXFLAGS="-stdlib=libc++"
export LDFLAGS="-stdlib=libc++"

./Configure --prefix=/usr/local/openssl-3.5.1 --openssldir=/usr/local/openssl-3.5.1 \
    linux-x86_64 shared zlib

make -j8
sudo make install

# Verify installation
/usr/local/openssl-3.5.1/bin/openssl version
```

##### 4. Verify Dependencies
```bash
# Check Boost version and libraries
ls -la /usr/local/lib/libboost_* | grep -E "(log|system|program_options|json)"

# Check library dependencies (should show libc++ for our build)
ldd /usr/local/lib/libboost_log.so.1.88.0

# Check OpenSSL
ls -la /usr/local/openssl-3.5.1/lib/
```

#### Windows Setup

##### Option 1: WSL2 (Recommended)
Use Windows Subsystem for Linux with the Linux setup above.

##### Option 2: Native Windows with MSYS2
```bash
# Install MSYS2
winget install MSYS2.MSYS2

# Open MSYS2 terminal and install tools
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-boost mingw-w64-x86_64-openssl

# Note: System packages may not have C++23 support
# For full C++23 features, build Boost/OpenSSL from source
```

### Building the Project

#### Multi-Configuration Support

This project supports separate Debug and Release builds in different directories:

#### Quick Build Commands

**Debug Build:**
```bash
# Linux/WSL (Clang + libc++)
cmake -G "Unix Makefiles" -S . -B build-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_C_COMPILER=clang-20
cmake --build build-debug

# Windows (MSYS2)
cmake -G "MinGW Makefiles" -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

**Release Build:**
```bash
# Linux/WSL (Clang + libc++)
cmake -G "Unix Makefiles" -S . -B build-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++-20 \
    -DCMAKE_C_COMPILER=clang-20
cmake --build build-release

# Windows (MSYS2)
cmake -G "MinGW Makefiles" -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

#### Build Configurations

- **Debug**: `-g -Wall -Wextra -Wpedantic -O0 -DDEBUG`
  - Full debug symbols, all warnings, no optimization
- **Release**: `-O3 -DNDEBUG -march=native -flto=thin`
  - Maximum optimization, no debug symbols, Link Time Optimization
- **RelWithDebInfo**: `-O2 -g -DNDEBUG -gline-tables-only`
  - Optimized with debug symbols
- **MinSizeRel**: `-Oz -DNDEBUG -flto=thin`
  - Size-optimized release

#### Clean Build
```bash
# Clean specific configuration
cmake --build build-debug --target clean
cmake --build build-release --target clean

# Remove all build directories (cross-platform)
cmake -E rm -rf build-debug build-release
```

### Running the Application

#### Start Dependencies
```bash
# Start Redis server
redis-server

# Or with custom config
redis-server /path/to/redis.conf
```

#### Run Swftly
```bash
# Debug build
./build-debug/bin/swftly --port 8080 --redis-host 127.0.0.1 --redis-port 6379

# Release build  
./build-release/bin/swftly --port 8080 --redis-host 127.0.0.1 --redis-port 6379
```

#### Test the API
```bash
# Health check
curl http://localhost:8080/ping

# Create short URL
curl -X POST http://localhost:8080/api/urls \
  -H "Content-Type: application/json" \
  -d '{"url": "https://www.example.com"}'

# Use short URL
curl -L http://localhost:8080/a
```

## Development Workflow

### VS Code Setup

The project includes pre-configured VS Code tasks:

- **Build Debug**: `Ctrl+Shift+B` (default task)
- **Build Release**: `Ctrl+Shift+P` → "Tasks: Run Task" → "cmake-build-release"
- **Debug (Debug Build)**: `F5` → Select "Debug (Debug Build)"
- **Debug (Release Build)**: `F5` → Select "Debug (Release Build)"
- **Clean All**: `Ctrl+Shift+P` → "Tasks: Run Task" → "cmake-clean-all"

#### Available Tasks:
- `cmake-configure-debug` - Configure Debug build
- `cmake-configure-release` - Configure Release build  
- `cmake-build-debug` - Build Debug configuration
- `cmake-build-release` - Build Release configuration
- `cmake-clean-debug` - Clean Debug artifacts
- `cmake-clean-release` - Clean Release artifacts
- `cmake-clean-all` - Remove all build directories

### Project Structure
```
swftly/
├── src/                    # Source files (.cpp, .hpp)
│   ├── conf/              # Configuration management
│   ├── encode/            # Base62 encoding/decoding
│   ├── http/              # HTTP server implementation
│   │   ├── handlers/      # Request handlers
│   │   ├── server.cpp     # Boost.Beast async server with C++20 coroutines
│   │   └── router.cpp     # Request routing with transparent hashing
│   ├── logging/           # Boost.Log setup and configuration
│   ├── storage/           # Redis storage service
│   └── main.cpp           # Application entry point
├── build-debug/           # Debug build output (generated)
│   ├── bin/              # Debug executables
│   │   └── swftly        # Debug build of URL shortener
│   ├── lib/              # Debug libraries
│   └── compile_commands.json  # For clang-tidy and IDEs
├── build-release/         # Release build output (generated)  
│   ├── bin/              # Release executables
│   │   └── swftly        # Optimized release build
│   └── lib/              # Release libraries
├── .vscode/              # VS Code configuration
│   ├── launch.json       # Debug configurations (Debug & Release)
│   ├── tasks.json        # Build tasks (multi-config)
│   ├── settings.json     # Editor settings
│   └── c_cpp_properties.json  # IntelliSense configuration
├── CMakeLists.txt        # CMake configuration with Boost libraries
├── README.md             # Project overview and usage
└── CONTRIBUTING.md       # This file
```

### Adding New Features

#### Adding New Files

The build system automatically discovers new source files:

1. Add `.cpp` files to the appropriate `src/` subdirectory
2. Add `.hpp` files to the same subdirectory
3. Include headers in the relevant source files
4. Rebuild the project

No need to manually update `CMakeLists.txt` for new source files.

#### Adding New Dependencies

1. Update `CMakeLists.txt` to find the new library:
   ```cmake
   find_package(NewLibrary REQUIRED)
   target_link_libraries(${PROJECT_NAME} PRIVATE NewLibrary::NewLibrary)
   ```

2. Add includes to source files:
   ```cpp
   #include <newlibrary/header.hpp>
   ```

3. Update this CONTRIBUTING.md with installation instructions

### Code Style Guidelines

#### C++ Standards
- Use **C++23** features where beneficial
- Prefer **modern C++** idioms (smart pointers, RAII, etc.)
- Use **explicit** `boost::` prefixes (no `using namespace boost`)
- Follow **const-correctness** principles

#### Naming Conventions
- **Files**: `snake_case.cpp`, `snake_case.hpp`
- **Classes**: `PascalCase`
- **Functions/Variables**: `snake_case`
- **Constants**: `kCamelCase` or `UPPER_CASE`
- **Namespaces**: `lowercase`

#### Code Organization
- **Header-only** when possible for templates
- **Separate** declaration and definition for non-templates
- **Include guards** using `#pragma once`
- **Forward declarations** to reduce compile times

#### Error Handling
- Use **`std::expected`** for recoverable errors
- Use **exceptions** for unrecoverable errors
- Provide **meaningful error messages**
- Handle **Boost.System error codes** appropriately

### Testing

#### Manual Testing
```bash
# Start server in debug mode
./build-debug/bin/swftly --log-level debug

# Test health endpoint
curl -v http://localhost:8080/ping

# Test URL creation
curl -X POST http://localhost:8080/api/urls \
  -H "Content-Type: application/json" \
  -d '{"url": "https://github.com/user/repo"}' -v

# Test redirection
curl -L -v http://localhost:8080/a
```

#### Load Testing
```bash
# Install Apache Bench
sudo apt install apache2-utils

# Test concurrent requests
ab -n 1000 -c 10 http://localhost:8080/ping

# Test POST requests
ab -n 100 -c 5 -p post_data.json -T application/json http://localhost:8080/api/urls
```

## Future Compatibility Plans

Currently, the project is optimized for **Clang + libc++** to leverage cutting-edge C++23 features. Future versions will include:

### Planned Compiler Support
- **GCC 13+** with **libstdc++**
- **MSVC 2022** with **MSVCRT**
- **MinGW-w64** for Windows builds

### Simplified Build System
- Auto-detection of compiler and standard library
- Fallback to C++20 features when C++23 unavailable
- Package manager integration (vcpkg, Conan)
- Precompiled binaries for major platforms

## Troubleshooting

### Common Issues

#### CMake can't find Boost libraries
```bash
# Verify Boost installation
ls -la /usr/local/lib/libboost_* | grep -E "(log|system|program_options|json)"

# If libraries are missing, rebuild Boost:
cd ~/boost_1_88_0
sudo ./b2 clean
sudo ./b2 -j8 toolset=clang variant=release,debug \
    cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++" \
    threading=multi link=shared,static install
```

#### Linker errors with "undefined reference to boost::"
This typically means Boost libraries were built with a different standard library (libstdc++ vs libc++).
```bash
# Check what standard library Boost uses:
ldd /usr/local/lib/libboost_log.so.1.88.0

# Should show libc++.so, not libstdc++.so
# If showing libstdc++, rebuild Boost with the correct flags above
```

#### CMake can't find OpenSSL
```bash
# Verify OpenSSL installation
ls -la /usr/local/openssl-3.5.1/lib/

# Set OpenSSL path in CMake:
cmake -DOPENSSL_ROOT_DIR=/usr/local/openssl-3.5.1 -S . -B build
```

#### Build fails with C++23 feature errors
- Ensure you're using **Clang 20+** or **GCC 13+**
- Verify **libc++** is being used: check compiler output for `-stdlib=libc++`
- Some C++23 features may require newer libc++ versions

#### VS Code IntelliSense errors
- Ensure `.vscode/c_cpp_properties.json` has correct compiler path
- Reload VS Code window: `Ctrl+Shift+P` → "Developer: Reload Window"
- Check that `compile_commands.json` is generated in build directory

#### Server won't start
```bash
# Check if port 8080 is in use
sudo netstat -tlnp | grep :8080

# Check Redis is running
redis-cli ping

# Test server with verbose logging
./build-debug/bin/swftly --log-level trace
```

#### Redis connection issues
```bash
# Start Redis with default config
redis-server

# Test Redis connectivity
redis-cli ping

# Check Redis logs
tail -f /var/log/redis/redis-server.log
```

#### Permission errors during builds
```bash
# Ensure you have write permissions to /usr/local
sudo chown -R $(whoami):$(whoami) /usr/local/include /usr/local/lib

# Or use a custom prefix:
./b2 --prefix=/home/$(whoami)/boost-install install
# Then set: export BOOST_ROOT=/home/$(whoami)/boost-install
```

## Contributing Guidelines

### Before You Start
1. **Fork** the repository
2. **Clone** your fork: `git clone https://github.com/your-username/swftly.git`
3. **Create** a feature branch: `git checkout -b feature/amazing-feature`
4. **Set up** the development environment following this guide

### Development Process
1. **Write** your code following the style guidelines
2. **Test** your changes thoroughly
3. **Update** documentation if needed
4. **Commit** your changes: `git commit -m 'Add some amazing feature'`
5. **Push** to your branch: `git push origin feature/amazing-feature`
6. **Open** a Pull Request

### Pull Request Guidelines
- **Clear description** of changes and motivation
- **Reference** any related issues
- **Include tests** for new functionality
- **Update documentation** for user-facing changes
- **Ensure** all builds pass (Debug and Release)
- **Follow** existing code style and conventions

### Code Review Process
1. **Automated checks** must pass (builds, formatting)
2. **Manual review** by maintainers
3. **Address feedback** promptly
4. **Squash commits** if requested
5. **Merge** after approval

Thank you for contributing to Swftly! 