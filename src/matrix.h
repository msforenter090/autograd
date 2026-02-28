#pragma once

#include <cstring>

template<typename Type>
class Matrix {
public:
    using TypeValue = Type;
    using Reference = TypeValue&;
    using Pointer = TypeValue*;

    Matrix(int width, int height) : mWidth(width), mHeight(height), mData(nullptr) {
        mData = new TypeValue[mWidth * mHeight];
    }

    Matrix(const Matrix<TypeValue> &other) : mWidth(other.mWidth), mHeight(other.mHeight) {
        mData = new TypeValue[mWidth * mHeight];
        // assume same dimensions and this and rhs are not the same object
        std::memcpy(mData, other.mData, sizeof(TypeValue) * other.mWidth * other.mHeight);
    }

    Matrix<TypeValue>& operator=(const Matrix<TypeValue> &rhs) {
        // assume same dimensions and this and rhs are not the same object
        std::memcpy(mData, rhs.mData, sizeof(TypeValue) * mWidth * mHeight);
        return *this;
    }

    // Operators
    Reference operator()(int i, int j) {
        return mData[i * mWidth + j];
    }

    ~Matrix() {
        delete mData;
        mData = nullptr;
    }

    int width() const { return mWidth; }
    int height() const { return mHeight; }

private:
    int mWidth;
    int mHeight;
    TypeValue *mData;
};

template<typename T>
Matrix<T> operator+(const Matrix<T> &lhs, const Matrix<T> &rhs) {
    return Matrix<T>(lhs.width(), lhs.height());
}

template<typename T>
Matrix<T> operator-(const Matrix<T> &lhs, const Matrix<T> &rhs) {
    return Matrix<T>(lhs.width(), lhs.height());
}

