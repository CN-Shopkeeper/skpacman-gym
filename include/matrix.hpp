#pragma once

#include "pch.hpp"

template <typename T>
class Matrix final {
   private:
    std::unique_ptr<T[]> data_;
    int w_;
    int h_;

    int flagCount_;

    std::optional<SDL_Point> highlight_;

   public:
    Matrix(int w, int h)
        : data_(std::unique_ptr<T[]>(new T[w * h])),
          w_(w),
          h_(h),
          flagCount_(0),
          highlight_(std::nullopt) {}

    void Fill(const T& value) {
        for (int i = 0; i < w_ * h_; i++) {
            GetByIndex(i) = value;
        }
    }

    T& Get(int x, int y) { return data_.get()[x + y * w_]; }

    const T& Get(int x, int y) const { return data_.get()[x + y * w_]; }

    T& GetByIndex(int idx) { return data_.get()[idx]; }

    int MaxSize() const { return w_ * h_; }

    int Width() const { return w_; }

    int Height() const { return h_; }

    bool IsIn(int x, int y) const {
        return x >= 0 && x < w_ && y >= 0 && y < h_;
    }

    void SetHighlight(SDL_Point p) {
        if (IsIn(p.x, p.y)) {
            highlight_ = p;
        } else {
            highlight_ = std::nullopt;
        }
    }

    void RemoveHeight() { highlight_ = std::nullopt; }

    std::optional<SDL_Point> GetHightlight() const { return highlight_; }

    int GetFlagCount() const { return flagCount_; }

    void AddFlagCount(int offset) { flagCount_ += offset; }
};