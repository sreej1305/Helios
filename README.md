# Helios

Helios is a high-performance, Linux-based HTTP server written in pure C. It utilizes `epoll` for scalable I/O and a custom thread pool to handle concurrent client connections efficiently.

## Features

-   **Scalable I/O**: Leverages `epoll` (edge-triggered) for high-concurrency event multiplexing.
-   **Concurrency**: Implements a producer-consumer thread pool for parallel request processing.
-   **Low Overhead**: Built with pure C and POSIX libraries for maximum performance.
-   **Real-time Metrics**: Tracks total requests, active connections, and average response times.
-   **Dynamic Logging**: Thread-safe logging to `logs/server.log`.
-   **Graceful Shutdown**: Properly cleans up resources and shuts down on `SIGINT` (Ctrl+C).

## Endpoints

-   `GET /`: Returns a welcome message.
-   `GET /health`: Returns JSON status with connections and simplified uptime.
-   `GET /metrics`: Returns a text summary of server performance.
-   `POST /echo`: Returns the body of the request.
-   `404`: Handles unknown endpoints with a custom text response.

## Getting Started

### Prerequisites

-   Linux OS (for `epoll` support)
-   GCC compiler
-   `make` build utility
-   `curl` (for testing)

### Installation

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/sreej1305/Helios.git
    cd Helios
    ```

2.  **Build the project**:
    ```bash
    make
    ```

### Usage

1.  **Start the server**:
    ```bash
    ./helios
    ```

2.  **Test endpoints**:
    ```bash
    curl http://localhost:8080/
    curl http://localhost:8080/health
    curl http://localhost:8080/metrics
    curl -d "Hello Helios" http://localhost:8080/echo
    ```

## Configuration

Settings can be adjusted in `helios.conf`:

```conf
port=8080
thread_pool_size=4
queue_size=100
log_path=logs/server.log
```

## Architecture

Helios follows a modular architecture:

-   `src/server.c`: Main `epoll` loop and socket handling.
-   `src/threadpool.c`: Worker thread management and task queue.
-   `src/http.c`: Request parsing and response routing.
-   `src/metrics.c`: Thread-safe performance tracking.
-   `src/logger.c`: Thread-safe file logging.
-   `src/config.c`: Configuration loading.

## License

This project is open-source and available under the MIT License.
