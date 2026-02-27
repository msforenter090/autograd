#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

namespace autograd {
    class Node;
    class Tape;

    class Node {
        Node *parent;
    };

    template<typename Type>
    class Variable {
    public:
        using TypeValue = Type;

        Variable(Tape &tape) : mTape(tape), mValue() { }
        Variable(const Variable& other) : mTape(other.mTape), mValue() { }

        friend Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue + rhs.mValue;
            return Variable(lhs.mTape);
        }

        friend Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue - rhs.mValue;
            return Variable(lhs.mTape);
        }

    private:
        TypeValue mValue;
        Tape &mTape;
    };


    class Tape {
    public:
        Tape() { }

        template<typename T>
        Variable<T> variable(const T& value) {
            return Variable<T>(*this);
        }

        void push_back(Node *node) {
            mTape.emplace_back(node);
            std::cout << "Hello" << std::endl;
        }

    private:
        std::vector<Node*> mTape;
    };
}
