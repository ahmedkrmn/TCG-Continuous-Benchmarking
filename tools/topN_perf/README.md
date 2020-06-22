## Get The N Most Executed Functions with Perf

Easily detect QEMU runtime hotspots by printing the most executed functions when running a given program.

### Usage

Assume that you're testing against an arm executable of the [Coulomb benchmark](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmakrs/coulomb).

```bash
topN_perf.py -n 20 -- qemu-arm coulomb_double-arm
```

### Output

```
 No.  Percentage  Name                       Caller
----  ----------  -------------------------  -------------------------
   1      16.25%  float64_mul                qemu-x86_64
   2      12.01%  float64_sub                qemu-x86_64
   3      11.99%  float64_add                qemu-x86_64
   4       5.69%  helper_mulsd               qemu-x86_64
   5       4.68%  helper_addsd               qemu-x86_64
   6       4.43%  helper_lookup_tb_ptr       qemu-x86_64
   7       4.28%  helper_subsd               qemu-x86_64
   8       2.71%  f64_compare                qemu-x86_64
   9       2.71%  helper_ucomisd             qemu-x86_64
  10       1.04%  helper_pand_xmm            qemu-x86_64
  11       0.71%  float64_div                qemu-x86_64
  12       0.63%  helper_pxor_xmm            qemu-x86_64
  13       0.50%  0x00007f7b7004ef95         [JIT] tid 491
  14       0.50%  0x00007f7b70044e83         [JIT] tid 491
  15       0.36%  helper_por_xmm             qemu-x86_64
  16       0.32%  helper_cc_compute_all      qemu-x86_64
  17       0.30%  0x00007f7b700433f0         [JIT] tid 491
  18       0.30%  float64_compare_quiet      qemu-x86_64
  19       0.27%  soft_f64_addsub            qemu-x86_64
  20       0.26%  round_to_int               qemu-x86_64
```
