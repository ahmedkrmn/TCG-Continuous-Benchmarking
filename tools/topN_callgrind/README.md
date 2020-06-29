## Get The N Most Executed Functions with Callgrind

Easily detect QEMU runtime hotspots by printing the most executed functions when running a given program.

### Usage

Assume that you're testing against an x86_64 executable of the [Coulomb benchmark](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmakrs/coulomb).

```bash
./topN_callgrind.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

### Output

```
 No.  Percentage  Function Name              Source File
----  ----------  -------------------------  ------------------------------
   1     24.577%  0x00000000082db000         ???
   2     20.467%  float64_mul                <qemu>/fpu/softfloat.c
   3     14.720%  float64_sub                <qemu>/fpu/softfloat.c
   4     13.864%  float64_add                <qemu>/fpu/softfloat.c
   5      4.876%  helper_mulsd               <qemu>/target/i386/ops_sse.h
   6      3.767%  helper_subsd               <qemu>/target/i386/ops_sse.h
   7      3.549%  helper_addsd               <qemu>/target/i386/ops_sse.h
   8      2.185%  helper_ucomisd             <qemu>/target/i386/ops_sse.h
   9      1.667%  helper_lookup_tb_ptr       <qemu>/include/exec/tb-lookup.h
  10      1.662%  f64_compare                <qemu>/fpu/softfloat.c
  11      1.509%  helper_lookup_tb_ptr       <qemu>/accel/tcg/tcg-runtime.c
  12      0.635%  helper_lookup_tb_ptr       <qemu>/include/exec/exec-all.h
  13      0.616%  float64_div                <qemu>/fpu/softfloat.c
  14      0.502%  helper_pand_xmm            <qemu>/target/i386/ops_sse.h
  15      0.502%  float64_mul                <qemu>/include/fpu/softfloat.h
  16      0.476%  helper_lookup_tb_ptr       <qemu>/target/i386/cpu.h
  17      0.437%  float64_compare_quiet      <qemu>/fpu/softfloat.c
  18      0.414%  helper_pxor_xmm            <qemu>/target/i386/ops_sse.h
  19      0.353%  round_to_int               <qemu>/fpu/softfloat.c
  20      0.347%  helper_cc_compute_all      <qemu>/target/i386/cc_helper.c
```
