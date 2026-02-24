#pragma once

namespace autograd {
    template<typename Type>
    class Variable {
    public:
        using TypeValue = Type;
        TypeValue mValue;

        Variable(const TypeValue& value) : mValue(value) { }
        Variable(const Variable& other) : mValue(other.mValue) { }
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
}
