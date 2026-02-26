#pragma once

#include <vector>
#include "variable.h"

#include <iostream>

namespace autograd {
    class Node;

    class Tape {
    public:
        Tape() {}

        template<typename T>
        Variable<T> variable(const T& value) {
            return Variable(*this, value);
        }

        void push_back(Node *node) {
            mTape.emplace_back(node);
            std::cout << "Hello" << std::endl;
        }

    private:
        std::vector<Node*> mTape;
    };
}
