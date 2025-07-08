# Contributing to Swftly

Thank you for your interest in contributing to Swftly! This document provides detailed setup instructions, development guidelines, and troubleshooting tips.

## Development Environment Setup

### Prerequisites

#### Required Tools
- **CMake** 3.16 or higher
- **C++23 Compiler**: 
  - **Clang** 15+ (recommended for C++23 features)
  - **GCC** 13+ (good C++23 support)
  - **MSVC** 2022+ (partial C++23 support)
- **Boost Libraries** 1.75+ (system, log, program_options, json components)
- **OpenSSL** 3.0+ (for Redis connections)
- **Redis Server** 6+ (for runtime)
- **Git** (for cloning)

### Platform Setup

#### Linux/WSL (Recommended)

##### Option 1: System Packages (Easiest)

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git redis-server
sudo apt install libboost-all-dev libssl-dev

# Fedora/RHEL
sudo dnf install gcc gcc-c++ cmake git redis boost-devel openssl-devel

# Arch Linux
sudo pacman -S base-devel cmake git redis boost openssl
```

##### Option 2: Modern Compilers (For Latest Features)

```bash
# Ubuntu/Debian - Install latest Clang
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18  # or latest version

# Ubuntu/Debian - Install latest GCC
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install gcc-13 g++-13

# Set as default (optional)
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```

##### Option 3: Custom Build (Advanced Users Only)

Only needed if system packages are too old or you need specific optimizations:

<details>
<summary>Custom Boost Build Instructions</summary>

```bash
# Download Boost (only if system version is too old)
cd ~/
wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.gz
tar xzf boost_1_85_0.tar.gz
cd boost_1_85_0

# Build with your preferred compiler
./bootstrap.sh --with-toolset=gcc  # or clang
sudo ./b2 -j8 variant=release,debug threading=multi link=shared,static install

# For Clang with libc++ (optional)
sudo ./b2 -j8 toolset=clang variant=release,debug \
    cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++" \
    threading=multi link=shared,static install
```
</details>

#### Windows Setup

##### Option 1: MSYS2 (Recommended)
```bash
# Install MSYS2 from https://www.msys2.org/
# Open MSYS2 terminal and install packages
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-boost mingw-w64-x86_64-openssl
pacman -S git make
```

##### Option 2: Visual Studio
```bash
# Install Visual Studio 2022 with C++ workload
# Install vcpkg for dependencies
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install boost openssl
```

##### Option 3: WSL2 (Use Linux instructions above)

#### macOS Setup

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake boost openssl redis git

# For latest Clang
brew install llvm

# For latest GCC  
brew install gcc
```

### Building the Project

The new build system automatically detects your compiler and standard library, making it much easier to build across different environments.

#### Quick Build Commands

**Using System Compiler (Auto-detected):**
```bash
# Configure and build (Release)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Configure and build (Debug)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

**Explicitly Choosing Compiler:**
```bash
# Use GCC
cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Use Clang with default standard library
cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Use Clang with libc++ (optional)
cmake -B build -DCMAKE_CXX_COMPILER=clang++ -DUSE_LIBCXX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Use custom OpenSSL installation
cmake -B build -DOPENSSL_CUSTOM_PATH=/usr/local/openssl-3.5.1 -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Windows-specific:**
```bash
# MSYS2/MinGW
cmake -G "MinGW Makefiles" -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Visual Studio
cmake -B build
cmake --build build --config Release
```

#### Build Options

- **`USE_LIBCXX=ON`**: Force use of libc++ with Clang (default: OFF, uses default stdlib)
- **`CMAKE_BUILD_TYPE`**: Debug, Release, RelWithDebInfo, MinSizeRel
- **`BOOST_ROOT`**: Custom Boost installation path (if not in standard location)
- **`OPENSSL_CUSTOM_PATH`**: Custom OpenSSL installation path (e.g., `/usr/local/openssl-3.5.1`)

#### Build Configurations

- **Debug**: Full debug symbols, no optimization, all warnings
- **Release**: Maximum optimization, no debug symbols, Link Time Optimization (LTO) where available
- **RelWithDebInfo**: Optimized with debug symbols for profiling
- **MinSizeRel**: Size-optimized release

#### Clean Build
```bash
# Clean specific configuration
cmake --build build --target clean

# Remove build directory entirely
rm -rf build build-debug
```

### Compiler and Standard Library Support

#### Tested Combinations

| Compiler | Standard Library | Status | Notes |
|----------|------------------|--------|-------|
| Clang 15+ | libstdc++ | ✅ Fully Supported | Default, most compatible |
| Clang 15+ | libc++ | ✅ Fully Supported | Use `-DUSE_LIBCXX=ON` |
| GCC 13+ | libstdc++ | ✅ Fully Supported | Default for GCC |
| MSVC 2022 | MSVCRT | ⚠️ Partial | Some C++23 features limited |

#### Feature Support by Compiler

| Feature | Clang 15+ | GCC 13+ | MSVC 2022 |
|---------|-----------|---------|-----------|
| `std::expected` | ✅ | ✅ | ✅ |
| `std::format` | ✅ | ✅ | ✅ |
| Coroutines | ✅ | ✅ | ✅ |
| Concepts | ✅ | ✅ | ✅ |

