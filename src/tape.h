#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

namespace autograd {
    class Node;
    class Tape;

    class Node {
    public:
        Node(Node *parent = nullptr) : mParent(parent) { }

    public:
        Node *mParent;
    };

    template<typename Type>
    class Variable {
    public:
        using TypeValue = Type;

        Variable(Tape &tape, Node *node) : mTape(tape), mNode(node), mValue() { }
        Variable(const Variable &other) : mTape(other.mTape), mValue() { }

        friend Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue + rhs.mValue;
            auto newNode = new Node();
            lhs.mNode->mParent = newNode;
            rhs.mNode->mParent = newNode;
            lhs.mTape.push_back(newNode);
            return Variable(lhs.mTape, newNode);
        }

        friend Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
            Type result = lhs.mValue - rhs.mValue;
            auto newNode = new Node();
            lhs.mNode->mParent = newNode;
            rhs.mNode->mParent = newNode;
            return Variable(lhs.mTape, newNode);
        }

    private:
        Tape &mTape;
        Node *mNode;
        TypeValue mValue;
    };


    class Tape {
    public:
        Tape() { }

        template<typename T>
        Variable<T> variable(const T& value) {
            auto newNode = new Node();
            mTape.push_back(newNode);
            return Variable<T>(*this, newNode);
        }

    private:
        std::vector<Node*> mTape;
    };
}
