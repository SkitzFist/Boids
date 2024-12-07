#ifndef BOIDS_THREAD_VECTOR_H
#define BOIDS_THREAD_VECTOR_H

#include <cstring>
#include <mutex>
#include <type_traits>
#include <utility>

template <class T>
class ThreadVector {
  public:
    ThreadVector()
        : m_data(nullptr), m_size(0), m_capacity(0) {}

    ~ThreadVector() {
        reset();
    }

    void initialize(std::size_t capacity) {
        reset();
        m_capacity = capacity;
        m_data = new T[m_capacity]{};
        m_size = 0;
    }

    // Resets the object to a zeroed state
    void reset() {
        delete[] m_data; // Safely releases memory
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    // Copy constructor
    ThreadVector(const ThreadVector<T>& other)
        : m_data(nullptr), m_size(0), m_capacity(0) {
        copy_from(other);
    }

    // Copy assignment operator
    ThreadVector<T>& operator=(const ThreadVector<T>& other) {
        if (this != &other) {
            reset();
            copy_from(other);
        }
        return *this;
    }

    // Move constructor
    ThreadVector(ThreadVector<T>&& other) noexcept
        : m_data(nullptr), m_size(0), m_capacity(0) {
        move_from(std::move(other));
    }

    // Move assignment operator
    ThreadVector<T>& operator=(ThreadVector<T>&& other) noexcept {
        if (this != &other) {
            reset();
            move_from(std::move(other));
        }
        return *this;
    }

    const T& operator[](std::size_t i) const {
        return m_data[i];
    }

    T& operator[](std::size_t i) {
        return m_data[i];
    }

    void push_back(const T& item) {
        std::scoped_lock lock(m_mutex);
        ensure_capacity();
        m_data[m_size++] = item;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        std::scoped_lock lock(m_mutex);
        ensure_capacity();
        new (&m_data[m_size++]) T(std::forward<Args>(args)...);
    }

    std::size_t size() const noexcept {
        std::scoped_lock lock(m_mutex);
        return m_size;
    }

    std::size_t capacity() const noexcept {
        std::scoped_lock lock(m_mutex);
        return m_capacity;
    }

    void clear() {
        m_size = 0;
    }

  private:
    T* m_data;
    std::size_t m_size;
    std::size_t m_capacity;
    mutable std::mutex m_mutex;

    void ensure_capacity() {
        if (m_size == m_capacity) {
            resize(m_capacity == 0 ? 1 : m_capacity * 2);
        }
    }

    void resize(std::size_t new_capacity) {
        T* new_data = new T[new_capacity]{};

        if constexpr (std::is_trivially_copyable<T>::value) {
            std::memcpy(new_data, m_data, sizeof(T) * m_size);
        } else {
            for (std::size_t i = 0; i < m_size; ++i) {
                new (&new_data[i]) T(std::move(m_data[i]));
                m_data[i].~T();
            }
        }

        delete[] m_data;

        m_data = new_data;
        m_capacity = new_capacity;
    }

    void copy_from(const ThreadVector<T>& other) {
        initialize(other.m_capacity);
        m_size = other.m_size;

        if constexpr (std::is_trivially_copyable<T>::value) {
            std::memcpy(m_data, other.m_data, sizeof(T) * m_size);
        } else {
            for (std::size_t i = 0; i < m_size; ++i) {
                new (&m_data[i]) T(other.m_data[i]);
            }
        }
    }

    void move_from(ThreadVector<T>&& other) {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }
};

#endif
