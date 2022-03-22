// File: buffer.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/inet/buffer.hpp"

Buffer::Buffer(void *data, size_t size) : owns_data_(false) {
    data_ = data;
    size_ = size;
}

template<typename T, typename Allocator>
Buffer::Buffer(std::vector<T, Allocator> &data) : owns_data_(false) {
    data_ = data.data();
    size_ = sizeof(T) * data.size();
}

Buffer::Buffer(std::string data) : owns_data_(false) {
    data_ = data.data();
    size_ = data.size();
}

Buffer::Buffer(size_t size) : owns_data_(true) {
    data_ = new char[size];
    size_ = size;
}

Buffer::Buffer(Buffer &&other) {
    *this = std::move(other);
}

Buffer &Buffer::operator=(Buffer &&other) {
    if (this == &other) return *this;

    owns_data_ = other.owns_data_;
    data_ = other.data_;
    size_ = other.size_;

    other.data_ = nullptr;
    other.size_ = 0;

    return *this;
}

Buffer::~Buffer() {
    if (owns_data_) delete[] (char *)data_;
}

void *Buffer::data() const {
    return data_;
}

size_t Buffer::size() const {
    return size_;
}

Buffer Buffer::operator+(size_t offset) const {
    return Buffer((char *)data_ + offset, size_ - offset);
}