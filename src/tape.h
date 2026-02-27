#include <vector>
#include <iostream>

namespace autograd {
    class Node;
    class Tape;

    template<typename Type>
    class Variable;

    template<typename Type>
    Variable<Type> operator+(const Variable<Type>&, const Variable<Type>&);

    template<typename Type>
    Variable<Type> operator-(const Variable<Type>&, const Variable<Type>&);

    class Node {
    public:
        Node(Node *parent = nullptr) : mParent(parent) { }
        Node *mParent;
    };

    template<typename Type>
    class Variable {
    public:
        Variable(Tape &tape, Node *node) : mTape(tape), mNode(node), mValue() { }

        // This tells the compiler this friend is a template specialization.
        friend Variable<Type> operator+ <Type>(const Variable<Type>&, const Variable<Type>&);
        friend Variable<Type> operator- <Type>(const Variable<Type>&, const Variable<Type>&);

        void backtrace() {
            while(mNode != nullptr) {
                std::cout << mNode << std::endl;
                mNode = mNode->mParent;
            }
        }

    private:
        Tape &mTape;
        Node *mNode;
        Type mValue;
    };

    class Tape {
    public:
        void push_back(Node* n) {
            mTape.push_back(n);
        }

        template<typename T>
        Variable<T> variable(const T& value) {
            auto newNode = new Node();
            mTape.push_back(newNode);
            return Variable<T>(*this, newNode);
        }

    private:
        std::vector<Node*> mTape;
    };

    template<typename Type>
    Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
        auto newNode = new Node();
        auto result = lhs.mValue + rhs.mValue;

        lhs.mNode->mParent = newNode;
        rhs.mNode->mParent = newNode;
        lhs.mTape.push_back(newNode);

        return Variable<Type>(lhs.mTape, newNode);
    }

    template<typename Type>
    Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
        auto newNode = new Node();
        auto result = lhs.mValue - rhs.mValue;

        lhs.mNode->mParent = newNode;
        rhs.mNode->mParent = newNode;
        lhs.mTape.push_back(newNode);

        return Variable<Type>(lhs.mTape, newNode);
    }
}
