#ifndef BIGINT_MY_SHARED_PTR_H
#define BIGINT_MY_SHARED_PTR_H

#include <cstddef>
#include <cassert>

template<typename T>
struct ptr_body {
    T *ptr;
    size_t cnt;

    ptr_body(T *ptr_) {
        ptr = ptr_;
        cnt = 1;
    }

    ~ptr_body() {
        operator delete(ptr);
    }

};

template<typename T>
class my_shared_ptr {
    ptr_body<T> *ptr = nullptr;
public:
    my_shared_ptr() = default;

    my_shared_ptr(T *ptr_) {
        if (ptr_ != nullptr) {
            ptr = new ptr_body<T>(ptr_);
        } else {
            ptr = nullptr;
        }
    }

    ~my_shared_ptr() {
        if (ptr != nullptr) {
            ptr->cnt--;
            if (ptr->cnt == 0) {
                delete ptr;
            }
        }
    }

    my_shared_ptr(my_shared_ptr const &rhs) = delete;

    my_shared_ptr(my_shared_ptr &rhs) = delete;

    my_shared_ptr &operator=(const my_shared_ptr &rhs) {
        if (ptr == rhs.ptr) {
            return *this;
        }
        if (rhs.ptr == nullptr) {
            if (ptr != nullptr) {
                if (!--ptr->cnt) {
                    delete ptr;
                }
                ptr = nullptr;
            }
        } else {
            if (ptr != nullptr) {
                if (!--ptr->cnt) {
                    delete ptr;
                }
            }
            ptr = rhs.ptr;
            ptr->cnt++;
        }
        return *this;
    }

    T &operator[](size_t i) {
        return ptr->ptr[i];
    }

    T &operator[](size_t i) const {
        return ptr->ptr[i];
    }

    T *get() const {
        return (ptr == nullptr) ? nullptr : ptr->ptr;
    }

    bool is_unique() {
        return ptr->cnt == 1;
    }

};

#endif //BIGINT_MY_SHARED_PTR_H
