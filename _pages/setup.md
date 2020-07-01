---
layout: page
title: Setting Up the Reports Testbed
permalink: /setup/
---

The sections below cover all of the technical information, dependencies, and setup instructions required to have a testbed identical to the one used in the reports.

**System Information:**

- Architecture: **x86_64**
- CPU Codename: **Coffee Lake**
- CPU Model: **Intel Core i7-8750H**
- Memory: **16GB DDR4 2667 MHz**
- OS: **Ubuntu MATE 18.04.4 LTS**

<hr />

**QEMU Dependencies:**

Below is the list of all Linux dependencies required to build QEMU. Please copy and paste the command in your terminal.

```bash
sudo apt-get install  \
autoconf  \
build-essential  \
git \
git-email  \
kcachegrind \
libaio-dev  \
libasound2-dev  \
libavcodec-dev  \
libbluetooth-dev  \
libbrlapi-dev  \
libbz2-dev  \
libcap-dev  \
libcap-ng-dev  \
libcurl4-gnutls-dev  \
libfdt-dev  \
libfontconfig1-dev  \
libfreetype6-dev  \
libgcrypt20-dev  \
libglib2.0-dev  \
libglu1-mesa-dev  \
libgtk-3-dev  \
libibverbs-dev  \
libiscsi-dev  \
libjpeg8-dev  \
liblog4cpp5-dev  \
liblzo2-dev  \
libncurses5-dev  \
libnfs-dev  \
libnuma-dev  \
libogg-dev  \
libpixman-1-dev  \
libpng-dev  \
librbd-dev  \
librdmacm-dev  \
libsasl2-dev  \
libsdl2-dev  \
libseccomp-dev  \
libsnappy-dev  \
libssh2-1-dev  \
libssl-dev  \
libtool  \
libvde-dev  \
libvdeplug-dev  \
libvte-2.91-dev  \
libxen-dev  \
libxrandr-dev  \
linux-tools-common \
qemu  \
qemu-kvm  \
valgrind  \
valkyrie \
xfslibs-dev  \
zlib1g-dev
```

<hr />

**QEMU Setup:**

All reports are based on QEMU version [5.0.0](https://git.qemu.org/?p=qemu.git;a=commit;h=fdd76fecdde1ad444ff4deb7f1c4f7e4a1ef97d6) with a basic GCC build unless other wise stated in the report.

The bash snippet below downloads QEMU, extracts the archive, and creates a basic GCC build.

```bash
wget https://download.qemu.org/qemu-5.0.0.tar.xz
tar xfv qemu-5.0.0.tar.xz
cd qemu-5.0.0
mkdir build-gcc
cd build-gcc
../configure
```

<hr />

**Cross Compilers Setup:**

Below is a list of all 17 compilers that are used in the reports. Please copy and paste the command in your terminal.

```bash
sudo apt-get install  \
g++  \
g++-aarch64-linux-gnu  \
g++-alpha-linux-gnu  \
g++-arm-linux-gnueabi  \
g++-hppa-linux-gnu  \
g++-m68k-linux-gnu  \
g++-mips-linux-gnu  \
g++-mips64-linux-gnuabi64  \
g++-mips64el-linux-gnuabi64  \
g++-mipsel-linux-gnu  \
g++-powerpc-linux-gnu  \
g++-powerpc64-linux-gnu  \
g++-powerpc64le-linux-gnu  \
g++-riscv64-linux-gnu  \
g++-s390x-linux-gnu  \
g++-sh4-linux-gnu  \
g++-sparc64-linux-gnu
```
