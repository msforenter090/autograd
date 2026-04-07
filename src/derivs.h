#pragma once

#include "matrix.h"

namespace autograd {
    template<typename T, typename... Args>
    T identity(Args&& ...args) {
        return T(std::forward<Args>(args)...);
    }
}
