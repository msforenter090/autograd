#include <iostream>
#include "tape.h"


int main() {

    autograd::Tape tape;
    auto taped_a = tape.variable(12);
    auto taped_b = tape.variable(12);
    auto taped_c = taped_a + taped_b;
    return 0;
}
