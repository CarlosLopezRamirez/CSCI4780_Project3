// File: include/inet/buffer.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <string>
#include <vector>

class Buffer {
  public:
    // Constructs a buffer using the given memory and the size of its associated range
    Buffer(void *data, size_t size);

    // Constructs a buffer using the given vector
    template<typename T, typename Allocator>
    Buffer(std::vector<T, Allocator> &data);

    // Constructs a buffer using the given string
    Buffer(std::string data);

    // Constructs a buffer of the given size
    Buffer(size_t size);

    // Makes this buffer non-copyable and non-copy-assignable
    Buffer(Buffer &other) = delete;
    Buffer &operator=(Buffer &other) = delete;

    // Move constructs a Buffer from another buffer
    Buffer(Buffer &&other);

    // Move assigns a Buffer from another buffer
    Buffer &operator=(Buffer &&other);

    // Destructs a buffer
    ~Buffer();

    // Returns a pointer to the underlying data of this buffer
    void *data() const;

    // Returns the size of the underlying data of this buffer
    size_t size() const;

    // Returns a Buffer that is the result of offsetting this buffer by `offset` bytes
    Buffer operator+(size_t offset) const;

  private:
    // A pointer to the underlying data of this buffer
    void *data_;

    // The size of the underlying data of this buffer
    size_t size_;

    // True if this buffer is responsible for allocating and deallocating the buffer
    bool owns_data_;
};