#pragma once

template<typename T, int W, int H>
class Matrix {
public:
    Matrix() : mData() { }

    Matrix<T, W, H>& operator=(const Matrix<T, W, H>& rhs) {
        return *this;
    }

private:
    using TypeValue = T;
    const int Width = W;
    const int Height = H;
    TypeValue mData[Width * Height];
};

template<typename T, int W, int H>
Matrix<T, W, H> operator+(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
    return Matrix<T, H, W>(nullptr);
}

template<typename T, int W, int H>
Matrix<T, W, H> operator-(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
    return Matrix<T, H, W>(nullptr);
}

