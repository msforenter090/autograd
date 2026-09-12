#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace autograd {

template<typename Type> class Tape;
template<typename Type> class Variable;
template<typename Type> class Gradient;

namespace detail {
    template<typename Type> struct Ops;
}

/* One entry on the tape: the local partial derivatives of this node's value with
   respect to each of its (at most two) inputs, and where those inputs sit on the
   tape. Unused slots hold a zero weight and point at the node itself, so the
   backward pass can run the same two lines on every node without branching. */
template<typename Type>
class Node {
    friend class Tape<Type>;
    friend class Variable<Type>;

    Node(Type weight0, std::size_t dep0, Type weight1, std::size_t dep1)
        : mWeights{weight0, weight1}, mDeps{dep0, dep1} { }

    Type mWeights[2];
    std::size_t mDeps[2];
};

/* Append-only record of every operation performed on the variables bound to it. */
template<typename Type>
class Tape {
    static_assert(std::is_floating_point<Type>::value,
                  "autograd::Tape requires a floating-point type");

    friend class Variable<Type>;
    friend class Gradient<Type>;
    friend struct detail::Ops<Type>;

public:
    Tape() = default;

    /* Variables refer to their tape by reference and by index, so the tape must
       not be copied or moved out from under them. Moving would leave the source
       with an empty node vector while live variables still pointed into it. */
    Tape(const Tape &) = delete;
    Tape &operator=(const Tape &) = delete;
    Tape(Tape &&) = delete;
    Tape &operator=(Tape &&) = delete;

    Variable<Type> variable(Type value);

    std::size_t size() const { return mNodes.size(); }

private:
    std::size_t leaf() {
        const std::size_t index = mNodes.size();
        mNodes.push_back(Node<Type>(Type(0), index, Type(0), index));
        return index;
    }

    std::size_t unary(Type weight, std::size_t dep) {
        const std::size_t index = mNodes.size();
        mNodes.push_back(Node<Type>(weight, dep, Type(0), index));
        return index;
    }

    std::size_t binary(Type weight0, std::size_t dep0, Type weight1, std::size_t dep1) {
        const std::size_t index = mNodes.size();
        mNodes.push_back(Node<Type>(weight0, dep0, weight1, dep1));
        return index;
    }

    std::vector<Node<Type>> mNodes;
};

/* A value that records what was done to it. */
template<typename Type>
class Variable {
    friend class Tape<Type>;
    friend class Gradient<Type>;
    friend struct detail::Ops<Type>;

public:
    Variable(const Variable &other)
        : mTape(other.mTape),
          mValue(other.mValue),
          mIndex(other.mTape.unary(Type(1), other.mIndex)) { }

    Variable(Variable &&) = default;

    Variable &operator=(const Variable &other) {
        if (this != &other) {
            requireSameTape(other);
            mValue = other.mValue;
            mIndex = mTape.unary(Type(1), other.mIndex);
        }
        return *this;
    }

    Variable &operator=(Variable &&other) {
        requireSameTape(other);
        mValue = other.mValue;
        mIndex = other.mIndex;
        return *this;
    }

    Variable &operator+=(const Variable &other) { return *this = *this + other; }
    Variable &operator-=(const Variable &other) { return *this = *this - other; }
    Variable &operator*=(const Variable &other) { return *this = *this * other; }
    Variable &operator/=(const Variable &other) { return *this = *this / other; }

    Variable &operator+=(Type scalar) { return *this = *this + scalar; }
    Variable &operator-=(Type scalar) { return *this = *this - scalar; }
    Variable &operator*=(Type scalar) { return *this = *this * scalar; }
    Variable &operator/=(Type scalar) { return *this = *this / scalar; }

    Type value() const { return mValue; }

    /* Partial derivatives of this variable with respect to every variable it was
       built from, computed in a single reverse pass over the tape. */
    Gradient<Type> gradient() const;

private:
    Variable(Tape<Type> &tape, Type value, std::size_t index)
        : mTape(tape), mValue(value), mIndex(index) { }

    void requireSameTape(const Variable &other) const {
        if (&mTape != &other.mTape) {
            throw std::invalid_argument("autograd::Variables are not from the same autograd::Tape");
        }
    }

    Tape<Type> &mTape;
    Type mValue;
    std::size_t mIndex;
};

/* The result of one backward pass: an adjoint per tape node, indexed the same way
   the tape is, so a lookup is a subscript. */
template<typename Type>
class Gradient {
    friend class Variable<Type>;

public:
    Type withRespectTo(const Variable<Type> &variable) const {
        if (&mTape != &variable.mTape) {
            throw std::invalid_argument("autograd::Variable is not from this autograd::Gradient's tape");
        }
        /* Variables recorded after the differentiated output are not ancestors of
           it, so the derivative is zero. They sit past the end of the adjoint
           array, which is sized to the output's node only. */
        if (variable.mIndex >= mGradients.size()) {
            return Type(0);
        }
        return mGradients[variable.mIndex];
    }

    std::size_t size() const { return mGradients.size(); }

private:
    Gradient(const Tape<Type> &tape, std::vector<Type> &&gradients)
        : mTape(tape), mGradients(std::move(gradients)) { }

    const Tape<Type> &mTape;
    std::vector<Type> mGradients;
};

template<typename Type>
Variable<Type> Tape<Type>::variable(Type value) {
    return Variable<Type>(*this, value, leaf());
}

