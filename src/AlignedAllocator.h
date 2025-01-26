#ifndef ALIGNED_ALLOCATOR_H
#define ALIGNED_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <immintrin.h>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

// Custom aligned allocator
template <typename T, std::size_t Alignment>
class AlignedAllocator {
  public:
    using value_type = T;
    static constexpr std::size_t alignment = Alignment;

    using pointer = T*;
    using const_pointer = const T*;

    using void_pointer = void*;
    using const_void_pointer = const void*;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <class U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    AlignedAllocator() noexcept {}

    template <class U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    ~AlignedAllocator() {}

    T* allocate(std::size_t n) {
        std::size_t alignment = Alignment;
        if (alignment < alignof(void*)) {
            alignment = alignof(void*);
        }

        void* ptr = nullptr;

#if defined(_MSC_VER)
        ptr = _aligned_malloc(n * sizeof(T), alignment);
        if (!ptr)
            throw std::bad_alloc();
#elif defined(__MINGW32__)
        ptr = __mingw_aligned_malloc(n * sizeof(T), alignment);
        if (!ptr)
            throw std::bad_alloc();
#elif defined(__APPLE__) || defined(__linux__)
        if (posix_memalign(&ptr, alignment, n * sizeof(T)) != 0) {
            ptr = nullptr;
            throw std::bad_alloc();
        }
#else
        ptr = std::aligned_alloc(alignment, n * sizeof(T));
        if (!ptr)
            throw std::bad_alloc();
#endif

        return reinterpret_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t) noexcept {
        if (!p)
            return;

#if defined(_MSC_VER)
        _aligned_free(p);
#elif defined(__MINGW32__)
        __mingw_aligned_free(p);
#elif defined(__APPLE__) || defined(__linux__)
        free(p);
#else
        std::free(p);
#endif
    }

    // Optional: Max size
    size_type max_size() const noexcept {
        return (std::numeric_limits<size_type>::max)() / sizeof(T);
    }

    // Optional: Construct and destroy
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new ((void*)p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    // Comparison operators
    bool operator==(const AlignedAllocator& other) const noexcept {
        return Alignment == other.alignment;
    }

    bool operator!=(const AlignedAllocator& other) const noexcept {
        return !(*this == other);
    }
};

using AlignedFloatVector = std::vector<float, AlignedAllocator<float, 32>>;
using AlignedInt32Vector = std::vector<int32_t, AlignedAllocator<int32_t, 32>>;

#endif