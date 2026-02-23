#include <iostream>
#include <string>
#include <vector>

// Node In a computation tree.
class Node {
public:
    Node *parent;
};

class Tape {
public:
    std::vector<Node*> tape;

    void push_back(Node * const node) {
        tape.emplace_back(node);
    }
};

// Node In a computation tree.


// Class we are tracking.
// Matrix for now.
template<typename Type>
class Variable {
public:
    using TypeValue = Type;
    TypeValue mValue;
    Variable(TypeValue value) : mValue(value) { }
};

template<typename Type>
Variable<Type> operator+(const Variable<Type>& lhs, const Variable<Type>& rhs) {
    Type result = lhs.mValue + rhs.mValue;
    return Variable(result);
}

template<typename Type>
Variable<Type> operator-(const Variable<Type>& lhs, const Variable<Type>& rhs) {
    Type result = lhs.mValue - rhs.mValue;
    return Variable(result);
}

template<typename T, int W, int H>
class Matrix {
public:
    using TypeValue = T;
    const int Width = W;
    const int Height = H;

    TypeValue *mData;
    Matrix(TypeValue* data) : mData(data) { }

    Matrix<T, W, H>& operator=(const Matrix<T, W, H>& rhs) {
    return *this;
}
};

template<typename T, int W, int H>
Matrix<T, W, H> operator+(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
    return Matrix<T, H, W>(nullptr);
}

template<typename T, int W, int H>
Matrix<T, W, H> operator-(const Matrix<T, W, H>& lhs, const Matrix<T, W, H>& rhs) {
    return Matrix<T, H, W>(nullptr);
}


int main() {
    Variable<Matrix<float, 3, 3>> a(Matrix<float, 3, 3>(nullptr));
    Variable<Matrix<float, 3, 3>> b(Matrix<float, 3, 3>(nullptr));
    Variable<Matrix<float, 3, 3>> c(Matrix<float, 3, 3>(nullptr));
    c = a + b;
    return 0;
}
