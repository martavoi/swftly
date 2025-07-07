# Swftly

A high-performance URL shortener built with modern C++23, Boost.Beast, and Redis. Designed for reliability and maximum performance with C++20 coroutines and async HTTP processing.

## Features

- **Ultra-Low Latency**: Async HTTP server with C++20 coroutines
- **Collision-Free**: Redis auto-increment with Base62 encoding  
- **High Performance**: Boost.Beast with zero-copy operations
- **Modern C++**: C++23 features with Boost libraries
- **RESTful API**: Clean JSON API endpoints
- **Health Monitoring**: Built-in health check endpoints

## Architecture

Swftly uses a simple yet scalable architecture:

1. **Redis Backend**: Stores URL mappings using auto-incrementing integer IDs
2. **Base62 Encoding**: Converts integer IDs to short codes (a-zA-Z0-9)
3. **Collision-Free**: Auto-increment ensures unique IDs, Base62 keeps URLs short
4. **Async Processing**: C++20 coroutines handle thousands of concurrent requests

```
POST /api/urls → Redis INCR → Base62 encode → Return short_code
GET /{short_code} → Base62 decode → Redis GET → 301 redirect
```

This approach scales horizontally and avoids hash collisions while keeping short codes minimal length.

## Performance Optimizations

Swftly is built for maximum performance with several key optimizations:

### Zero-Allocation Routing
- **Transparent Hashing**: C++20 transparent hash maps enable route lookups without string allocations
- **String View Lookups**: Routes are matched using `std::string_view` without creating temporary strings
- **O(1) Route Resolution**: Hash table lookups with optimized Boost hash combining

### Compiled Lookup Tables
- **Base62 Decode Table**: 256-element lookup table generated at compile time for O(1) character decoding
- **No Runtime Computation**: Character-to-value mapping computed during compilation, not at runtime
- **ASCII Optimization**: Direct array indexing using ASCII values for maximum decode speed

### Memory Efficiency
- **Stateless Operations**: Thread-safe encoding/decoding without shared state
- **Minimal Allocations**: Smart buffer management and string pre-sizing
- **Zero-Copy Operations**: Boost.Beast enables zero-copy HTTP processing where possible

These optimizations ensure Swftly can handle thousands of concurrent requests with minimal latency.

## Usage Examples

### Create Short URL
```bash
curl -X POST http://localhost:8080/api/urls \
  -H "Content-Type: application/json" \
  -d '{"url": "https://www.example.com"}'
```

**Response:**
```json
{
  "short_code": "a",
  "url": "https://www.example.com"
}
```

### Use Short URL
```bash
curl -L http://localhost:8080/a
# Redirects to: https://www.example.com
```

### Health Check
```bash
curl http://localhost:8080/ping
```

**Response:**
```json
{
  "status": "ok",
  "timestamp": "2024-01-15T10:30:45Z"
}
```

## Quick Start

### Prerequisites
- **C++23 Compiler**: Clang 20+ or GCC 13+
- **Redis Server**: Version 6+
- **Boost Libraries**: 1.85+ (compiled with matching stdlib)
- **OpenSSL**: For Redis connections

### Build and Run

```bash
# 1. Start Redis
redis-server

# 2. Clone and build
git clone <your-repo>
cd swftly
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Run the server
./build/bin/swftly
```

The server starts on `http://localhost:8080` by default.

**Note**: This project was built with Clang and libc++ for optimal C++23 support. Future versions will support GCC/libstdc++/MinGW for broader compatibility.

For detailed build instructions, dependencies, and development setup, see [CONTRIBUTING.md](CONTRIBUTING.md).

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/` | Server information |
| `GET` | `/ping` | Health check |
| `POST` | `/api/urls` | Create short URL |
| `GET` | `/{short_code}` | Redirect to original URL |

## Configuration

Environment variables:
- `SWFTLY_PORT`: Server port (default: 8080)
- `SWFTLY_HOST`: Server host (default: 127.0.0.1)
- `REDIS_HOST`: Redis host (default: 127.0.0.1)
- `REDIS_PORT`: Redis port (default: 6379)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup, build instructions, and contribution guidelines.

## License

[Add your license here]
