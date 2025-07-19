# Performance Optimization Guide

This guide provides comprehensive strategies for optimizing performance in CoreBaseApplication (CBA). It covers profiling techniques, optimization strategies, and best practices for achieving high-performance applications.

## Table of Contents

- [Performance Profiling](#performance-profiling)
- [CPU Optimization](#cpu-optimization)
- [Memory Optimization](#memory-optimization)
- [I/O Optimization](#io-optimization)
- [Network Optimization](#network-optimization)
- [Concurrency and Threading](#concurrency-and-threading)
- [Compiler Optimizations](#compiler-optimizations)
- [Benchmarking](#benchmarking)
- [Performance Monitoring](#performance-monitoring)

## Performance Profiling

### CPU Profiling

#### Using Perf (Linux)

```bash
# Profile CPU usage
perf record -g ./your_application
perf report

# Profile specific function
perf record -g -e cpu-cycles ./your_application
perf annotate function_name

# Real-time profiling
perf top -g
```

#### Using Visual Studio Profiler (Windows)

```cpp
// Add profiling markers in code
#ifdef _WIN32
#include <profileapi.h>

class ScopedProfiler {
public:
    explicit ScopedProfiler(const char* name) : m_name(name) {
        QueryPerformanceCounter(&m_start);
    }
    
    ~ScopedProfiler() {
        LARGE_INTEGER end, frequency;
        QueryPerformanceCounter(&end);
        QueryPerformanceFrequency(&frequency);
        
        double elapsed = static_cast<double>(end.QuadPart - m_start.QuadPart) / frequency.QuadPart;
        printf("%s: %.6f seconds\n", m_name, elapsed);
    }
    
private:
    const char* m_name;
    LARGE_INTEGER m_start;
};

#define PROFILE_SCOPE(name) ScopedProfiler _prof(name)
#else
#define PROFILE_SCOPE(name)
#endif

// Usage
void expensiveFunction() {
    PROFILE_SCOPE("expensiveFunction");
    // Function implementation
}
```

#### Custom Profiling Infrastructure

```cpp
// High-resolution timer
class HighResolutionTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::nanoseconds;
    
    void start() {
        m_start = Clock::now();
    }
    
    Duration elapsed() const {
        return std::chrono::duration_cast<Duration>(Clock::now() - m_start);
    }
    
    double elapsedSeconds() const {
        return elapsed().count() / 1e9;
    }
    
private:
    TimePoint m_start;
};

// Profiling manager
class ProfileManager {
public:
    struct ProfileData {
        std::string name;
        size_t callCount = 0;
        Duration totalTime{0};
        Duration minTime{Duration::max()};
        Duration maxTime{Duration::min()};
        
        double averageMs() const {
            return callCount > 0 ? (totalTime.count() / callCount) / 1e6 : 0.0;
        }
    };
    
    static ProfileManager& getInstance() {
        static ProfileManager instance;
        return instance;
    }
    
    void recordProfile(const std::string& name, Duration duration) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& data = m_profiles[name];
        data.name = name;
        data.callCount++;
        data.totalTime += duration;
        data.minTime = std::min(data.minTime, duration);
        data.maxTime = std::max(data.maxTime, duration);
    }
    
    void printReport() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        printf("\n=== Performance Report ===\n");
        printf("%-30s %10s %10s %10s %10s\n", "Function", "Calls", "Avg(ms)", "Min(ms)", "Max(ms)");
        printf("%-30s %10s %10s %10s %10s\n", "--------", "-----", "-------", "-------", "-------");
        
        for (const auto& [name, data] : m_profiles) {
            printf("%-30s %10zu %10.3f %10.3f %10.3f\n",
                   name.c_str(),
                   data.callCount,
                   data.averageMs(),
                   data.minTime.count() / 1e6,
                   data.maxTime.count() / 1e6);
        }
    }
    
private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ProfileData> m_profiles;
};

// RAII profiler
class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name) : m_name(name) {
        m_timer.start();
    }
    
    ~ScopedProfiler() {
        ProfileManager::getInstance().recordProfile(m_name, m_timer.elapsed());
    }
    
private:
    std::string m_name;
    HighResolutionTimer m_timer;
};

#define PROFILE_FUNCTION() ScopedProfiler _prof(__FUNCTION__)
#define PROFILE_SCOPE(name) ScopedProfiler _prof(name)
```

### Memory Profiling

#### Using Valgrind (Linux)

```bash
# Memory leak detection
valgrind --tool=memcheck --leak-check=full ./your_application

# Cache profiling
valgrind --tool=cachegrind ./your_application
cg_annotate cachegrind.out.pid

# Heap profiling
valgrind --tool=massif ./your_application
ms_print massif.out.pid
```

#### Custom Memory Tracking

```cpp
// Memory tracker
class MemoryTracker {
public:
    struct AllocationInfo {
        size_t size;
        std::string file;
        int line;
        std::chrono::time_point<std::chrono::steady_clock> timestamp;
    };
    
    static MemoryTracker& getInstance() {
        static MemoryTracker instance;
        return instance;
    }
    
    void recordAllocation(void* ptr, size_t size, const char* file, int line) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_allocations[ptr] = {size, file, line, std::chrono::steady_clock::now()};
        m_totalAllocated += size;
        m_currentAllocated += size;
        m_peakAllocated = std::max(m_peakAllocated, m_currentAllocated);
    }
    
    void recordDeallocation(void* ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_allocations.find(ptr);
        if (it != m_allocations.end()) {
            m_currentAllocated -= it->second.size;
            m_allocations.erase(it);
        }
    }
    
    void printReport() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        printf("\n=== Memory Report ===\n");
        printf("Total allocated: %zu bytes\n", m_totalAllocated);
        printf("Current allocated: %zu bytes\n", m_currentAllocated);
        printf("Peak allocated: %zu bytes\n", m_peakAllocated);
        printf("Active allocations: %zu\n", m_allocations.size());
        
        if (!m_allocations.empty()) {
            printf("\nMemory leaks detected:\n");
            for (const auto& [ptr, info] : m_allocations) {
                printf("  %p: %zu bytes at %s:%d\n", ptr, info.size, info.file.c_str(), info.line);
            }
        }
    }
    
private:
    mutable std::mutex m_mutex;
    std::unordered_map<void*, AllocationInfo> m_allocations;
    size_t m_totalAllocated = 0;
    size_t m_currentAllocated = 0;
    size_t m_peakAllocated = 0;
};

// Override global new/delete (optional, for debugging)
#ifdef ENABLE_MEMORY_TRACKING
void* operator new(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    MemoryTracker::getInstance().recordAllocation(ptr, size, file, line);
    return ptr;
}

void operator delete(void* ptr) noexcept {
    MemoryTracker::getInstance().recordDeallocation(ptr);
    free(ptr);
}

#define new new(__FILE__, __LINE__)
#endif
```

## CPU Optimization

### Algorithm Optimization

```cpp
// Use appropriate data structures
class OptimizedLookup {
public:
    // O(1) average case lookup
    void addMapping(const std::string& key, int value) {
        m_hashMap[key] = value;
    }
    
    // O(log n) lookup with ordering
    void addSortedMapping(const std::string& key, int value) {
        m_sortedMap[key] = value;
    }
    
    // Use flat_map for better cache locality (if available)
    void addFlatMapping(const std::string& key, int value) {
        // boost::container::flat_map or std::flat_map (C++23)
        auto it = std::lower_bound(m_flatData.begin(), m_flatData.end(), key,
                                  [](const auto& pair, const std::string& k) {
                                      return pair.first < k;
                                  });
        if (it != m_flatData.end() && it->first == key) {
            it->second = value;
        } else {
            m_flatData.emplace(it, key, value);
        }
    }
    
private:
    std::unordered_map<std::string, int> m_hashMap;
    std::map<std::string, int> m_sortedMap;
    std::vector<std::pair<std::string, int>> m_flatData;
};

// Cache-friendly data layout
struct ParticleAoS { // Array of Structures (cache-unfriendly)
    float x, y, z;
    float vx, vy, vz;
    float mass;
    int id;
};

class ParticleSoA { // Structure of Arrays (cache-friendly)
public:
    void addParticle(float x, float y, float z, float vx, float vy, float vz, float mass, int id) {
        m_x.push_back(x);
        m_y.push_back(y);
        m_z.push_back(z);
        m_vx.push_back(vx);
        m_vy.push_back(vy);
        m_vz.push_back(vz);
        m_mass.push_back(mass);
        m_id.push_back(id);
    }
    
    // Update positions (vectorizable)
    void updatePositions(float dt) {
        const size_t count = m_x.size();
        for (size_t i = 0; i < count; ++i) {
            m_x[i] += m_vx[i] * dt;
            m_y[i] += m_vy[i] * dt;
            m_z[i] += m_vz[i] * dt;
        }
    }
    
private:
    std::vector<float> m_x, m_y, m_z;
    std::vector<float> m_vx, m_vy, m_vz;
    std::vector<float> m_mass;
    std::vector<int> m_id;
};
```

### SIMD Optimization

```cpp
#include <immintrin.h>

// Vectorized operations
class VectorMath {
public:
    // Add two arrays of floats using AVX
    static void addArraysAVX(const float* a, const float* b, float* result, size_t count) {
        const size_t simdCount = count & ~7; // Round down to multiple of 8
        
        for (size_t i = 0; i < simdCount; i += 8) {
            __m256 va = _mm256_load_ps(&a[i]);
            __m256 vb = _mm256_load_ps(&b[i]);
            __m256 vr = _mm256_add_ps(va, vb);
            _mm256_store_ps(&result[i], vr);
        }
        
        // Handle remaining elements
        for (size_t i = simdCount; i < count; ++i) {
            result[i] = a[i] + b[i];
        }
    }
    
    // Dot product using AVX
    static float dotProductAVX(const float* a, const float* b, size_t count) {
        __m256 sum = _mm256_setzero_ps();
        const size_t simdCount = count & ~7;
        
        for (size_t i = 0; i < simdCount; i += 8) {
            __m256 va = _mm256_load_ps(&a[i]);
            __m256 vb = _mm256_load_ps(&b[i]);
            sum = _mm256_fmadd_ps(va, vb, sum);
        }
        
        // Horizontal sum
        __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(sum, 0), _mm256_extractf128_ps(sum, 1));
        sum128 = _mm_hadd_ps(sum128, sum128);
        sum128 = _mm_hadd_ps(sum128, sum128);
        
        float result = _mm_cvtss_f32(sum128);
        
        // Handle remaining elements
        for (size_t i = simdCount; i < count; ++i) {
            result += a[i] * b[i];
        }
        
        return result;
    }
};

// Auto-vectorization friendly code
void processData(std::vector<float>& data) {
    // Compiler can auto-vectorize this loop
    const size_t size = data.size();
    for (size_t i = 0; i < size; ++i) {
        data[i] = data[i] * 2.0f + 1.0f;
    }
}
```

### Branch Optimization

```cpp
// Minimize branches in hot paths
class BranchOptimization {
public:
    // Use lookup tables instead of branches
    static int signLookup(float value) {
        static const int signTable[2] = {-1, 1};
        return signTable[value >= 0.0f];
    }
    
    // Use conditional moves
    static int maxBranchless(int a, int b) {
        return a > b ? a : b; // Compiler generates conditional move
    }
    
    // Predict branches correctly
    static void processItems(const std::vector<Item>& items) {
        for (const auto& item : items) {
            // Use [[likely]] and [[unlikely]] (C++20)
            if (item.isValid()) [[likely]] {
                processValidItem(item);
            } else [[unlikely]] {
                handleInvalidItem(item);
            }
        }
    }
    
    // Separate hot and cold paths
    static void hotPath(const Data& data) {
        if (data.isCommonCase()) {
            // Hot path - keep inline
            processCommonCase(data);
        } else {
            // Cold path - move to separate function
            handleRareCase(data);
        }
    }
    
private:
    static void processValidItem(const Item& item) { /* implementation */ }
    static void handleInvalidItem(const Item& item) { /* implementation */ }
    static void processCommonCase(const Data& data) { /* implementation */ }
    
    // Mark cold functions as noinline
    [[gnu::noinline]] static void handleRareCase(const Data& data) {
        // Rare case implementation
    }
};
```

## Memory Optimization

### Memory Pool Allocation

```cpp
// Fixed-size memory pool
template<typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool() {
        // Initialize free list
        for (size_t i = 0; i < PoolSize - 1; ++i) {
            reinterpret_cast<FreeNode*>(&m_storage[i])->next = 
                reinterpret_cast<FreeNode*>(&m_storage[i + 1]);
        }
        reinterpret_cast<FreeNode*>(&m_storage[PoolSize - 1])->next = nullptr;
        m_freeHead = reinterpret_cast<FreeNode*>(&m_storage[0]);
    }
    
    template<typename... Args>
    T* allocate(Args&&... args) {
        if (!m_freeHead) {
            return nullptr; // Pool exhausted
        }
        
        FreeNode* node = m_freeHead;
        m_freeHead = m_freeHead->next;
        
        return new(node) T(std::forward<Args>(args)...);
    }
    
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        ptr->~T();
        
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = m_freeHead;
        m_freeHead = node;
    }
    
private:
    struct FreeNode {
        FreeNode* next;
    };
    
    alignas(T) char m_storage[PoolSize][sizeof(T)];
    FreeNode* m_freeHead;
};

// Variable-size memory pool
class VariableMemoryPool {
public:
    explicit VariableMemoryPool(size_t blockSize = 64 * 1024) 
        : m_blockSize(blockSize) {
        allocateNewBlock();
    }
    
    ~VariableMemoryPool() {
        for (auto* block : m_blocks) {
            std::free(block);
        }
    }
    
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);
        
        if (m_currentOffset + alignedSize > m_blockSize) {
            allocateNewBlock();
        }
        
        void* ptr = static_cast<char*>(m_currentBlock) + m_currentOffset;
        m_currentOffset += alignedSize;
        return ptr;
    }
    
    void reset() {
        m_currentBlock = m_blocks.empty() ? nullptr : m_blocks[0];
        m_currentOffset = 0;
    }
    
private:
    void allocateNewBlock() {
        void* block = std::aligned_alloc(64, m_blockSize); // 64-byte aligned
        if (!block) {
            throw std::bad_alloc();
        }
        m_blocks.push_back(block);
        m_currentBlock = block;
        m_currentOffset = 0;
    }
    
    std::vector<void*> m_blocks;
    void* m_currentBlock = nullptr;
    size_t m_currentOffset = 0;
    size_t m_blockSize;
};
```

### Cache Optimization

```cpp
// Cache-friendly data structures
template<typename T>
class CacheFriendlyVector {
public:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    static constexpr size_t ELEMENTS_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(T);
    
    void push_back(const T& value) {
        if (m_size >= m_capacity) {
            grow();
        }
        m_data[m_size++] = value;
    }
    
    // Prefetch data for better cache performance
    void prefetchRange(size_t start, size_t count) const {
        const size_t end = std::min(start + count, m_size);
        for (size_t i = start; i < end; i += ELEMENTS_PER_CACHE_LINE) {
            __builtin_prefetch(&m_data[i], 0, 3); // Read, high temporal locality
        }
    }
    
    // Process data in cache-friendly chunks
    template<typename Func>
    void processInChunks(Func func, size_t chunkSize = 1024) const {
        for (size_t i = 0; i < m_size; i += chunkSize) {
            const size_t end = std::min(i + chunkSize, m_size);
            
            // Prefetch next chunk
            if (end < m_size) {
                prefetchRange(end, std::min(chunkSize, m_size - end));
            }
            
            // Process current chunk
            for (size_t j = i; j < end; ++j) {
                func(m_data[j]);
            }
        }
    }
    
private:
    void grow() {
        size_t newCapacity = m_capacity == 0 ? 16 : m_capacity * 2;
        T* newData = static_cast<T*>(std::aligned_alloc(CACHE_LINE_SIZE, newCapacity * sizeof(T)));
        
        if (m_data) {
            std::uninitialized_move_n(m_data, m_size, newData);
            std::free(m_data);
        }
        
        m_data = newData;
        m_capacity = newCapacity;
    }
    
    T* m_data = nullptr;
    size_t m_size = 0;
    size_t m_capacity = 0;
};
```

## I/O Optimization

### Asynchronous I/O

```cpp
// Async file operations
class AsyncFileReader {
public:
    using ReadCallback = std::function<void(std::vector<char>, std::error_code)>;
    
    AsyncFileReader() : m_ioContext(), m_work(m_ioContext) {
        // Start worker threads
        for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {
            m_threads.emplace_back([this] {
                m_ioContext.run();
            });
        }
    }
    
    ~AsyncFileReader() {
        m_ioContext.stop();
        for (auto& thread : m_threads) {
            thread.join();
        }
    }
    
    void readFileAsync(const std::string& filename, ReadCallback callback) {
        boost::asio::post(m_ioContext, [filename, callback = std::move(callback)] {
            std::ifstream file(filename, std::ios::binary);
            if (!file) {
                callback({}, std::make_error_code(std::errc::no_such_file_or_directory));
                return;
            }
            
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            std::vector<char> data(size);
            file.read(data.data(), size);
            
            callback(std::move(data), {});
        });
    }
    
private:
    boost::asio::io_context m_ioContext;
    boost::asio::io_context::work m_work;
    std::vector<std::thread> m_threads;
};

// Memory-mapped file I/O
class MemoryMappedFile {
public:
    explicit MemoryMappedFile(const std::string& filename) {
#ifdef _WIN32
        m_fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (m_fileHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open file");
        }
        
        LARGE_INTEGER fileSize;
        GetFileSizeEx(m_fileHandle, &fileSize);
        m_size = fileSize.QuadPart;
        
        m_mappingHandle = CreateFileMappingA(m_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!m_mappingHandle) {
            CloseHandle(m_fileHandle);
            throw std::runtime_error("Failed to create file mapping");
        }
        
        m_data = MapViewOfFile(m_mappingHandle, FILE_MAP_READ, 0, 0, 0);
        if (!m_data) {
            CloseHandle(m_mappingHandle);
            CloseHandle(m_fileHandle);
            throw std::runtime_error("Failed to map file");
        }
#else
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error("Failed to open file");
        }
        
        struct stat st;
        if (fstat(fd, &st) == -1) {
            close(fd);
            throw std::runtime_error("Failed to get file size");
        }
        
        m_size = st.st_size;
        m_data = mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        
        if (m_data == MAP_FAILED) {
            throw std::runtime_error("Failed to map file");
        }
#endif
    }
    
    ~MemoryMappedFile() {
#ifdef _WIN32
        if (m_data) UnmapViewOfFile(m_data);
        if (m_mappingHandle) CloseHandle(m_mappingHandle);
        if (m_fileHandle != INVALID_HANDLE_VALUE) CloseHandle(m_fileHandle);
#else
        if (m_data != MAP_FAILED) {
            munmap(m_data, m_size);
        }
#endif
    }
    
    const void* data() const { return m_data; }
    size_t size() const { return m_size; }
    
private:
#ifdef _WIN32
    HANDLE m_fileHandle = INVALID_HANDLE_VALUE;
    HANDLE m_mappingHandle = nullptr;
#endif
    void* m_data = nullptr;
    size_t m_size = 0;
};
```

### Buffered I/O

```cpp
// High-performance buffered writer
class BufferedWriter {
public:
    explicit BufferedWriter(const std::string& filename, size_t bufferSize = 64 * 1024)
        : m_file(filename, std::ios::binary), m_buffer(bufferSize), m_bufferPos(0) {
        if (!m_file) {
            throw std::runtime_error("Failed to open file for writing");
        }
    }
    
    ~BufferedWriter() {
        flush();
    }
    
    void write(const void* data, size_t size) {
        const char* bytes = static_cast<const char*>(data);
        
        while (size > 0) {
            size_t available = m_buffer.size() - m_bufferPos;
            size_t toWrite = std::min(size, available);
            
            std::memcpy(m_buffer.data() + m_bufferPos, bytes, toWrite);
            m_bufferPos += toWrite;
            bytes += toWrite;
            size -= toWrite;
            
            if (m_bufferPos == m_buffer.size()) {
                flushBuffer();
            }
        }
    }
    
    void flush() {
        if (m_bufferPos > 0) {
            flushBuffer();
        }
        m_file.flush();
    }
    
private:
    void flushBuffer() {
        m_file.write(m_buffer.data(), m_bufferPos);
        m_bufferPos = 0;
    }
    
    std::ofstream m_file;
    std::vector<char> m_buffer;
    size_t m_bufferPos;
};
```

## Network Optimization

### Connection Pooling

```cpp
// HTTP connection pool
class ConnectionPool {
public:
    struct Connection {
        std::unique_ptr<HttpClient> client;
        std::chrono::steady_clock::time_point lastUsed;
        bool inUse = false;
    };
    
    explicit ConnectionPool(size_t maxConnections = 10, 
                           std::chrono::seconds maxIdleTime = std::chrono::seconds(300))
        : m_maxConnections(maxConnections), m_maxIdleTime(maxIdleTime) {}
    
    std::shared_ptr<HttpClient> acquireConnection(const std::string& host) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Find available connection
        auto& connections = m_connections[host];
        for (auto& conn : connections) {
            if (!conn.inUse) {
                conn.inUse = true;
                conn.lastUsed = std::chrono::steady_clock::now();
                return std::shared_ptr<HttpClient>(conn.client.get(), 
                    [this, host, &conn](HttpClient*) {
                        this->releaseConnection(host, conn);
                    });
            }
        }
        
        // Create new connection if under limit
        if (connections.size() < m_maxConnections) {
            connections.emplace_back();
            auto& conn = connections.back();
            conn.client = std::make_unique<HttpClient>();
            conn.client->connect(host);
            conn.inUse = true;
            conn.lastUsed = std::chrono::steady_clock::now();
            
            return std::shared_ptr<HttpClient>(conn.client.get(),
                [this, host, &conn](HttpClient*) {
                    this->releaseConnection(host, conn);
                });
        }
        
        return nullptr; // Pool exhausted
    }
    
    void cleanup() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now = std::chrono::steady_clock::now();
        
        for (auto& [host, connections] : m_connections) {
            connections.erase(
                std::remove_if(connections.begin(), connections.end(),
                    [now, this](const Connection& conn) {
                        return !conn.inUse && 
                               (now - conn.lastUsed) > m_maxIdleTime;
                    }),
                connections.end());
        }
    }
    
private:
    void releaseConnection(const std::string& host, Connection& conn) {
        std::lock_guard<std::mutex> lock(m_mutex);
        conn.inUse = false;
        conn.lastUsed = std::chrono::steady_clock::now();
    }
    
    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<Connection>> m_connections;
    size_t m_maxConnections;
    std::chrono::seconds m_maxIdleTime;
};
```

### Batch Operations

```cpp
// Batch HTTP requests
class BatchHttpClient {
public:
    struct Request {
        std::string url;
        std::string method;
        std::string body;
        std::map<std::string, std::string> headers;
    };
    
    struct Response {
        int statusCode;
        std::string body;
        std::map<std::string, std::string> headers;
        std::error_code error;
    };
    
    std::vector<Response> executeBatch(const std::vector<Request>& requests) {
        std::vector<Response> responses(requests.size());
        std::vector<std::future<void>> futures;
        
        // Process requests in parallel
        for (size_t i = 0; i < requests.size(); ++i) {
            futures.emplace_back(std::async(std::launch::async, [this, &requests, &responses, i] {
                responses[i] = executeRequest(requests[i]);
            }));
        }
        
        // Wait for all requests to complete
        for (auto& future : futures) {
            future.wait();
        }
        
        return responses;
    }
    
private:
    Response executeRequest(const Request& request) {
        // Implementation of single request
        // ...
        return {};
    }
};
```

## Concurrency and Threading

### Thread Pool

```cpp
// High-performance thread pool
class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < numThreads; ++i) {
            m_threads.emplace_back([this] {
                workerLoop();
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_condition.notify_all();
        
        for (auto& thread : m_threads) {
            thread.join();
        }
    }
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        auto future = task->get_future();
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stop) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            m_tasks.emplace([task] { (*task)(); });
        }
        
        m_condition.notify_one();
        return future;
    }
    
private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [this] {
                    return m_stop || !m_tasks.empty();
                });
                
                if (m_stop && m_tasks.empty()) {
                    return;
                }
                
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
            
            task();
        }
    }
    
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_stop = false;
};
```

### Lock-Free Data Structures

```cpp
// Lock-free ring buffer
template<typename T, size_t Size>
class LockFreeRingBuffer {
public:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
    
    bool push(const T& item) {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t nextHead = (head + 1) & (Size - 1);
        
        if (nextHead == m_tail.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        m_buffer[head] = item;
        m_head.store(nextHead, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        
        if (tail == m_head.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }
        
        item = m_buffer[tail];
        m_tail.store((tail + 1) & (Size - 1), std::memory_order_release);
        return true;
    }
    
    bool empty() const {
        return m_tail.load(std::memory_order_acquire) == 
               m_head.load(std::memory_order_acquire);
    }
    
    bool full() const {
        size_t head = m_head.load(std::memory_order_acquire);
        return ((head + 1) & (Size - 1)) == m_tail.load(std::memory_order_acquire);
    }
    
private:
    alignas(64) std::atomic<size_t> m_head{0};
    alignas(64) std::atomic<size_t> m_tail{0};
    std::array<T, Size> m_buffer;
};
```

## Compiler Optimizations

### Compilation Flags

```cmake
# CMakeLists.txt optimization settings
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # GCC/Clang optimizations
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -O3                    # Maximum optimization
            -march=native          # Use native CPU instructions
            -mtune=native          # Tune for native CPU
            -flto                  # Link-time optimization
            -ffast-math            # Fast math (if applicable)
            -funroll-loops         # Unroll loops
            -fomit-frame-pointer   # Omit frame pointer
            -DNDEBUG               # Disable assertions
        )
        
        # Link-time optimization
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    # MSVC optimizations
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /O2                    # Maximum optimization
            /Ob2                   # Inline expansion
            /Oi                    # Enable intrinsics
            /Ot                    # Favor speed
            /GL                    # Whole program optimization
            /DNDEBUG               # Disable assertions
        )
        
        target_link_options(${PROJECT_NAME} PRIVATE
            /LTCG                  # Link-time code generation
        )
    endif()
endif()
```

### Profile-Guided Optimization

```bash
# GCC Profile-Guided Optimization
# Step 1: Build with profiling
g++ -O2 -fprofile-generate -o myapp myapp.cpp

# Step 2: Run with representative workload
./myapp < typical_input.txt

# Step 3: Build with profile data
g++ -O2 -fprofile-use -o myapp_optimized myapp.cpp

# MSVC Profile-Guided Optimization
# Step 1: Build with profiling
cl /O2 /GL /LTCG:PGI myapp.cpp

# Step 2: Run with representative workload
myapp.exe < typical_input.txt

# Step 3: Build with profile data
cl /O2 /GL /LTCG:PGO myapp.cpp
```

## Benchmarking

### Micro-benchmarking

```cpp
// Google Benchmark integration
#include <benchmark/benchmark.h>

static void BM_VectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(state.range(0));
        
        for (int i = 0; i < state.range(0); ++i) {
            v.push_back(i);
        }
        
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
    
    state.SetComplexityN(state.range(0));
}

REGISTER_BENCHMARK(BM_VectorPushBack)
    ->Range(8, 8 << 10)
    ->Complexity(benchmark::oN);

// Custom benchmark framework
class SimpleBenchmark {
public:
    template<typename Func>
    static double measureTime(Func func, int iterations = 1000) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            func();
            benchmark::DoNotOptimize(func); // Prevent optimization
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return duration.count() / static_cast<double>(iterations);
    }
    
    template<typename Func>
    static void compareFunctions(const std::string& name1, Func func1,
                               const std::string& name2, Func func2,
                               int iterations = 1000) {
        double time1 = measureTime(func1, iterations);
        double time2 = measureTime(func2, iterations);
        
        printf("\n=== Performance Comparison ===\n");
        printf("%-20s: %.2f ns\n", name1.c_str(), time1);
        printf("%-20s: %.2f ns\n", name2.c_str(), time2);
        printf("Speedup: %.2fx\n", time1 / time2);
    }
};
```

## Performance Monitoring

### Runtime Performance Metrics

```cpp
// Performance metrics collector
class PerformanceMetrics {
public:
    struct Metrics {
        double cpuUsage;
        size_t memoryUsage;
        size_t peakMemoryUsage;
        double averageResponseTime;
        size_t requestCount;
        size_t errorCount;
    };
    
    static PerformanceMetrics& getInstance() {
        static PerformanceMetrics instance;
        return instance;
    }
    
    void recordRequest(std::chrono::nanoseconds responseTime, bool success) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_requestCount++;
        if (!success) {
            m_errorCount++;
        }
        
        // Update moving average
        double newTime = responseTime.count() / 1e6; // Convert to milliseconds
        m_averageResponseTime = (m_averageResponseTime * (m_requestCount - 1) + newTime) / m_requestCount;
    }
    
    void updateMemoryUsage(size_t currentUsage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_memoryUsage = currentUsage;
        m_peakMemoryUsage = std::max(m_peakMemoryUsage, currentUsage);
    }
    
    Metrics getMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return {
            getCpuUsage(),
            m_memoryUsage,
            m_peakMemoryUsage,
            m_averageResponseTime,
            m_requestCount,
            m_errorCount
        };
    }
    
private:
    double getCpuUsage() const {
        // Platform-specific CPU usage calculation
#ifdef _WIN32
        // Windows implementation
        return 0.0;
#else
        // Linux implementation
        return 0.0;
#endif
    }
    
    mutable std::mutex m_mutex;
    size_t m_memoryUsage = 0;
    size_t m_peakMemoryUsage = 0;
    double m_averageResponseTime = 0.0;
    size_t m_requestCount = 0;
    size_t m_errorCount = 0;
};
```

## See Also

- [Development Setup](setup.md)
- [Best Practices](best-practices.md)
- [Architecture Overview](../architecture/overview.md)
- [System Monitoring](../modules/monitoring.md)
- [Network Management](../modules/networking.md)