#pragma once
#include "tape.h"

namespace autograd {
    class Tape;

    template<typename Type>
    class Variable {
    public:
        using TypeValue = Type;
        Variable(Tape &tape, const TypeValue& value) : mTape(tape), mValue(value) { }
        Variable(Tape &tape, const Variable& other) : mTape(tape), mValue(other.mValue) { }

        friend Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue + rhs.mValue;
//          lhs.push_back(nullptr);
            return Variable(lhs.mTape, result);
        }

        friend Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue - rhs.mValue;
            return Variable(lhs.mTape, result);
        }

    private:
        TypeValue mValue;
        Tape &mTape;
    };
}