template<typename Type>
Gradient<Type> Variable<Type>::gradient() const {
    /* Every node's dependencies have an index no greater than its own, so adjoints
       only ever flow toward lower indices. Nothing above the seed can become
       nonzero, which makes the tail of the tape both unnecessary to allocate and
       unnecessary to walk. */
    const std::size_t count = mIndex + 1;

    std::vector<Type> gradients(count, Type(0));
    gradients[mIndex] = Type(1);

    for (std::size_t i = count; i-- > 0;) {
        const Node<Type> &node = mTape.mNodes[i];

        /* Final by construction: everything that could add to slot i has already
           been visited. Reading it once also keeps the two writes below correct
           when both dependencies name the same slot, as in x + x. */
        const Type adjoint = gradients[i];

        gradients[node.mDeps[0]] += node.mWeights[0] * adjoint;
        gradients[node.mDeps[1]] += node.mWeights[1] * adjoint;
    }

    return Gradient<Type>(mTape, std::move(gradients));
}

namespace detail {

/* Single point of access to the private tape and variable internals, so the
   operations below stay ordinary free functions with no friendship of their own. */
template<typename Type>
struct Ops {
    static Variable<Type> unary(const Variable<Type> &operand, Type value, Type weight) {
        return Variable<Type>(operand.mTape, value, operand.mTape.unary(weight, operand.mIndex));
    }

    static Variable<Type> binary(const Variable<Type> &lhs, const Variable<Type> &rhs,
                                 Type value, Type weightLhs, Type weightRhs) {
        lhs.requireSameTape(rhs);
        return Variable<Type>(lhs.mTape, value,
                              lhs.mTape.binary(weightLhs, lhs.mIndex, weightRhs, rhs.mIndex));
    }
};

} // namespace detail

template<typename Type>
Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
    return detail::Ops<Type>::binary(lhs, rhs, lhs.value() + rhs.value(), Type(1), Type(1));
}

template<typename Type>
Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
    return detail::Ops<Type>::binary(lhs, rhs, lhs.value() - rhs.value(), Type(1), Type(-1));
}

template<typename Type>
Variable<Type> operator*(const Variable<Type> &lhs, const Variable<Type> &rhs) {
    return detail::Ops<Type>::binary(lhs, rhs, lhs.value() * rhs.value(), rhs.value(), lhs.value());
}

template<typename Type>
Variable<Type> operator/(const Variable<Type> &lhs, const Variable<Type> &rhs) {
    const Type inverse = Type(1) / rhs.value();
    return detail::Ops<Type>::binary(lhs, rhs, lhs.value() * inverse,
                                     inverse, -lhs.value() * inverse * inverse);
}

template<typename Type>
Variable<Type> operator+(const Variable<Type> &operand, Type scalar) {
    return detail::Ops<Type>::unary(operand, operand.value() + scalar, Type(1));
}

template<typename Type>
Variable<Type> operator+(Type scalar, const Variable<Type> &operand) {
    return operand + scalar;
}

template<typename Type>
Variable<Type> operator-(const Variable<Type> &operand, Type scalar) {
    return detail::Ops<Type>::unary(operand, operand.value() - scalar, Type(1));
}

template<typename Type>
Variable<Type> operator-(Type scalar, const Variable<Type> &operand) {
    return detail::Ops<Type>::unary(operand, scalar - operand.value(), Type(-1));
}

template<typename Type>
Variable<Type> operator*(const Variable<Type> &operand, Type scalar) {
    return detail::Ops<Type>::unary(operand, operand.value() * scalar, scalar);
}

template<typename Type>
Variable<Type> operator*(Type scalar, const Variable<Type> &operand) {
    return operand * scalar;
}

template<typename Type>
Variable<Type> operator/(const Variable<Type> &operand, Type scalar) {
    const Type inverse = Type(1) / scalar;
    return detail::Ops<Type>::unary(operand, operand.value() * inverse, inverse);
}

template<typename Type>
Variable<Type> operator/(Type scalar, const Variable<Type> &operand) {
    const Type inverse = Type(1) / operand.value();
    return detail::Ops<Type>::unary(operand, scalar * inverse, -scalar * inverse * inverse);
}

template<typename Type>
Variable<Type> operator+(const Variable<Type> &operand) {
    return detail::Ops<Type>::unary(operand, operand.value(), Type(1));
}

template<typename Type>
Variable<Type> operator-(const Variable<Type> &operand) {
    return detail::Ops<Type>::unary(operand, -operand.value(), Type(-1));
}

template<typename Type>
Variable<Type> exp(const Variable<Type> &operand) {
    const Type value = std::exp(operand.value());
    return detail::Ops<Type>::unary(operand, value, value);
}

template<typename Type>
Variable<Type> log(const Variable<Type> &operand) {
    return detail::Ops<Type>::unary(operand, std::log(operand.value()), Type(1) / operand.value());
}

template<typename Type>
Variable<Type> sqrt(const Variable<Type> &operand) {
    const Type value = std::sqrt(operand.value());
    return detail::Ops<Type>::unary(operand, value, Type(0.5) / value);
}

template<typename Type>
Variable<Type> pow(const Variable<Type> &operand, Type exponent) {
    return detail::Ops<Type>::unary(operand, std::pow(operand.value(), exponent),
                                    exponent * std::pow(operand.value(), exponent - Type(1)));
}

template<typename Type>
Variable<Type> tanh(const Variable<Type> &operand) {
    const Type value = std::tanh(operand.value());
    return detail::Ops<Type>::unary(operand, value, Type(1) - value * value);
}

template<typename Type>
Variable<Type> sigmoid(const Variable<Type> &operand) {
    const Type value = Type(1) / (Type(1) + std::exp(-operand.value()));
    return detail::Ops<Type>::unary(operand, value, value * (Type(1) - value));
}

} // namespace autograd