### Running the Application

#### Start Dependencies
```bash
# Start Redis server
redis-server

# Or with custom config
redis-server /path/to/redis.conf

# Check Redis is running
redis-cli ping
```

#### Run Swftly
```bash
# Debug build
./build-debug/bin/swftly --port 8080 --redis-host 127.0.0.1 --redis-port 6379

# Release build  
./build/bin/swftly --port 8080 --redis-host 127.0.0.1 --redis-port 6379

# With custom log level
./build/bin/swftly --log-level debug --port 8080
```

#### Test the API
```bash
# Health check
curl http://localhost:8080/ping

# Create short URL
curl -X POST http://localhost:8080/api/urls \
  -H "Content-Type: application/json" \
  -d '{"url": "https://www.example.com"}'

# Use short URL (follow redirects)
curl -L http://localhost:8080/a
```

## Development Workflow

### VS Code Setup

The project includes pre-configured VS Code tasks that work with the new build system:

- **Build Debug**: `Ctrl+Shift+B` (default task)
- **Build Release**: `Ctrl+Shift+P` → "Tasks: Run Task" → "cmake-build-release"
- **Configure**: `Ctrl+Shift+P` → "Tasks: Run Task" → "cmake-configure"
- **Clean**: `Ctrl+Shift+P` → "Tasks: Run Task" → "cmake-clean"

#### Available Tasks:
- `cmake-configure` - Configure build system
- `cmake-build-debug` - Build Debug configuration
- `cmake-build-release` - Build Release configuration
- `cmake-clean` - Clean build artifacts

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
├── build/                 # Release build output (generated)
│   └── bin/swftly        # Optimized release build
├── build-debug/           # Debug build output (generated)
│   ├── bin/swftly        # Debug build with symbols
│   └── compile_commands.json  # For clang-tidy and IDEs
├── .vscode/              # VS Code configuration
├── CMakeLists.txt        # Portable CMake configuration
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
- Use **C++23** features where beneficial and widely supported
- Prefer **modern C++** idioms (smart pointers, RAII, etc.)
- Use **explicit** `boost::` prefixes (no `using namespace boost`)
- Follow **const-correctness** principles
- Use standard library features over compiler extensions

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
# Install Apache Bench (Ubuntu/Debian)
sudo apt install apache2-utils

# Test concurrent requests
ab -n 1000 -c 10 http://localhost:8080/ping

# Test POST requests (create test data file first)
echo '{"url": "https://example.com"}' > post_data.json
ab -n 100 -c 5 -p post_data.json -T application/json http://localhost:8080/api/urls
```

## Troubleshooting

### Common Issues

#### CMake can't find Boost libraries
```bash
# Try installing development packages
# Ubuntu/Debian
sudo apt install libboost-all-dev

# Fedora/RHEL
sudo dnf install boost-devel

# If using custom installation, set BOOST_ROOT
cmake -DBOOST_ROOT=/path/to/boost -B build
```

#### CMake can't find OpenSSL
```bash
# Install development packages
# Ubuntu/Debian
sudo apt install libssl-dev

# Fedora/RHEL
sudo dnf install openssl-devel

# For custom installation
cmake -DOPENSSL_CUSTOM_PATH=/usr/local/openssl-3.5.1 -B build
```

#### Compiler not found
```bash
# Explicitly specify compiler
cmake -DCMAKE_CXX_COMPILER=g++-13 -B build
cmake -DCMAKE_CXX_COMPILER=clang++-15 -B build
```

#### Build fails with C++23 feature errors
- Check if your compiler supports the required features
- The build system will warn about missing features
- Consider using a newer compiler version

#### Linking errors with different standard libraries
- Make sure all components (Boost, OpenSSL, your app) use the same standard library
- For Clang: use either all libstdc++ or all libc++
- Check build output for standard library mismatches

#### VS Code IntelliSense errors
- Ensure `compile_commands.json` is generated in build directory
- Reload VS Code window: `Ctrl+Shift+P` → "Developer: Reload Window"
- Check that VS Code is using the correct compiler in settings

#### Server won't start
```bash
# Check if port 8080 is in use
sudo netstat -tlnp | grep :8080

# Test Redis connectivity
redis-cli ping

# Run with verbose logging
./build/bin/swftly --log-level trace
```

#### Redis connection issues
```bash
# Start Redis server
redis-server

# Check Redis status
redis-cli ping

# Check Redis configuration
redis-cli config get '*'
```

## Contributing Guidelines

### Before You Start
1. **Fork** the repository
2. **Clone** your fork: `git clone https://github.com/your-username/swftly.git`
3. **Create** a feature branch: `git checkout -b feature/amazing-feature`
4. **Set up** the development environment following this guide

### Development Process
1. **Write** your code following the style guidelines
2. **Test** your changes thoroughly on your platform
3. **Update** documentation if needed
4. **Commit** your changes: `git commit -m 'Add some amazing feature'`
5. **Push** to your branch: `git push origin feature/amazing-feature`
6. **Open** a Pull Request

### Pull Request Guidelines
- **Clear description** of changes and motivation
- **Reference** any related issues
- **Test** on multiple compilers if possible
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