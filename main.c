#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double a0;
    double a1;
    double a2;
} Activations;

typedef struct {
    double i0;
    double i1;
    double i2;
    double t;
    double achived;
    int iteration;
} Sample;

static double sigmoid(double z) {
    return 1.0 / (1.0 + exp(-z));
}

static Activations forward(double i0, double i1, double i2,
                           double w00, double w01, double w02,
                           double w1, double w2) {
    Activations out;
    double z0 = w00 * i0 + w01 * i1 + w02 * i2;
    out.a0 = sigmoid(z0);

    double z1 = w1 * out.a0;
    out.a1 = sigmoid(z1);

    double z2 = w2 * out.a1;
    out.a2 = sigmoid(z2);
    return out;
}

static Sample next_sample(size_t iter) {
    /* Small toy dataset (OR-like classification). */
    static const Sample samples[] = {
        // Randomoze input values, use static values for target.
        {0.3, 0.4, 0.444, 0.23, 0.0, 0},
        {0.112, 0.992, 0.631, 0.87, 0.0, 0},
    };
    return samples[iter % (sizeof(samples) / sizeof(samples[0]))];
}

int main(void) {
    /* Weights: input->layer0 and then depth-only links. */
    double w00 = 0.15;
    double w01 = -0.20;
    double w02 = 0.10;
    double w1 = 0.25;
    double w2 = -0.30;

    const double eta = 0.5;
    const double epsilon = 0.01;
    const int max_iters = 200000;

    double error = INFINITY;
    int iter = 0;

    for (size_t sample = 0; sample < 2; sample++) {
        Sample s = next_sample(sample);
        iter = 0;
        error = INFINITY;
        printf("==================== New Sample ====================\n");
        while (error > epsilon && iter < max_iters) {
            Activations a = forward(s.i0, s.i1, s.i2, w00, w01, w02, w1, w2);

            /* Backpropagation deltas (sigmoid derivative = a * (1 - a)). */
            double delta2 = (a.a2 - s.t) * a.a2 * (1.0 - a.a2);
            double delta1 = delta2 * w2 * a.a1 * (1.0 - a.a1);
            double delta0 = delta1 * w1 * a.a0 * (1.0 - a.a0);

            /* Gradient descent updates. */
            w2 -= eta * delta2 * a.a1;
            w1 -= eta * delta1 * a.a0;
            w00 -= eta * delta0 * s.i0;
            w01 -= eta * delta0 * s.i1;
            w02 -= eta * delta0 * s.i2;

            // error = 0.5 * (s.t - a.a2) * (s.t - a.a2);
            error = fabs(s.t - a.a2);
            s.achived = a.a2;
            s.iteration = iter;

            printf("iter=%d error=%.8f sample=(%f, %f, %f) target=%f pred=%f\n",
                iter, error, s.i0, s.i1, s.i2, s.t, a.a2);
            iter++;
        }
    }

    printf("Final predictions:\n");
    for (size_t k = 0; k < 2; k++) {
        Sample s = next_sample(k);
        Activations a = forward(s.i0, s.i1, s.i2, w00, w01, w02, w1, w2);
        double e = 0.5 * (s.t - a.a2) * (s.t - a.a2);
        printf("input=(%.0f, %.0f, %.0f) target=%.0f pred=%.5f err=%.8f iter=%d\n",
               s.i0, s.i1, s.i2, s.t, a.a2, e, s.iteration);
    }

    return EXIT_SUCCESS;
}
