#pragma once
#include <stdexcept>
#include "lab2_files/DynamicArray.hpp"

template <typename T>
class SlidingCache {
public:
    explicit SlidingCache(int capacity) : capacity_(capacity), start_index_(-1), count_(0) {
        if (capacity <= 0) throw std::invalid_argument("Capacity must be > 0");
        buffer_ = new DynamicArray<T>(capacity);
    }

    ~SlidingCache() {
        delete buffer_;
    }

    bool IsEmpty() const { return count_ == 0; }
    int GetCount() const { return count_; }
    int GetCapacity() const { return capacity_; }

    void Clear() {
        start_index_ = -1;
        count_ = 0;
    }

    bool Contains(int index) const {
        if (IsEmpty()) return false;
        return index >= start_index_ && index < start_index_ + count_;
    }

    const T& Get(int index) const {
        if (!Contains(index)) throw std::out_of_range("Index not in sliding cache");
        int internal_index = index - start_index_;
        return buffer_->Get(internal_index);
    }

    void Push(const T& item, int global_index) {
        if (IsEmpty()) {
            start_index_ = global_index;
            buffer_->Set(0, item);
            count_ = 1;
        } else if (global_index == start_index_ + count_) {
            // Добавление в конец окна
            if (count_ < capacity_) {
                buffer_->Set(count_, item);
                count_++;
            } else {
                // Окно переполнено, сдвигаем влево
                for (int i = 1; i < capacity_; ++i) {
                    buffer_->Set(i - 1, buffer_->Get(i));
                }
                buffer_->Set(capacity_ - 1, item);
                start_index_++;
            }
        } else {
            // Если индекс оторван от текущего окна, начинаем окно заново
            Clear();
            start_index_ = global_index;
            buffer_->Set(0, item);
            count_ = 1;
        }
    }

private:
    DynamicArray<T>* buffer_;
    int capacity_;
    int start_index_;
    int count_;
};
