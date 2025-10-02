# syntax=docker/dockerfile:1

# ==============================================================================
# Builder Stage - Compile the application
# ==============================================================================
FROM alpine:3 AS builder

# Accept version as build argument (set by CI/CD)
ARG VERSION=0.1.0

# Install build dependencies
# - g++ for C++23 support (Alpine 3.21+ has GCC 13.2+)
# - cmake for build system
# - boost-dev for Boost libraries (1.84+ in Alpine 3.21)
# - openssl-dev for SSL/TLS support
# - make for build process
RUN apk add --no-cache \
    g++ \
    cmake \
    make \
    boost-dev \
    openssl-dev \
    && rm -rf /var/cache/apk/*

# Set working directory for build
WORKDIR /build

# Copy only dependency files first for better layer caching
COPY CMakeLists.txt ./

# Copy source code
COPY src/ ./src/

# Configure and build in Release mode with optimizations
# Use all available cores for compilation
# Pass version to CMake
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DPROJECT_VERSION=${VERSION} \
    && cmake --build build --parallel $(nproc) \
    && strip build/bin/swftly

# ==============================================================================
# Runtime Stage - Minimal production image
# ==============================================================================
FROM alpine:3 AS runtime

# Install only runtime dependencies
# - libstdc++ for C++ standard library
# - boost-* runtime libraries (only what we need, not -dev packages)
# - openssl for SSL/TLS
RUN apk add --no-cache \
    libstdc++ \
    boost-system \
    boost-log \
    boost-program_options \
    boost-json \
    openssl \
    ca-certificates \
    && rm -rf /var/cache/apk/*

# Create non-root user for security
RUN addgroup -g 1000 swftly \
    && adduser -D -u 1000 -G swftly swftly

# Set working directory
WORKDIR /app

# Copy binary from builder stage
COPY --from=builder --chown=swftly:swftly /build/build/bin/swftly /app/swftly

# Switch to non-root user
USER swftly

# Expose default port (can be overridden)
EXPOSE 8080

# Add healthcheck
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget --no-verbose --tries=1 --spider http://localhost:8080/ping || exit 1

# Run the application
ENTRYPOINT ["/app/swftly"]
CMD []

