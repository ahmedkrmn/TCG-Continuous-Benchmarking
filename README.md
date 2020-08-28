## TCG Continuous Benchmarking

<p align="center">
  <img alt="GSoC and QEMU Logo"  src="logo.png" />
</p>

[TCG Continuous Benchmarking](https://summerofcode.withgoogle.com/projects/#5636539490500608) is the QEMU project for Google Summer of Code 2020 which offers an in depth exploratory performance analysis and benchmarking for the QEMU emulator using the TCG JIT in both its Linux user and system modes.

This repository presents all of the work done throughout the project.

---

### Performance Analysis Reports and Tools

A [dedicated website](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/) was built to host the weekly analysis reports of the project. Each report tackled a specific topic and introduced all the tools, procedures and methodologies to achieve the required results.

The website offers a [Search page](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/search) for searching the reports for a specific phrase, a [Tags page](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/search) for viewing the reports per tag, and a [Setup page](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/setup/) that shows the testbed hardware information and all the required setup procedures.

<p align="center">
  <img alt="Reports Website" width="75%" src="reports_website.png" />
</p>

**Submitted Reports:**

- [Report 1 - Measuring Basic Performance Metrics of QEMU](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-Basic-Performance-Metrics-of-QEMU/)
  - **Tools:** [topN_perf.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_perf) and [topN_callgrind.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_callgrind)
- [Report 2 - Dissecting QEMU Into Three Main Parts](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Dissecting-QEMU-Into-Three-Main-Parts/)
  - **Tools:** [dissect.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/dissect_qemu_instructions)
- [Report 3 - QEMU 5.0 and 5.1-pre-soft-freeze Dissect Comparison](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/QEMU-5.0-and-5.1-pre-soft-freeze-Dissect-Comparison/)
- [Report 4 - Listing QEMU Helpers and Function Callees](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Listing-QEMU-Helpers-and-Function-Callees/)
  - **Tools:** [list_helpers.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/list_helpers_and_fn_callees) and [list_fn_callees.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/list_helpers_and_fn_callees)
- [Report 5 - Finding Commits Affecting QEMU Performance](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Finding-Commits-Affecting-QEMU-Performance/)
  - **Tools:** [bisect.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/bisect)
- [Report 6 - Performance Comparison of Two QEMU Builds](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Performance-Comparison-of-Two-QEMU-Builds/)
- [Report 7 - Measuring QEMU Emulation Efficiency](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-QEMU-Emulation-Efficiency/)
- [Report 8 - QEMU Nightly Performance Tests](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/QEMU-Nightly-Performance-Tests/)
  - **Tools:** [Nightly Tests System](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/qemu_nightly_tests)
- [Report 9 - Measuring QEMU Performance in System Mode](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-QEMU-Performance-in-System-Mode/)
  - **Tools:** [topN_system.py](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_system)
- [Report 10 - Measuring QEMU Performance in System Mode - Part Two](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-QEMU-Performance-in-System-Mode-Part-Two/)

---

### Experiments

The month before the project was spent on collecting benchmarking and analysis data on QEMU performance. The following is the list of tools used in these experiments.

- [Build and Profile Benchmarks](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/experiments/build_and_profile_benchmarks)
- [LIBM Test](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/experiments/libm_test)

---

### List of Submitted Patches and Bugs

**Patches:**

- [[PATCH 0/2] Update use_goto_tb() in hppa and rx targets](https://www.mail-archive.com/qemu-devel@nongnu.org/msg704950.html)
  - [[PATCH 1/2] target/hppa: Check page crossings in use_goto_tb() only in system mode](https://www.mail-archive.com/qemu-devel@nongnu.org/msg704952.html)
  - [[PATCH 2/2] target/rx: Check for page crossings in use_goto_tb()](https://www.mail-archive.com/qemu-devel@nongnu.org/msg704951.html)
- [[PATCH v2 0/1] Check for page crossings in use_goto_tb() for rx target](https://www.mail-archive.com/qemu-devel@nongnu.org/msg708098.html)
  - [[PATCH v2 1/1] target/rx: Check for page crossings in use_goto_tb()](https://www.mail-archive.com/qemu-devel@nongnu.org/msg708099.html)
- [[PATCH 0/3] Add Scripts for Finding Top 25 Executed Functions](https://www.mail-archive.com/qemu-devel@nongnu.org/msg713035.html)
  - [[PATCH 1/3] MAINTAINERS: Add 'Miscellaneous' section](https://www.mail-archive.com/qemu-devel@nongnu.org/msg713036.html)
  - [[PATCH 2/3] scripts/performance: Add callgrind_top_25.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg713037.html)
  - [[PATCH 3/3] scripts/performance: Add perf_top_25.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg713038.html)
- [[PATCH v2 0/3] Add Scripts for Finding Top 25 Executed Functions](https://www.mail-archive.com/qemu-devel@nongnu.org/msg714310.html)
  - [[PATCH v2 1/3] scripts/performance: Add topN_perf.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg714309.html)
  - [[PATCH v2 2/3] scripts/performance: Add topN_callgrind.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg714312.html)
  - [[PATCH v2 3/3] MAINTAINERS: Add 'Performance Tools and Tests' subsection](https://www.mail-archive.com/qemu-devel@nongnu.org/msg714314.html)
- [[PATCH v3 0/3] Add Scripts for Finding Top 25 Executed Functions](https://www.mail-archive.com/qemu-devel@nongnu.org/msg716011.html)
  - [[PATCH v3 1/3] scripts/performance: Add topN_perf.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg716010.html)
  - [[PATCH v3 2/3] scripts/performance: Add topN_callgrind.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg716014.html)
  - [[PATCH v3 3/3] MAINTAINERS: Add 'Performance Tools and Tests' subsection](https://www.mail-archive.com/qemu-devel@nongnu.org/msg716013.html)
- [[PATCH v4 0/3] Add Scripts for Finding Top 25 Executed Functions](https://www.mail-archive.com/qemu-devel@nongnu.org/msg717095.html)
  - [[PATCH v4 1/3] scripts/performance: Add topN_perf.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg717097.html)
  - [[PATCH v4 2/3] scripts/performance: Add topN_callgrind.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg717094.html)
  - [[PATCH v4 3/3] MAINTAINERS: Add 'Performance Tools and Tests' subsection](https://www.mail-archive.com/qemu-devel@nongnu.org/msg717099.html)
- [[PATCH 0/1] Add Script for Dissecting QEMU Execution](https://www.mail-archive.com/qemu-devel@nongnu.org/msg718461.html)
  - [[PATCH 1/1] scripts/performance: Add dissect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg718462.html)
- [[PATCH v2 0/1] Add Script for Dissecting QEMU Execution](https://www.mail-archive.com/qemu-devel@nongnu.org/msg719000.html)
  - [[PATCH v2 1/1] scripts/performance: Add dissect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg718999.html)
- [[PATCH v3 0/1] Add Script for Dissecting QEMU Execution](https://www.mail-archive.com/qemu-devel@nongnu.org/msg721457.html)
  - [[PATCH v3 1/1] scripts/performance: Add dissect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg721458.html)
- [[PATCH 0/2] Add list_fn_callees.py and list_helpers.py scripts](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723083.html)
  - [[PATCH 1/2] scripts/performance: Add list_fn_callees.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723086.html)
  - [[PATCH 2/2] scripts/performance: Add list_helpers.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723089.html)
- [[PATCH v2 0/2] Add list_fn_callees.py and list_helpers.py scripts](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723519.html)
  - [[PATCH v2 1/2] scripts/performance: Add list_fn_callees.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723518.html)
  - [[PATCH v2 2/2] scripts/performance: Add list_helpers.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg723520.html)
- [[PATCH 0/1] Add bisect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg724526.html)
  - [[PATCH 1/1] scripts/performance: Add bisect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg724527.html)
- [[PATCH 0/9] GSoC 2020 - TCG Continuous Benchmarking scripts and tools](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734487.html)
  - [[PATCH 1/9] scripts/performance: Refactor topN_perf.py](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734489.html)
  - [[PATCH 2/9] scripts/performance: Refactor topN_callgrind.py](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734492.html)
  - [[PATCH 3/9] scripts/performance: Refactor dissect.py](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734488.html)
  - [[PATCH 4/9] scripts/performance: Add list_fn_callees.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734486.html)
  - [[PATCH 5/9] scripts/performance: Add list_helpers.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734490.html)
  - [[PATCH 6/9] scripts/performance: Add bisect.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734493.html)
  - [[PATCH 7/9] tests/performance: Add nightly tests](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734495.html)
  - [[PATCH 8/9] MAINTAINERS: Add 'tests/performance' to 'Performance Tools and Tests' subsection](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734494.html)
  - [[PATCH 9/9] scripts/performance: Add topN_system.py script](https://www.mail-archive.com/qemu-devel@nongnu.org/msg734491.html)

**Bugs:**

- [#1880722 Problems related to checking page crossing in use_goto_tb()](https://bugs.launchpad.net/qemu/+bug/1880722)
- [#1880763 Missing page crossing check in use_goto_tb() for rx target](https://bugs.launchpad.net/qemu/+bug/1880763)
- [#1892081 Performance improvement when using "QEMU_FLATTEN" with softfloat type conversions](https://bugs.launchpad.net/qemu/+bug/1892081)
