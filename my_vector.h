#ifndef BIGINT_MY_VECTOR_H
#define BIGINT_MY_VECTOR_H

#include <cstddef>
#include <memory>
#include "my_shared_ptr.h"

const size_t max_size = 2;

template<typename T>
class my_vector {
    size_t size_ = 0;
    size_t alloc_ = 0;
    my_shared_ptr<T> data_;
    T small_[max_size];

    void convert() {
        alloc_ = max_size * 2;
        data_ = static_cast<T *>(operator new(alloc_ * sizeof(T)));
        for (size_t i = 0; i < max_size; i++) {
            data_[i] = small_[i];
        }
    }

    void allocation(size_t new_alloc_) {
        T *new_data_ = static_cast<T *>(operator new(new_alloc_ * sizeof(T)));
        for (size_t i = 0; i < size_; i++) {
            new_data_[i] = data_[i];
        }
        my_shared_ptr<T> new_ptr_(new_data_);
        data_ = new_ptr_;
    }

public:

    my_vector() = default;

    my_vector &operator=(const my_vector &rhs) {
        if (this == &rhs) {
            return *this;
        }
        if (rhs.data_.get() == nullptr) {
            for (size_t i = 0; i < max_size; i++) {
                small_[i] = rhs.small_[i];
            }
        } else {
            data_ = rhs.data_;
        }
        size_ = rhs.size_;
        alloc_ = rhs.alloc_;
        return *this;
    }

    const T &operator[](size_t i) const {
        if (data_.get() == nullptr) {
            return small_[i];
        }
        return data_[i];
    }

    T &operator[](size_t i) {
        if (data_.get() == nullptr) {
            return small_[i];
        }
        return data_[i];
    }

    void push_back(T value) {
        if (size_ < max_size) {
            small_[size_++] = value;
            return;
        }
        if (size_ == max_size && data_.get() == nullptr) {
            convert();
        }
        if (size_ == alloc_) {
            alloc_ = alloc_ ? (alloc_ << 1) : 8;
            allocation(alloc_);
        }
        data_[size_++] = value;
    }

    void pop_back() {
        size_--;
    }

    void resize(size_t new_size_) {
        if (new_size_ <= alloc_) {
            size_ = new_size_;
            if (new_size_ <= max_size && data_.get() != nullptr) {
                for (size_t i = 0; i < new_size_; i++) {
                    small_[i] = data_[i];
                }
            }
            return;
        }
        allocation(new_size_);
        for (size_t i = size_; i < new_size_; i++) {
            data_[i] = 0;
        }
        alloc_ = size_ = new_size_;
    }

    T &back() {
        if (data_.get() != nullptr) {
            return data_[size_ - 1];
        }
        return small_[size_ - 1];
    }

    size_t size() const {
        return size_;
    }

    bool empty() {
        return size_ == 0;
    }

    void copy_on_write() {
        if (data_.get() != nullptr && !data_.is_unique()) {
            T *new_data_ = static_cast<T *>(operator new(size_ * sizeof(T)));
            for (size_t i = 0; i < size_; ++i) {
                size_t t = data_[i];
                new_data_[i] = t;
            }
            my_shared_ptr<T> new_ptr_(new_data_);
            data_ = new_ptr_;
            alloc_ = size_;
        }
    };
};

#endif //BIGINT_MY_VECTOR_H
