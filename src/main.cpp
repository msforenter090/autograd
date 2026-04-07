#include <iostream>
#include "matrix.h"
#include "tape.h"


int main() {
    autograd::Tape tape;
    auto taped_a = tape.variable<MatrixImplementation<float>>(3, 3);
    auto taped_b = tape.variable<MatrixImplementation<float>>(3, 3);
    (*taped_a)(0, 0) = 101;
    auto taped_c = taped_a + taped_b;
    taped_a.backtrace();
    return 0;
}