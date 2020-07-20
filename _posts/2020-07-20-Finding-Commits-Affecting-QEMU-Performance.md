---
layout: post
title: Finding Commits Affecting QEMU Performance
subtitle: Using "git bisect" for Detecting Performance Improvements and Degradations
tags: [Perf, Callgrind, git bissect, 5.1-pre-soft-freeze, ppc, mips]
excerpt: This report concludes a mini series of three reports that presented a comparison between QEMU 5.0 and QEMU 5.1-pre-soft-freeze. The first report in the series presented the comparison results as well as an analysis of the performance degradation introduced in the PowerPC targets using KCachegrind. The second report presented two new scripts for inspecting performance changes as well as the analysis of three other targets. Now it's time to find the commits responsible for the performance changes discussed in the two previous reports. This report provides a new script for performing the process of locating those commits automatically.
---

## Intro

This report concludes a mini series of three reports that presented a comparison between [QEMU 5.0](https://git.qemu.org/?p=qemu.git;a=commit;h=fdd76fecdde1ad444ff4deb7f1c4f7e4a1ef97d6) and [QEMU 5.1-pre-soft-freeze](https://git.qemu.org/?p=qemu.git;a=commit;h=fc1bff958998910ec8d25db86cd2f53ff125f7ab).
The first report in the series, ["QEMU 5.0 and 5.1-pre-soft-freeze Dissect Comparison"](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/QEMU-5.0-and-5.1-pre-soft-freeze-Dissect-Comparison/) presented the comparison results as well as an analysis of the performance degradation introduced in the PowerPC targets using KCachegrind.
The second report, ["Listing QEMU Helpers and Function Callees"](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Listing-QEMU-Helpers-and-Function-Callees/) presented two new scripts for inspecting performance changes as well as the analysis of three other targets.

Now it's time to find the commits responsible for the performance changes discussed in the two previous reports. This is what this report achieves by providing a new script for performing the process of locating those commits automatically.

## Table of Contents

- [Principle of Operation](#principle-of-operation)
- [Usage Instructions](#usage-instructions)
- [Finding the Commits](#finding-the-commits)
  - [MIPS Performance Improvement](#mips-performance-improvement)
  - [PowerPC Performance Degradation](#powerpc-performance-degradation)

## Principle of Operation

Given a range of commits to search between, one way to locate the performance change is by going through all commits in order and measuring the performance in each one to locate the commit with the first performance improvement or degradation. This can be achieved in linear time, which is considered very slow given the extra time taken by the performance measurement tools with each step in the search. The process of rebuilding QEMU itself with each search step is a big time consumer as well. To optimize this process, a faster search algorithm must be used, which makes binary search a great fit for this use case.

Git provides a command for abstracting the process of binary searching a git repository. This is the `git bissect` command which works be specifying two commit hashes to search in between. The default keywords for progressing through the commits in the search range are `git bissect good` and `git bissect bad`. The script overrides these two terms with the more descriptive `git bissect fast` and `git bissect slow` to better suit our use case.

The performance tools used for measurements are Perf and Callgrind. The choice of the underling tool is left for the user. As a guideline, Perf is quicker and can be used for detecting relatively big performance changes. On the other hand, Callgrind is slower, but it's more precise and stable, so it can be used for detecting small performance changes. For a detailed comparison between the two tools, please refer to the ["Measuring Basic Performance Metrics of QEMU"](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-Basic-Performance-Metrics-of-QEMU/) report.

The script is available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/bisect).

## Usage Instructions

**Syntax:**

```
./bisect.py -s,--start START [-e,--end END] [-q,--qemu QEMU] --target TARGET --tool {perf,callgrind} -- \
<target executable> [<target executable options>]
```

**Required Arguments:**

- -s,--start START : First commit hash in the search range
- --target TARGET : QEMU target name
- --tool {perf,callgrind} : Underlying tool used for performance measurements
- \<target executable\> \[\<target executable options\>\] : Benchmark path and arguments

**Optional Arguments:**

- [-e,--end END] : Last commit hash in the search range

  Default Value: Latest commit hash

- [-q,--qemu QEMU] : QEMU path

  Default Value: Path to a temporary directory containing a QEMU clone from GitHub

**Example of Usage:**

```bash
./bisect.py --start=fdd76fecdd --end=fc1bff9589 --target=mips --tool=perf -- coulomb_double-mips -n 1000
```

The above command will run the script specifying `fdd76fecdd` (QEMU 5.0) and `fc1bff9589` (QEMU 5.1-pre-soft-freeze) as the first and last commits in search boundary respectively. The selected target is `mips` and the tool used for performance measurement is `perf` given that the change in performance is 4.6% which is more than suitable to be used with Perf.

As you can see, a QEMU path is not specified, so the script will clone QEMU from GitHub and use the cloned repository throughout the script. The last required argument is the benchmark executable path and its flags which are provided after `--` to signify the end of the script-specific options.

The example above is exactly how the MIPS performance improvement can be detected.

## Finding the Commits

### MIPS Performance Improvement

Running the example command from the previous section, the result of running the script can be divided into five main blocks:

First block is the cloning progress of QEMU:

```
Fetching QEMU: Cloning into '/tmp/tmpdnftq6jr/qemu'...
remote: Enumerating objects: 90, done.
remote: Counting objects: 100% (90/90), done.
remote: Compressing objects: 100% (58/58), done.
remote: Total 504415 (delta 37), reused 51 (delta 32), pack-reused 504325
Receiving objects: 100% (504415/504415), 275.88 MiB | 1.32 MiB/s, done.
Resolving deltas: 100% (408816/408816), done.
```

Second block is the output of the performance measurements for the provided start and end commits:

```
Start Commit Instructions:     10,443,202,303
End Commit Instructions:       9,957,988,353
Performance Change:            +4.646%
```

Third block is the estimated number of bisect steps to be performed:

```
Estimated Number of Steps:     10
```

Fourth block is the bisect steps:

```
**************BISECT STEP 1**************
Instructions:        10,357,320,930
Status:              slow commit
**************BISECT STEP 2**************
Instructions:        9,957,899,711
Status:              fast commit
**************BISECT STEP 3**************
Instructions:        9,958,416,973
Status:              fast commit
**************BISECT STEP 4**************
Instructions:        10,357,001,629
Status:              slow commit
**************BISECT STEP 5**************
Instructions:        10,357,705,984
Status:              slow commit
**************BISECT STEP 6**************
Instructions:        9,957,817,538
Status:              fast commit
**************BISECT STEP 7**************
Instructions:        9,957,754,986
Status:              fast commit
**************BISECT STEP 8**************
Instructions:        10,357,316,118
Status:              slow commit
**************BISECT STEP 9**************
Instructions:        10,356,993,068
Status:              slow commit
**************BISECT STEP 10**************
Instructions:        10,357,592,149
Status:              slow commit
**************BISECT STEP 11**************
Instructions:        10,357,327,494
Status:              slow commit
```

And finally, last block is the bisect result:

```
*****************BISECT RESULT*****************
commit 9579f7816855757c747f9428a8e53d0fe0a0e9b7
Author: Aleksandar Markovic <aleksandar.qemu.devel@gmail.com>
Date:   Mon May 18 22:09:15 2020 +0200

    target/mips: fpu: Refactor conversion from ieee to mips exception flags

    The original conversion function is used for regular and MSA floating
    point instructions handling. Since there are some nuanced differences
    between regular and MSA floating point exception handling, provide two
    instances of the conversion function, rather than just a single common
    one. Inline both instances of this function instances for the sake of
    performance. Improve variable naming in surrounding code for clarity.

    Reviewed-by: Aleksandar Rikalo <aleksandar.rikalo@syrmia.com>
    Signed-off-by: Aleksandar Markovic <aleksandar.qemu.devel@gmail.com>
    Message-Id: <20200518200920.17344-17-aleksandar.qemu.devel@gmail.com>
```

The obtained bisect result confirms the analysis presented in the previous report regarding the MIPS improvement. Indeed, this ieee refactoring change is what inlined the `ieee_ex_to_mips` function causing the performance improvement in all MIPS targets.

### PowerPC Performance Degradation

To detect the performance degradation in PowerPC, the script will be ran as before, but with some few changes.
Firstly, the target and benchmark executable should be updated to ppc. Secondly, a local QEMU directory is used to save time instead of cloning from GitHub. Lastly, Callgrind is used instead of Perf to insure the correctness of the results given that the performance degradation was approx 2.4%.

```bash
./bisect.py --start=fdd76fecdd --end=fc1bff9589 --target=ppc --tool=callgrind --qemu=<qemu> -- coulomb_double-ppc -n 1000
```

Results:

```
Start Commit Instructions:     12,710,790,060
End Commit Instructions:       13,031,083,512
Performance Change:            -2.458%

Estimated Number of Steps:     10

**************BISECT STEP 1**************
Instructions:        13,031,097,790
Status:              slow commit
**************BISECT STEP 2**************
Instructions:        12,710,805,265
Status:              fast commit
**************BISECT STEP 3**************
Instructions:        13,031,028,053
Status:              slow commit
**************BISECT STEP 4**************
Instructions:        12,711,763,211
Status:              fast commit
**************BISECT STEP 5**************
Instructions:        13,031,027,292
Status:              slow commit
**************BISECT STEP 6**************
Instructions:        12,711,748,738
Status:              fast commit
**************BISECT STEP 7**************
Instructions:        12,711,748,788
Status:              fast commit
**************BISECT STEP 8**************
Instructions:        13,031,100,493
Status:              slow commit
**************BISECT STEP 9**************
Instructions:        12,714,472,954
Status:              fast commit
**************BISECT STEP 10**************
Instructions:        12,715,409,153
Status:              fast commit
**************BISECT STEP 11**************
Instructions:        12,715,394,739
Status:              fast commit

*****************BISECT RESULT*****************
commit 0673ecdf6cb2b1445a85283db8cbacb251c46516
Author: Richard Henderson <richard.henderson@linaro.org>
Date:   Tue May 5 10:40:23 2020 -0700

    softfloat: Inline float64 compare specializations

    Replace the float64 compare specializations with inline functions
    that call the standard float64_compare{,_quiet} functions.
    Use bool as the return type.

    Reviewed-by: Alex Bennée <alex.bennee@linaro.org>
    Signed-off-by: Richard Henderson <richard.henderson@linaro.org>
```

The result proves the claim presented in the two previous reports that the softfloat change was the reason behind the PowerPC performance drop. The replacement of the compare specializations with the standard `float64_compare` and `float64_compare_quiet` is the reason behind the performance regression.

<hr />

By successfully being able to locate the commits behind the performance change, this mini series of three reports is now concluded!

Next report will present a new performance comparison, but instead of comparing two QEMU versions as in this mini series, two different types of QEMU builds will be compared.
