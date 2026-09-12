# Autograd System

This repository contains a simple autograd system, designed for automatic differentiation. It provides the core functionalities to compute gradients of arbitrary computational graphs, enabling the implementation of various machine learning algorithms.

## Design

Reverse-mode automatic differentiation over a flat, append-only tape.

Each operation records a `Node` holding the local partial derivatives of its
result with respect to its inputs, plus the tape indices of those inputs. A
node's dependencies always have a lower index than the node itself, so the tape
is topologically ordered by construction and the backward pass is a single
reverse linear scan rather than a graph traversal — no recursion, no cycles.

The scan keeps one adjoint per node and accumulates with `+=`, which is what
makes both fan-out (one value feeding several operations) and aliasing (`x * x`,
where one node names the same input twice) come out right without special cases.

Two details worth knowing:

- **The adjoint array is sized to the output node, not the whole tape.** Adjoints
  only flow toward lower indices, so nothing above the seed can ever become
  nonzero. Differentiating an early variable does not pay for a long tape.
- **`withRespectTo` returns zero for variables past the end of that array.** Those
  were recorded after the output and so are not ancestors of it. That is the
  correct derivative, not a sentinel.

The tape is never cleared, so a long training loop grows it without bound and
each `gradient()` call walks more of it than the last. Scope a `Tape` per
iteration if that matters.

## Usage

Header-only. Include `src/tape.h`:

```cpp
#include "tape.h"

autograd::Tape<double> tape;
autograd::Variable<double> x = tape.variable(0.5);
autograd::Variable<double> y = tape.variable(4.2);

autograd::Variable<double> z = (x + y) * (x * y);
autograd::Gradient<double> grad = z.gradient();

z.value();                 // 9.87
grad.withRespectTo(x);     // 21.84
grad.withRespectTo(y);     // 4.45
```

Available operations: `+ - * /` (variable/variable and variable/scalar), unary
`+ -`, compound assignment, and `exp`, `log`, `sqrt`, `pow`, `tanh`, `sigmoid`.

Variables are bound to the tape that created them; mixing tapes throws
`std::invalid_argument`. Tapes are neither copyable nor movable, since live
variables refer to them by reference.

## Tests

```bash
make test
```

Builds with ASan and UBSan enabled. The suite covers fan-out, aliasing, adjoint
array sizing and bounds, the elementary derivative rules, a finite-difference
cross-check, and a comparison of the tape's gradients against the hand-derived
backpropagation deltas in `main.c` for the same network.
