#include <iostream>
#include "tape.h"

// Node In a computation tree.
//template<typename T, int W, int H>
//class Matrix {
//public:
//    using TypeValue = T;
//    const int Width = W;
//    const int Height = H;
//
//    TypeValue *mData;
//    Matrix(TypeValue* data) : mData(data) { }
//
//    Matrix<T, W, H>& operator=(const Matrix<T, W, H>& rhs) {
//    return *this;
//}
//};
//
//template<typename T, int W, int H>
//Matrix<T, W, H> operator+(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
//    return Matrix<T, H, W>(nullptr);
//}
//
//template<typename T, int W, int H>
//Matrix<T, W, H> operator-(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
//    return Matrix<T, H, W>(nullptr);
//}


int main() {
    autograd::Tape tape;
    auto taped_a = tape.variable(12);
    auto taped_b = tape.variable(12);
    auto taped_c = taped_a + taped_b;
    return 0;
}
