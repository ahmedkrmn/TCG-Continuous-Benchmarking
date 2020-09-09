---
layout: post
title: QEMU Nightly Performance Tests
subtitle: Automatically Measuring the Performance Impact of Daily QEMU Changes
excerpt: QEMU currently lacks a system for measuring the performance of targets automatically. The previous reports introduced different tools and methods for locating performance regressions, but all of them had to be manually executed by the user when needed. This report presents a new nightly tests system that runs automatically each night. After the execution is completed, it sends a report to the QEMU mailing list with the performance measurements of seventeen different QEMU targets, and how these measurements compare to previously obtained ones.
---

## Intro

QEMU currently lacks a system for measuring the performance of targets automatically. The previous reports introduced different tools and methods for locating performance regressions, but all of them had to be manually executed by the user when needed.

This report devises a new nightly tests system that runs automatically each night. After the execution is completed, it sends a report to the QEMU mailing list with the performance measurements of seventeen different QEMU targets, and how these measurements compare to previously obtained ones.

The full source code is available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/qemu_nightly_tests).

## Table of Contents

- [Nightly Tests Benchmarks](#nightly-tests-benchmarks)
- [Nightly Tests System Execution Flow](#nightly-tests-system-execution-flow)
- [Nightly Tests System Execution Details](#nightly-tests-system-execution-details)
  - [Terminology](#terminology)
  - [Report Structure](#report-structure)
  - [Directory Structure](#directory-structure)

## Nightly Tests Benchmarks

To perform the nightly tests, eight different benchmarks are used to provide a variety of workloads for measuring QEMU performance. Five of these benchmarks were introduced in previous reports. This report adds three new ones to the list:

**dijkstra_double:**

Find the shortest path between the source node and all other nodes using Dijkstra's algorithm. The graph contains n nodes where all nxn distances are double values. The value of n can be specified using the -n flag. The default value is 2000.

**dijkstra_int32:**

Find the shortest path between the source node and all other nodes using Dijkstra's algorithm. The graph contains n nodes where all nxn distances are int32 values. The value of n can be specified using the -n flag. The default value is 2000.

**search_string:**

Search for the occurrence of a small string in a much larger random string ("needle in a hay"). The search process is repeated n number of times and each time, a different large random string ("hay") is generated. The value of n can be specified using the -n flag. The default value is 20.

All in all, the list of benchmarks used in the nightly tests can be categorized as follows:

| Double          | Integer        | String        |
| --------------- | -------------- | ------------- |
| dijkstra_double | dijkstra_int32 | qsort_string  |
| matmult_double  | matmult_int32  | search_string |
| qsort_double    | qsort_int32    |               |

All benchmarks are available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmarks).

## Nightly Tests System Execution Flow

![System Flow]({{ site.baseurl }}/images/nightly_tests/system_flow.png)

## Nightly Tests System Execution Details

QEMU performance is presented in terms of the number of instructions for each target when executed under the current QEMU master. This number is compared to yesterday's results and the reference version results which are both displayed in percentages.

**Required Settings:**

Update the [`GMAIL_USER`](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/242613d274ba96648f0f286367ca714166ba3a54/tools/qemu_nightly_tests/scripts/send_email.py#L31) object in `send_email.py` with your credentials.

**Running the System:**

```bash
python run_nightly_tests.py
```

### Terminology

**Reference Version:**

The base QEMU version used in the comparison. The default is v5.1.0 and it can also be manually provided using the `-r, --reference` flag.

**Latest:**

The latest measurements performed by the nightly test. It usually means "yesterday" as the tests are executed daily.

**History:**

Directory used for storing all the previous results of running the nightly tests.

### Report Structure

**First Section - Execution Details:**

```
Host CPU         : Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
Host Memory      : 15.49 GB

Start Time (UTC) : 2020-08-15 23:55:01
End Time (UTC)   : 2020-08-16 00:59:30
Execution Time   : 1:04:28.950629

Status           : SUCCESS
```

**Second Section - Results (SUCCESS):**

<pre class="highlight" style="font-size: 14.6205px;">
<code> --------------------------------------------------------
             SUMMARY REPORT - COMMIT d0ed6a69
 --------------------------------------------------------
                     AVERAGE RESULTS
 --------------------------------------------------------
 Target              Instructions      Latest      v5.1.0
 ----------  --------------------  ----------  ----------
 aarch64            2 118 465 797         N/A       -----
 alpha              1 838 388 986         N/A       -----
 arm                7 887 973 867         N/A       -----
 hppa               4 124 976 683         N/A       -----
 m68k               2 453 400 216         N/A       -----
 mips               1 812 620 481         N/A       -----
 mipsel             1 947 706 458         N/A       -----
 mips64             1 862 481 942         N/A       -----
 mips64el           1 984 197 821         N/A       -----
 ppc                2 394 300 315         N/A       -----
 ppc64              2 488 025 671         N/A       -----
 ppc64le            2 470 178 837         N/A       -----
 riscv64            1 367 753 288         N/A       -----
 s390x              3 058 482 814         N/A       -----
 sh4                2 278 475 330         N/A       -----
 sparc64            3 186 987 593         N/A       -----
 x86_64             1 734 458 339         N/A       -----
 --------------------------------------------------------

                    DETAILED RESULTS
 --------------------------------------------------------
 Test Program: dijkstra_double
 --------------------------------------------------------
 Target              Instructions      Latest      v5.1.0
 ----------  --------------------  ----------  ----------
 aarch64            3 019 590 811         N/A       -----
 alpha              3 078 095 968         N/A       -----
 arm               15 982 057 134         N/A       -----
 hppa               7 012 000 675         N/A       -----
 m68k               3 914 614 463         N/A       -----
 mips               2 979 125 423         N/A       -----
 mipsel             3 141 375 985         N/A       -----
 mips64             3 163 713 173         N/A       -----
 mips64el           3 314 091 136         N/A       -----
 ppc                4 692 124 964         N/A       -----
 ppc64              4 875 582 799         N/A       -----
 ppc64le            4 859 834 109         N/A       -----
 riscv64            2 150 241 306         N/A       -----
 s390x              4 455 491 800         N/A       -----
 sh4                3 816 826 725         N/A       -----
 sparc64            4 399 768 117         N/A       -----
 x86_64             2 441 352 288         N/A       -----
 --------------------------------------------------------</code>
</pre>

Same as `dijkstra_double`, a similar table is sent for each of the benchmarks.

**Remarks:**
The above report is the output when running the script for the first time when the QEMU master commit was [d0ed6a69](https://git.qemu.org/?p=qemu.git;a=commit;h=d0ed6a69d399ae193959225cdeaa9382746c91cc). This is also the same commit for the reference version (v5.1.0) so two things can be noticed:

1. All "Latest" percentages are "N/A". This is because for the first run, no previous results were stored.
2. All "v5.1.0" percentages are "`-----`" meaning that there is no change. This is because both the current master and the reference were basically the same commit.

**Second Section - Results (FAILURE):**

```
--------------------------------------------------------
                  ERROR LOGS
--------------------------------------------------------
2020-08-13T21:00:02.098674 - Verifying executables of 8 benchmarks for 17 targets
2020-08-13T21:00:02.099265 - Verifying results of reference version v5.0.0
2020-08-13T21:00:02.194617 - Checking out master
2020-08-13T21:00:02.229009 - Pulling the latest changes from QEMU master
From https://github.com/qemu/qemu
 * branch                  master     -> FETCH_HEAD
2020-08-13T21:00:05.832749 - Running 'configure' for master
2020-08-13T21:00:15.498038 - Running 'make' for master
ar: creating qemu-nightly-tests/qemu-nightly/build-gcc/capstone/libcapstone.a
qemu-nightly/linux-user/arm/cpu_loop.c:20:1: error: expected identifier or ‘(’ before ‘!’ token
 !! Manually Inserted Error !!
 ^
make[1]: *** [linux-user/arm/cpu_loop.o] Error 1
make: *** [arm-linux-user/all] Error 2
make: *** Waiting for unfinished jobs....
```

**Remarks:**
In this run, a syntax error was inserted in QEMU source code to verify that the nightly test will fail. Notice how the error logs show a step by step execution of the system flow to easily trace the error.

### Directory Structure

**Initial Directory Structure:**

```
.
├── benchmarks
│   └── source
│       ├── dijkstra_double
│       │   └── dijkstra_double.c
│       ├── dijkstra_int32
│       │   └── dijkstra_int32.c
│       ├── matmult_double
│       │   └── matmult_double.c
│       ├── matmult_int32
│       │   └── matmult_int32.c
│       ├── qsort_double
│       │   └── qsort_double.c
│       ├── qsort_int32
│       │   └── qsort_int32.c
│       ├── qsort_string
│       │   └── qsort_string.c
│       └── search_string
│           └── search_string.c
└── scripts
    ├── nightly_tests_core.py
    ├── run_nightly_tests.py
    └── send_email.py
```

**After First Run:**

```
.
├── benchmarks
│   ├── executables
│   │   ├── dijkstra_double
│   │   │   ├── dijkstra_double-aarch64
│   │   │   ├── dijkstra_double-alpha
│   │   │   ├── dijkstra_double-arm
│   │   │   ├── dijkstra_double-hppa
│   │   │   ├── dijkstra_double-m68k
│   │   │   ├── dijkstra_double-mips
│   │   │   ├── dijkstra_double-mips64
│   │   │   ├── dijkstra_double-mips64el
│   │   │   ├── dijkstra_double-mipsel
│   │   │   ├── dijkstra_double-ppc
│   │   │   ├── dijkstra_double-ppc64
│   │   │   ├── dijkstra_double-ppc64le
│   │   │   ├── dijkstra_double-riscv64
│   │   │   ├── dijkstra_double-s390x
│   │   │   ├── dijkstra_double-sh4
│   │   │   ├── dijkstra_double-sparc64
│   │   │   └── dijkstra_double-x86_64
│   │   ├── dijkstra_int32
│   │   │   ├── dijkstra_int32-aarch64
│   │   │   ├── dijkstra_int32-alpha
│   │   │   ├── dijkstra_int32-arm
│   │   │   ├── dijkstra_int32-hppa
│   │   │   ├── dijkstra_int32-m68k
│   │   │   ├── dijkstra_int32-mips
│   │   │   ├── dijkstra_int32-mips64
│   │   │   ├── dijkstra_int32-mips64el
│   │   │   ├── dijkstra_int32-mipsel
│   │   │   ├── dijkstra_int32-ppc
│   │   │   ├── dijkstra_int32-ppc64
│   │   │   ├── dijkstra_int32-ppc64le
│   │   │   ├── dijkstra_int32-riscv64
│   │   │   ├── dijkstra_int32-s390x
│   │   │   ├── dijkstra_int32-sh4
│   │   │   ├── dijkstra_int32-sparc64
│   │   │   └── dijkstra_int32-x86_64
│   │   ├── matmult_double
│   │   │   ├── matmult_double-aarch64
│   │   │   ├── matmult_double-alpha
│   │   │   ├── matmult_double-arm
│   │   │   ├── matmult_double-hppa
│   │   │   ├── matmult_double-m68k
│   │   │   ├── matmult_double-mips
│   │   │   ├── matmult_double-mips64
│   │   │   ├── matmult_double-mips64el
│   │   │   ├── matmult_double-mipsel
│   │   │   ├── matmult_double-ppc
│   │   │   ├── matmult_double-ppc64
│   │   │   ├── matmult_double-ppc64le
│   │   │   ├── matmult_double-riscv64
│   │   │   ├── matmult_double-s390x
│   │   │   ├── matmult_double-sh4
│   │   │   ├── matmult_double-sparc64
│   │   │   └── matmult_double-x86_64
│   │   ├── matmult_int32
│   │   │   ├── matmult_int32-aarch64
│   │   │   ├── matmult_int32-alpha
│   │   │   ├── matmult_int32-arm
│   │   │   ├── matmult_int32-hppa
│   │   │   ├── matmult_int32-m68k
│   │   │   ├── matmult_int32-mips
│   │   │   ├── matmult_int32-mips64
│   │   │   ├── matmult_int32-mips64el
│   │   │   ├── matmult_int32-mipsel
│   │   │   ├── matmult_int32-ppc
│   │   │   ├── matmult_int32-ppc64
│   │   │   ├── matmult_int32-ppc64le
│   │   │   ├── matmult_int32-riscv64
│   │   │   ├── matmult_int32-s390x
│   │   │   ├── matmult_int32-sh4
│   │   │   ├── matmult_int32-sparc64
│   │   │   └── matmult_int32-x86_64
│   │   ├── qsort_double
│   │   │   ├── qsort_double-aarch64
│   │   │   ├── qsort_double-alpha
│   │   │   ├── qsort_double-arm
│   │   │   ├── qsort_double-hppa
│   │   │   ├── qsort_double-m68k
│   │   │   ├── qsort_double-mips
│   │   │   ├── qsort_double-mips64
│   │   │   ├── qsort_double-mips64el
│   │   │   ├── qsort_double-mipsel
│   │   │   ├── qsort_double-ppc
│   │   │   ├── qsort_double-ppc64
│   │   │   ├── qsort_double-ppc64le
│   │   │   ├── qsort_double-riscv64
│   │   │   ├── qsort_double-s390x
│   │   │   ├── qsort_double-sh4
│   │   │   ├── qsort_double-sparc64
│   │   │   └── qsort_double-x86_64
│   │   ├── qsort_int32
│   │   │   ├── qsort_int32-aarch64
│   │   │   ├── qsort_int32-alpha
│   │   │   ├── qsort_int32-arm
│   │   │   ├── qsort_int32-hppa
│   │   │   ├── qsort_int32-m68k
│   │   │   ├── qsort_int32-mips
│   │   │   ├── qsort_int32-mips64
│   │   │   ├── qsort_int32-mips64el
│   │   │   ├── qsort_int32-mipsel
│   │   │   ├── qsort_int32-ppc
│   │   │   ├── qsort_int32-ppc64
│   │   │   ├── qsort_int32-ppc64le
│   │   │   ├── qsort_int32-riscv64
│   │   │   ├── qsort_int32-s390x
│   │   │   ├── qsort_int32-sh4
│   │   │   ├── qsort_int32-sparc64
│   │   │   └── qsort_int32-x86_64
│   │   ├── qsort_string
│   │   │   ├── qsort_string-aarch64
│   │   │   ├── qsort_string-alpha
│   │   │   ├── qsort_string-arm
│   │   │   ├── qsort_string-hppa
│   │   │   ├── qsort_string-m68k
│   │   │   ├── qsort_string-mips
│   │   │   ├── qsort_string-mips64
│   │   │   ├── qsort_string-mips64el
│   │   │   ├── qsort_string-mipsel
│   │   │   ├── qsort_string-ppc
│   │   │   ├── qsort_string-ppc64
│   │   │   ├── qsort_string-ppc64le
│   │   │   ├── qsort_string-riscv64
│   │   │   ├── qsort_string-s390x
│   │   │   ├── qsort_string-sh4
│   │   │   ├── qsort_string-sparc64
│   │   │   └── qsort_string-x86_64
│   │   └── search_string
│   │       ├── search_string-aarch64
│   │       ├── search_string-alpha
│   │       ├── search_string-arm
│   │       ├── search_string-hppa
│   │       ├── search_string-m68k
│   │       ├── search_string-mips
│   │       ├── search_string-mips64
│   │       ├── search_string-mips64el
│   │       ├── search_string-mipsel
│   │       ├── search_string-ppc
│   │       ├── search_string-ppc64
│   │       ├── search_string-ppc64le
│   │       ├── search_string-riscv64
│   │       ├── search_string-s390x
│   │       ├── search_string-sh4
│   │       ├── search_string-sparc64
│   │       └── search_string-x86_64
│   └── source
│       ├── dijkstra_double
│       │   └── dijkstra_double.c
│       ├── dijkstra_int32
│       │   └── dijkstra_int32.c
│       ├── matmult_double
│       │   └── matmult_double.c
│       ├── matmult_int32
│       │   └── matmult_int32.c
│       ├── qsort_double
│       │   └── qsort_double.c
│       ├── qsort_int32
│       │   └── qsort_int32.c
│       ├── qsort_string
│       │   └── qsort_string.c
│       └── search_string
│           └── search_string.c
├── qemu_nightly
│   ├── ...
├── results
│   ├── history
│   │   ├── d0ed6a69-average-results.csv
│   │   ├── dijkstra_double
│   │   │   └── d0ed6a69-dijkstra_double-results.csv
│   │   ├── dijkstra_int32
│   │   │   └── d0ed6a69-dijkstra_int32-results.csv
│   │   ├── matmult_double
│   │   │   └── d0ed6a69-matmult_double-results.csv
│   │   ├── matmult_int32
│   │   │   └── d0ed6a69-matmult_int32-results.csv
│   │   ├── qsort_double
│   │   │   └── d0ed6a69-qsort_double-results.csv
│   │   ├── qsort_int32
│   │   │   └── d0ed6a69-qsort_int32-results.csv
│   │   ├── qsort_string
│   │   │   └── d0ed6a69-qsort_string-results.csv
│   │   └── search_string
│   │       └── d0ed6a69-search_string-results.csv
│   ├── latest
│   │   ├── dijkstra_double
│   │   │   └── d0ed6a69-dijkstra_double-results.csv
│   │   ├── dijkstra_int32
│   │   │   └── d0ed6a69-dijkstra_int32-results.csv
│   │   ├── matmult_double
│   │   │   └── d0ed6a69-matmult_double-results.csv
│   │   ├── matmult_int32
│   │   │   └── d0ed6a69-matmult_int32-results.csv
│   │   ├── qsort_double
│   │   │   └── d0ed6a69-qsort_double-results.csv
│   │   ├── qsort_int32
│   │   │   └── d0ed6a69-qsort_int32-results.csv
│   │   ├── qsort_string
│   │   │   └── d0ed6a69-qsort_string-results.csv
│   │   └── search_string
│   │       └── d0ed6a69-search_string-results.csv
│   └── v5.1.0
│       ├── dijkstra_double
│       │   └── 64aaebe0-dijkstra_double-results.csv
│       ├── dijkstra_int32
│       │   └── 64aaebe0-dijkstra_int32-results.csv
│       ├── matmult_double
│       │   └── 64aaebe0-matmult_double-results.csv
│       ├── matmult_int32
│       │   └── 64aaebe0-matmult_int32-results.csv
│       ├── qsort_double
│       │   └── 64aaebe0-qsort_double-results.csv
│       ├── qsort_int32
│       │   └── 64aaebe0-qsort_int32-results.csv
│       ├── qsort_string
│       │   └── 64aaebe0-qsort_string-results.csv
│       └── search_string
│           └── 64aaebe0-search_string-results.csv
└── scripts
    ├── nightly_tests_core.py
    ├── run_nightly_tests.py
    └── send_email.py
```
