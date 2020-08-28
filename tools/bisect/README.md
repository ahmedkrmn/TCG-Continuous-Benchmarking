## Bisecting QEMU Commit History

Python script that locates the commit that caused a performance degradation or improvement in QEMU using the git bisect command (binary search).

**Syntax:**

```bash
bisect.py [-h] -s,--start START [-e,--end END] [-q,--qemu QEMU] \
--target TARGET --tool {perf,callgrind} -- \
<target executable> [<target executable options>]
```

- [-h] - Print the script arguments help message
- -s,--start START - First commit hash in the search range
- [-e,--end END] - Last commit hash in the search range
  (default: Latest commit)
- [-q,--qemu QEMU] - QEMU path.
  (default: Path to a GitHub QEMU clone)
- --target TARGET - QEMU target name
- --tool {perf,callgrind} - Underlying tool used for measurements

For more details on how the script works and for usage examples, please check [Report 5 - Finding Commits Affecting QEMU Performance](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Finding-Commits-Affecting-QEMU-Performance/).

**Example of usage:**

```bash
bisect.py --start=fdd76fecdd --qemu=/path/to/qemu --target=ppc \
--tool=perf -- coulomb_double-ppc -n 1000
```

**Example output:**

```
Start Commit Instructions:     12,710,790,060
End Commit Instructions:       13,031,083,512
Performance Change:            -2.458%

Estimated Number of Steps:     10

*****************BISECT STEP 1*****************
Instructions:        13,031,097,790
Status:              slow commit
*****************BISECT STEP 2*****************
Instructions:        12,710,805,265
Status:              fast commit
*****************BISECT STEP 3*****************
Instructions:        13,031,028,053
Status:              slow commit
*****************BISECT STEP 4*****************
Instructions:        12,711,763,211
Status:              fast commit
*****************BISECT STEP 5*****************
Instructions:        13,031,027,292
Status:              slow commit
*****************BISECT STEP 6*****************
Instructions:        12,711,748,738
Status:              fast commit
*****************BISECT STEP 7*****************
Instructions:        12,711,748,788
Status:              fast commit
*****************BISECT STEP 8*****************
Instructions:        13,031,100,493
Status:              slow commit
*****************BISECT STEP 9*****************
Instructions:        12,714,472,954
Status:              fast commit
****************BISECT STEP 10*****************
Instructions:        12,715,409,153
Status:              fast commit
****************BISECT STEP 11*****************
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
***********************************************
```
