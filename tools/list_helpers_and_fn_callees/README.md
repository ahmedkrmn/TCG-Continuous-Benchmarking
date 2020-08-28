## Listing QEMU Helpers and Function Callees

This directory presents two Python scripts that facilitates the process of displaying the executed QEMU helpers and function callees without the need of setting up KCachegrind.

For more details on how the scripts works and for usage examples, please check [Report 4 - Listing QEMU Helpers and Function Callees](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Listing-QEMU-Helpers-and-Function-Callees/).

---

### Listing QEMU Helpers

`list_helpers.py` prints the executed helpers of a QEMU invocation.

**Syntax:**

```bash
list_helpers.py [-h] -- \
               <qemu executable> [<qemu executable options>] \
               <target executable> [<target executable options>]
```

- [-h] - Print the script arguments help message.

**Example of usage:**

```bash
list_helpers.py -- qemu-mips coulomb_double-mips -n10
```

**Example output:**

```
 Total number of instructions: 108,933,695

 Executed QEMU Helpers:

 No. Ins     Percent  Calls Ins/Call Helper Name             Source File
 --- ------- ------- ------ -------- --------------------    ---------------
   1 183,021  0.168%  1,305      140 helper_float_sub_d      <qemu>/target/mips/fpu_helper.c
   2 177,111  0.163%    770      230 helper_float_madd_d     <qemu>/target/mips/fpu_helper.c
   3 171,537  0.157%  1,014      169 helper_float_mul_d      <qemu>/target/mips/fpu_helper.c
   4 157,298  0.144%  2,443       64 helper_lookup_tb_ptr    <qemu>/accel/tcg/tcg-runtime.c
   5 138,123  0.127%    897      153 helper_float_add_d      <qemu>/target/mips/fpu_helper.c
   6  47,083  0.043%    207      227 helper_float_msub_d     <qemu>/target/mips/fpu_helper.c
   7  24,062  0.022%    487       49 helper_cmp_d_lt         <qemu>/target/mips/fpu_helper.c
   8  22,910  0.021%    150      152 helper_float_div_d      <qemu>/target/mips/fpu_helper.c
   9  15,497  0.014%    321       48 helper_cmp_d_eq         <qemu>/target/mips/fpu_helper.c
  10   9,100  0.008%     52      175 helper_float_trunc_w_d  <qemu>/target/mips/fpu_helper.c
  11   7,059  0.006%     10      705 helper_float_sqrt_d     <qemu>/target/mips/fpu_helper.c
  12   3,000  0.003%     40       75 helper_cmp_d_ule        <qemu>/target/mips/fpu_helper.c
  13   2,720  0.002%     20      136 helper_float_cvtd_w     <qemu>/target/mips/fpu_helper.c
  14   2,477  0.002%     27       91 helper_swl              <qemu>/target/mips/op_helper.c
  15   2,000  0.002%     40       50 helper_cmp_d_le         <qemu>/target/mips/fpu_helper.c
  16   1,800  0.002%     40       45 helper_cmp_d_un         <qemu>/target/mips/fpu_helper.c
  17   1,164  0.001%     12       97 helper_raise_exception_ <qemu>/target/mips/op_helper.c
  18     720  0.001%     10       72 helper_cmp_d_ult        <qemu>/target/mips/fpu_helper.c
  19     560  0.001%    140        4 helper_cfc1             <qemu>/target/mips/fpu_helper.c
```

---

### Listing Function Callees

`list_fn_callees.py` prints the callees of a given list of QEMU functions. It's a generalization of the `list_helpers.py` script.

**Syntax:**

```bash
list_fn_callees.py [-h] -f FUNCTION [FUNCTION ...] -- \
               <qemu executable> [<qemu executable options>] \
               <target executable> [<target executable options>]
```

- [-h] - Print the script arguments help message.
- -f FUNCTION [FUNCTION ...] - List of function names

**Example of usage:**

```bash
list_fn_callees.py -f helper_float_sub_d helper_float_mul_d -- \
                      qemu-mips coulomb_double-mips -n10
```

**Example output:**

```
 Total number of instructions: 108,952,851

 Callees of helper_float_sub_d:

 No. Instructions Percentage  Calls Ins/Call Function Name Source File
 --- ------------ ---------- ------ -------- ------------- ---------------
   1      153,160     0.141%  1,305     117  float64_sub   <qemu>/fpu/softfloat.c

 Callees of helper_float_mul_d:

 No. Instructions Percentage  Calls Ins/Call Function Name Source File
 --- ------------ ---------- ------ -------- ------------- ---------------
   1      131,137     0.120%  1,014      129 float64_mul   <qemu>/fpu/softfloat.c
```

---
