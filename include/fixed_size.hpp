#pragma once
#include <array>
#include <deque>
#include <iostream>
#include <vector>

template <typename T, std::size_t MaxSize>
class FixedSizeQueue {
   public:
    void push(const T& value) {
        if (queue_.size() >= MaxSize) {
            queue_.pop_front();
        }
        queue_.push_back(value);
    }

    const T& front() const { return queue_.front(); }

    std::size_t size() const { return queue_.size(); }

    const T& Get(int index) const { return queue_.at(index); }

    bool IsEqual(const std::array<T, MaxSize> vec) const {
        if (vec.size() != size()) {
            return false;
        }
        for (int i = 0; i < size(); i++) {
            if (vec[i] != queue_.at(i)) {
                return false;
            }
        }
        return true;
    }

   private:
    std::deque<T> queue_;
};

template <typename T, std::size_t MaxSize>
inline std::ostream& operator<<(std::ostream& stream,
                                const FixedSizeQueue<T, MaxSize>& queue) {
    stream << "[";
    for (int i = 0; i < queue.size(); i++) {
        stream << queue.Get(i) << ",";
    }
    stream << "]";
    return stream;
}

// class temp {
//     std::array<char, 10> chars;
// };

template <typename T, std::size_t MaxSize>
class FixedSizeStack {
   public:
    void push(const T& value) {
        if (!(stack_.size() > MaxSize)) {
            stack_.push_back(value);
        }
    }

    const T& front() const { return stack_.front(); }

    void Pop() {
        if (!stack_.empty()) {
            stack_.pop_back();
        }
    }

    std::size_t Size() const { return stack_.size(); }

    std::string ToString() const { return std::string("ToString"); }

    void Clear() { stack_.clear(); }

   private:
    std::deque<T> stack_;
};

template <std::size_t MaxSize>
class FixedSizeStack<char, MaxSize> {
   public:
    void push(const char& value) {
        if (!(stack_.size() > MaxSize)) {
            stack_.push_back(value);
        }
    }

    const char& Front() const { return stack_.front(); }

    std::size_t Size() const { return stack_.size(); }

    void Pop() {
        if (!stack_.empty()) {
            stack_.pop_back();
        }
    }

    std::string ToString() const {
        std::string content;
        for (int i = 0; i < stack_.size(); i++) {
            content += stack_.at(i);
        }
        return content;
    }

    void Clear() { stack_.clear(); }

   private:
    std::deque<char> stack_;
};