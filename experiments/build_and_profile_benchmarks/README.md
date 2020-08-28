## Build and Profile Benchmarks

### Introduction

A bash script that builds and profiles five string benchmarks for seventeen QEMU targets using Callgrind on seven different QEMU versions.

**Benchmarks:**

- **obscure_string:**

  Obscuring a set of random strings of a constant length by xoring them with 42.

- **reorder_string:**

  Repeated alphabetical ordering of string characters for a set of random strings.

- **reverse_string:**

  Repeated reversing of a set of random strings.

- **shuffle_string:**

  Repeated shuffling of a set of random strings.

- **toupper_string:**

  Repeated changing case to uppersace for a set of random strings.

**Targets List**:

- aarch64
- alpha
- arm
- hppa
- m68k
- mips
- mipsel
- mips64
- mips64el
- ppc
- ppc64
- ppc64le
- riscv64
- s390x
- sh4
- sparc64
- x86_64

**QEMU Versions:**

- 3.0.0
- 3.1.0
- 4.0.0
- 4.1.0
- 4.2.0
- 5.0.0
- master

For all QEMU version, benchmarks are profiled using the default value of input `n`. For QEMU 5.0.0, addition profiling is performed using `-n=10000`, `-n=20000`, `-n=30000`, `-n=40000`, `-n=50000`, `-n=60000`.

### Execution Details

Given a list of benchmark programs and a list of Linux cross compilers, the tool does the following:

1. Builds each of the given benchmarks with all of the given cross compilers.
2. For all of the resulting executables, it does the following:
   1. Runs QEMU version `5.0.0` and save the control results in the `control_results_output` directory.
   2. Using Callgrind, it does the following:
      1. Runs Qemu versions `3.0.0`, `3.1.0`, `4.0.0`, `4.1.0`, `4.2.0`, `5.0.0`, and `master` without providing a custom input to the benchmarks and save the results in the `callgrind_output` directory.
      2. Runs QEMU version `5.0.0` with providing `10000`, `20000`, `30000`, `40000`, `50000`, and `60000` as custom inputs to the benchmarks and saves the results in the `callgrind_output` directory.

Execution on an Intel Core i7-8750H with 16GB of RAM takes 12 hours and 46 minutes.

### Usage

Modify the `SCRIPT OPTIONS` section at the top of the scripts to include the following:

1. List of target compilers.
2. Absolute paths of the benchmark programs.
3. Absolute paths of build-gcc populated QEMU versions.

**Example:**

```bash
####################################SCRIPT OPTIONS#####################################
# List of target compilers
TARGET_COMPILERS=(
    aarch64-linux-gnu-gcc
    alpha-linux-gnu-gcc
    arm-linux-gnueabi-gcc
    hppa-linux-gnu-gcc
    mips-linux-gnu-gcc
    mipsel-linux-gnu-gcc
    mips64-linux-gnuabi64-gcc
    mips64el-linux-gnuabi64-gcc
    m68k-linux-gnu-gcc
    powerpc-linux-gnu-gcc
    powerpc64-linux-gnu-gcc
    powerpc64le-linux-gnu-gcc
    riscv64-linux-gnu-gcc
    sh4-linux-gnu-gcc
    sparc64-linux-gnu-gcc
    s390x-linux-gnu-gcc
    gcc
)
# List of benchmark absolute paths
BENCHMARK_PATHS=(
    ./benchmarks/obscure_string/obscure_string.c
    ./benchmarks/reorder_string/reorder_string.c
    ./benchmarks/reverse_string/reverse_string.c
    ./benchmarks/shuffle_string/shuffle_string.c
    ./benchmarks/toupper_string/toupper_string.c
)
# List of build-gcc populated QEMU absolute paths
QEMU_PATHS=(
    /build/qemu-master
    /build/qemu-5.0.0
    /build/qemu-4.2.0
    /build/qemu-4.1.0
    /build/qemu-4.0.0
    /build/qemu-3.1.0
    /build/qemu-3.0.0
)
#######################################################################################
```

### Results

[https://drive.google.com/file/d/13sIDh9r5XWojB0CTihFSY5WuaRggeFWa](https://drive.google.com/file/d/13sIDh9r5XWojB0CTihFSY5WuaRggeFWa)

**Directory structure after running the script:**

```
.
├── benchmarks
│   ├── obscure_string
│   │   └── obscure_string.c
│   ├── reorder_string
│   │   └── reorder_string.c
│   ├── reverse_string
│   │   └── reverse_string.c
│   ├── shuffle_string
│   │   └── shuffle_string.c
│   └── toupper_string
│       └── toupper_string.c
├── build_and_profile_benchmarks.sh
├── callgrind_output
│   ├── obscure_string
│   │   ├── 3.0.0
│   │   │   └── AK-L
│   │   │       ├── callgrind.out.obscure_string-aarch64.30249
│   │   │       ├── callgrind.out.obscure_string-alpha.30620
│   │   │       ├── callgrind.out.obscure_string-arm.31136
│   │   │       ├── callgrind.out.obscure_string-hppa.31496
│   │   │       ├── callgrind.out.obscure_string-m68k.905
│   │   │       ├── callgrind.out.obscure_string-mips.31868
│   │   │       ├── callgrind.out.obscure_string-mips64.32672
│   │   │       ├── callgrind.out.obscure_string-mips64el.744
│   │   │       ├── callgrind.out.obscure_string-mipsel.32205
│   │   │       ├── callgrind.out.obscure_string-ppc.1364
│   │   │       ├── callgrind.out.obscure_string-ppc64.1602
│   │   │       ├── callgrind.out.obscure_string-ppc64le.1825
│   │   │       ├── callgrind.out.obscure_string-riscv64.2001
│   │   │       ├── callgrind.out.obscure_string-s390x.2905
│   │   │       ├── callgrind.out.obscure_string-sh4.2247
│   │   │       ├── callgrind.out.obscure_string-sparc64.2682
│   │   │       └── callgrind.out.obscure_string-x86_64.3089
│   │   ├── 3.1.0
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 4.0.0
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 4.1.0
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 4.2.0
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-10000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-20000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-30000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-40000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-50000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   ├── 5.0.0-60000
│   │   │   └── AK-L
│   │   │       ├── ...
│   │   └── master
│   │       └── AK-L
│   │           ├── ...
│   ├── reorder_string
│   │   ├── ...
│   ├── reverse_string
│   │   ├── ...
│   ├── shuffle_string
│   │   ├── ...
│   └── toupper_string
│       ├── ...
└── control_results_output
    ├── obscure_string
    │   ├── obscure_string-aarch64.txt
    │   ├── obscure_string-alpha.txt
    │   ├── obscure_string-arm.txt
    │   ├── obscure_string-hppa.txt
    │   ├── obscure_string-m68k.txt
    │   ├── obscure_string-mips64el.txt
    │   ├── obscure_string-mips64.txt
    │   ├── obscure_string-mipsel.txt
    │   ├── obscure_string-mips.txt
    │   ├── obscure_string-ppc64le.txt
    │   ├── obscure_string-ppc64.txt
    │   ├── obscure_string-ppc.txt
    │   ├── obscure_string-riscv64.txt
    │   ├── obscure_string-s390x.txt
    │   ├── obscure_string-sh4.txt
    │   ├── obscure_string-sparc64.txt
    │   └── obscure_string-x86_64.txt
    ├── reorder_string
    │   ├── ...
    ├── reverse_string
    │   ├── ...
    ├── shuffle_string
    │   ├── ...
    └── toupper_string
        ├── ...
```
