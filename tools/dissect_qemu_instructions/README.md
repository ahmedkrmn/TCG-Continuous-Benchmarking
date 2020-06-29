## Dissect QEMU Instructions

Break down QEMU execution into three main phases: code generation, JIT execution and helpers execution.

### Usage

Assume that you're testing against an x86_64 executable of the [Coulomb benchmark](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmakrs/coulomb).

```bash
./dissect.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

### Output

```
Total Instructions:        4,702,865,362

Code Generation:             115,819,309	 2.463%
JIT Execution:             1,081,980,528	23.007%
Helpers:                   3,505,065,525	74.530%
```
