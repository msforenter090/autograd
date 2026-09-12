#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "tape.h"

using autograd::Tape;
using autograd::Variable;

namespace {

int gFailures = 0;

void check(const char *what, double got, double expected, double tolerance = 1e-9) {
    const bool ok = std::fabs(got - expected) <= tolerance;
    if (!ok) {
        gFailures++;
    }
    std::printf("  %-34s got %14.9f  expected %14.9f  %s\n",
                what, got, expected, ok ? "ok" : "FAIL");
}

double sigmoid(double z) {
    return 1.0 / (1.0 + std::exp(-z));
}

/* Fan-out: x reaches z along two distinct paths, so its adjoint is a sum. */
void testFanOut() {
    std::printf("fan-out, z = (x + y) * (x * y)\n");

    Tape<double> tape;
    Variable<double> x = tape.variable(0.5);
    Variable<double> y = tape.variable(4.2);
    Variable<double> z = (x + y) * (x * y);

    autograd::Gradient<double> grad = z.gradient();

    check("z", z.value(), 9.87);
    check("dz/dx = 2xy + y^2", grad.withRespectTo(x), 2 * 0.5 * 4.2 + 4.2 * 4.2);
    check("dz/dy = x^2 + 2xy", grad.withRespectTo(y), 0.5 * 0.5 + 2 * 0.5 * 4.2);
}

/* Aliasing: both dependency slots of one node name the same tape entry. */
void testAliasing() {
    std::printf("aliasing, both dependencies on one node\n");

    {
        Tape<double> tape;
        Variable<double> x = tape.variable(0.5);
        Variable<double> y = tape.variable(4.2);
        Variable<double> m = x + y;
        Variable<double> z = m + m;

        autograd::Gradient<double> grad = z.gradient();
        check("m + m: dz/dm", grad.withRespectTo(m), 2.0);
        check("m + m: dz/dx", grad.withRespectTo(x), 2.0);
    }
    {
        Tape<double> tape;
        Variable<double> x = tape.variable(0.5);
        Variable<double> z = x * x;

        check("x * x: dz/dx = 2x", z.gradient().withRespectTo(x), 1.0);
    }
    {
        Tape<double> tape;
        Variable<double> x = tape.variable(0.5);
        Variable<double> z = x - x;

        check("x - x: dz/dx", z.gradient().withRespectTo(x), 0.0);
    }
}

/* The adjoint array covers the output node and everything below it, nothing more. */
void testGradientLength() {
    std::printf("adjoint array length\n");

    Tape<double> tape;
    Variable<double> x = tape.variable(0.5);
    Variable<double> y = tape.variable(4.2);
    Variable<double> z = x * y;

    /* Unrelated work piled onto the same tape after z was recorded. */
    Variable<double> unrelated = tape.variable(1.0);
    for (int i = 0; i < 100; i++) {
        unrelated = unrelated * 1.000001;
    }

    autograd::Gradient<double> grad = z.gradient();

    check("tape size", static_cast<double>(tape.size()), 104.0);
    check("adjoint array size", static_cast<double>(grad.size()), 3.0);
    check("dz/dx", grad.withRespectTo(x), 4.2);

    /* Recorded after z, so not an ancestor of it: zero, and no out-of-bounds read. */
    check("dz/d(unrelated)", grad.withRespectTo(unrelated), 0.0);

    Variable<double> afterTheFact = tape.variable(7.0);
    check("dz/d(made after gradient())", grad.withRespectTo(afterTheFact), 0.0);
}

void testElementary() {
    std::printf("elementary functions\n");

    Tape<double> tape;
    Variable<double> x = tape.variable(1.7);

    check("d/dx exp(x)", autograd::exp(x).gradient().withRespectTo(x), std::exp(1.7));
    check("d/dx log(x)", autograd::log(x).gradient().withRespectTo(x), 1.0 / 1.7);
    check("d/dx sqrt(x)", autograd::sqrt(x).gradient().withRespectTo(x), 0.5 / std::sqrt(1.7));
    check("d/dx tanh(x)", autograd::tanh(x).gradient().withRespectTo(x),
          1.0 - std::tanh(1.7) * std::tanh(1.7));
    check("d/dx sigmoid(x)", autograd::sigmoid(x).gradient().withRespectTo(x),
          sigmoid(1.7) * (1.0 - sigmoid(1.7)));
    check("d/dx x^3", autograd::pow(x, 3.0).gradient().withRespectTo(x), 3.0 * 1.7 * 1.7);
}

/* The network from main.c, differentiated by tape instead of by hand. Expected
   values are main.c's own delta formulas. */
void testNetworkAgainstHandDerivedDeltas() {
    std::printf("network from main.c vs. its hand-derived deltas\n");

    const double i0 = 0.3, i1 = 0.4, i2 = 0.444, t = 0.23;
    const double w00v = 0.15, w01v = -0.20, w02v = 0.10, w1v = 0.25, w2v = -0.30;

    Tape<double> tape;
    Variable<double> w00 = tape.variable(w00v);
    Variable<double> w01 = tape.variable(w01v);
    Variable<double> w02 = tape.variable(w02v);
    Variable<double> w1 = tape.variable(w1v);
    Variable<double> w2 = tape.variable(w2v);

    Variable<double> a0 = autograd::sigmoid(w00 * i0 + w01 * i1 + w02 * i2);
    Variable<double> a1 = autograd::sigmoid(w1 * a0);
    Variable<double> a2 = autograd::sigmoid(w2 * a1);

    Variable<double> error = autograd::pow(a2 - t, 2.0) * 0.5;
    autograd::Gradient<double> grad = error.gradient();

    const double ha0 = sigmoid(w00v * i0 + w01v * i1 + w02v * i2);
    const double ha1 = sigmoid(w1v * ha0);
    const double ha2 = sigmoid(w2v * ha1);

    const double delta2 = (ha2 - t) * ha2 * (1.0 - ha2);
    const double delta1 = delta2 * w2v * ha1 * (1.0 - ha1);
    const double delta0 = delta1 * w1v * ha0 * (1.0 - ha0);

    check("a2 forward", a2.value(), ha2);
    check("dE/dw2", grad.withRespectTo(w2), delta2 * ha1);
    check("dE/dw1", grad.withRespectTo(w1), delta1 * ha0);
    check("dE/dw00", grad.withRespectTo(w00), delta0 * i0);
    check("dE/dw01", grad.withRespectTo(w01), delta0 * i1);
    check("dE/dw02", grad.withRespectTo(w02), delta0 * i2);
}

/* Independent cross-check that does not reuse any derivative rule from tape.h. */
void testAgainstFiniteDifferences() {
    std::printf("finite-difference cross-check\n");

    auto f = [](double a, double b) {
        return std::exp(a * b) / (a + b) + std::tanh(a * a) * std::log(b);
    };

    const double a = 0.7, b = 1.3, h = 1e-6;

    Tape<double> tape;
    Variable<double> va = tape.variable(a);
    Variable<double> vb = tape.variable(b);
    Variable<double> z = autograd::exp(va * vb) / (va + vb)
                       + autograd::tanh(va * va) * autograd::log(vb);

    autograd::Gradient<double> grad = z.gradient();

    check("value", z.value(), f(a, b));
    check("dz/da", grad.withRespectTo(va), (f(a + h, b) - f(a - h, b)) / (2 * h), 1e-6);
    check("dz/db", grad.withRespectTo(vb), (f(a, b + h) - f(a, b - h)) / (2 * h), 1e-6);
}

} // namespace

int main() {
    testFanOut();
    testAliasing();
    testGradientLength();
    testElementary();
    testNetworkAgainstHandDerivedDeltas();
    testAgainstFiniteDifferences();

    if (gFailures > 0) {
        std::printf("\n%d check(s) FAILED\n", gFailures);
        return EXIT_FAILURE;
    }

    std::printf("\nall checks passed\n");
    return EXIT_SUCCESS;
}
