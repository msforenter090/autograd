#include <vector>
#include <stdexcept>
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
        using TypeValue = Type;
        using Reference = TypeValue&;
        using Pointer = Type*;

        template<typename... Args>
        Variable(Tape &tape, Node *node, Args&& ...args) : mTape(tape), mNode(node), mValue(std::forward<Args>(args)...) { }
        Variable(Tape &tape, Node *node, TypeValue&& other) : mTape(tape), mNode(node), mValue(std::forward<TypeValue>(other)) { }

        Reference operator*() { return mValue; }

        // This tells the compiler this friend is a template specialization.
        friend Variable<TypeValue> operator+<TypeValue>(const Variable<TypeValue>&, const Variable<TypeValue>&);
        friend Variable<TypeValue> operator-<TypeValue>(const Variable<TypeValue>&, const Variable<TypeValue>&);

        void backtrace() {
            while(mNode != nullptr) {
                std::cout << mNode << std::endl;
                mNode = mNode->mParent;
            }
        }

    private:
        Tape &mTape;
        Node *mNode;
        TypeValue mValue;
    };

    class Tape {
    public:
        void push_back(Node* n) {
            mTape.push_back(n);
        }

        template<typename Type, typename...Args>
        Variable<Type> variable(Args&& ...args) {
            auto newNode = new Node();
            mTape.push_back(newNode);
            return Variable<Type>(*this, newNode, std::forward<Args>(args)...);
        }

    private:
        std::vector<Node*> mTape;
    };

    template<typename Type>
    Variable<Type> operator+(const Variable<Type> &lhs, const Variable<Type> &rhs) {
        if(&lhs.mTape != &rhs.mTape) {
            throw std::runtime_error("Different tape object.");
        }

        auto newNode = new Node();
        auto result = lhs.mValue + rhs.mValue;

        lhs.mNode->mParent = newNode;
        rhs.mNode->mParent = newNode;
        lhs.mTape.push_back(newNode);

        return Variable<Type>(lhs.mTape, newNode, result);
    }

    template<typename Type>
    Variable<Type> operator-(const Variable<Type> &lhs, const Variable<Type> &rhs) {
        if(&lhs.mTape != &rhs.mTape) {
            throw std::runtime_error("Different tape object.");
        }

        auto newNode = new Node();
        auto result = lhs.mValue - rhs.mValue;

        lhs.mNode->mParent = newNode;
        rhs.mNode->mParent = newNode;
        lhs.mTape.push_back(newNode);

        return Variable<Type>(lhs.mTape, newNode, result);
    }
}
