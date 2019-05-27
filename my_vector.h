#ifndef BIGINT_MY_VECTOR_H
#define BIGINT_MY_VECTOR_H

#include <cstddef>
#include <memory>
#include "my_shared_ptr.h"

template<typename T>
class my_vector {
    size_t size_ = 0;
    size_t alloc_ = 0;
    my_shared_ptr<T> data_;

public:

    my_vector() = default;

//    ~my_vector() = default;

    my_vector(my_vector const &) = delete;

    my_vector &operator=(const my_vector &rhs) {
        if (this == &rhs) {
            return *this;
        }
        data_ = rhs.data_;
        size_ = rhs.size_;
        alloc_ = rhs.alloc_;
        return *this;
    }

    const T &operator[](size_t i) const {
        return data_[i];
    }

    T &operator[](size_t i) {
        return data_[i];
    }

    void push_back(T value) {
        size_t old = size_;
        if (size_ == alloc_) {
            alloc_ = alloc_ ? (alloc_ << 1) : 8;
            resize(alloc_);
        }
        data_[old++] = value;
        size_ = old;
    }

    void pop_back() {
        size_--;
//        data_[size_].~T();
    }

    void resize(size_t new_size_) {
        if (new_size_ <= alloc_) {
            size_ = new_size_;
            return;
        }
        T *new_data_ = static_cast<T *>(operator new(new_size_ * sizeof(T)));
        for (size_t i = 0; i < size_; i++) {
            new(new_data_ + i) T(data_[i]);
        }
        for (size_t i = size_; i < new_size_; i++) {
            new(new_data_ + i) T(0);
        }
        my_shared_ptr<T> new_ptr_(new_data_);
        data_ = new_ptr_;
        alloc_ = size_ = new_size_;
    }

    T &back() {
        return data_[size_ - 1];
    }

    size_t size() const {
        return size_;
    }

    bool empty() {
        return size_ == 0;
    }

    void copy_on_write() {
        assert(size_ <= alloc_);
        T *new_data_ = static_cast<T *>(operator new(size_ * sizeof(T)));
        for (size_t i = 0; i < size_; ++i) {
            size_t t = data_[i];
            new_data_[i] = t;
        }
        my_shared_ptr<T> new_ptr_(new_data_);
        data_ = new_ptr_;
        alloc_ = size_;
    };
};

#endif //BIGINT_MY_VECTOR_H
