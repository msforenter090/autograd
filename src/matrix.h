#pragma once
#include <cstring>
#include <memory>

template<typename Type>
class MatrixImplementation {
public:
    using TypeValue = Type;
    using Reference = TypeValue&;
    using Pointer = TypeValue*;

    MatrixImplementation(int width, int height) : mWidth(width), mHeight(height), mData(nullptr) {
        mData = new TypeValue[mWidth * mHeight];
    }

    MatrixImplementation(const MatrixImplementation<TypeValue> &other) : mWidth(other.mWidth), mHeight(other.mHeight) {
        mData = new TypeValue[mWidth * mHeight];
        // assume same dimensions and this and rhs are not the same object
        std::memcpy(mData, other.mData, sizeof(TypeValue) * other.mWidth * other.mHeight);
    }

    MatrixImplementation<TypeValue>& operator=(const MatrixImplementation<TypeValue> &rhs) {
        // assume same dimensions and this and rhs are not the same object
        std::memcpy(mData, rhs.mData, sizeof(TypeValue) * mWidth * mHeight);
        return *this;
    }

    // Operators
    Reference operator()(int i, int j) {
        return mData[i * mWidth + j];
    }

    ~MatrixImplementation() {
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
MatrixImplementation<T> operator+(const MatrixImplementation<T> &lhs, const MatrixImplementation<T> &rhs) {
    return MatrixImplementation<T>(lhs.width(), lhs.height());
}

template<typename T>
MatrixImplementation<T> operator-(const MatrixImplementation<T> &lhs, const MatrixImplementation<T> &rhs) {
    return MatrixImplementation<T>(lhs.width(), lhs.height());
}
