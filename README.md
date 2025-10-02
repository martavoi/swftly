# Swftly

> **Production-ready URL shortener built with C++23 and Redis**  
> Ultra-fast, collision-free, horizontally scalable

[![Docker](https://img.shields.io/badge/docker-ghcr.io-blue)](https://github.com/martavoi/swftly/pkgs/container/swftly)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Redis](https://img.shields.io/badge/Redis-6%2B-red.svg)](https://redis.io/)

---

## 📑 Table of Contents

- [Quick Start](#-quick-start)
- [Features](#-features)
- [API Usage](#-api-usage)
- [Configuration](#️-configuration)
- [Architecture](#️-architecture)

---

## 🚀 Quick Start

### Using Docker (Recommended)

```bash
# Pull the latest image
docker pull ghcr.io/martavoi/swftly:latest

docker run -d \
  -p 8080:8080 \
  -e SWFTLY_REDIS_HOST=redis \
  ghcr.io/martavoi/swftly:latest
```

Server starts at `http://localhost:8080` 🎉

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| ⚡ **Ultra-Fast** | C++23 with zero-copy operations and O(1) lookups |
| 🔒 **Collision-Free** | Redis auto-increment ensures unique short codes |
| 📈 **Horizontally Scalable** | Stateless design, scales with Redis |
| 🐳 **Production-Ready** | Docker image, health checks, structured logging |
| 🔄 **Async I/O** | C++20 coroutines handle thousands of concurrent requests |

---

## 📖 API Usage

**Create a short URL:**
```bash
curl -X POST http://localhost:8080/api/urls \
  -H "Content-Type: application/json" \
  -d '{"url": "https://example.com"}'
```
```json
{ "short_code": "a", "url": "https://example.com" }
```

**Use the short URL:**
```bash
curl -L http://localhost:8080/a  # Redirects to https://example.com
```

**Health check:**
```bash
curl http://localhost:8080/ping
```

### Available Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `POST` | `/api/urls` | Create short URL |
| `GET` | `/{short_code}` | Redirect to original URL |
| `GET` | `/ping` | Health check |
| `GET` | `/` | Server info & version |

---

## ⚙️ Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `SWFTLY_ADDRESS` | `127.0.0.1` | Server bind address |
| `SWFTLY_PORT` | `8080` | Server port |
| `SWFTLY_THREADS` | `1` | Worker threads |
| `SWFTLY_LOG_LEVEL` | `info` | Log level (trace/debug/info/warning/error/fatal) |
| `SWFTLY_REDIS_HOST` | `127.0.0.1` | Redis server host |
| `SWFTLY_REDIS_PORT` | `6379` | Redis server port |

### Command-Line Arguments

```bash
./swftly --port 3000 --log-level debug --redis-host redis.example.com
```

| Flag | Description |
|------|-------------|
| `-p, --port` | Server port |
| `-a, --address` | Bind address |
| `-t, --threads` | Worker threads |
| `-l, --log-level` | Log level |
| `--redis-host` | Redis host |
| `--redis-port` | Redis port |
| `-h, --help` | Show help |

> 💡 **Tip:** CLI arguments override environment variables

---

## 🏗️ Architecture

```
POST /api/urls → Redis INCR → Base62 encode → short_code
GET /{code}    → Base62 decode → Redis GET → 301 redirect
```

**Tech Stack:**
- **C++23** - Modern C++ with coroutines
- **Boost.Beast** - High-performance HTTP server
- **Redis** - Fast key-value storage
- **Base62** - Compact URL encoding (a-zA-Z0-9)

**Why Fast:**
- Zero-copy operations
- Compile-time lookup tables
- O(1) routing with transparent hash maps
- Async I/O with coroutines

---

## 📦 Releases

View all releases and changelogs: [Releases Page](https://github.com/martavoi/swftly/releases)

Docker images: [GitHub Packages](https://github.com/martavoi/swftly/pkgs/container/swftly)

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details
