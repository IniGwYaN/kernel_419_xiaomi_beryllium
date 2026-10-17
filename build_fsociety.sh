#!/bin/bash
# build_fsociety.sh [ksun|nosu] - build fsociety kernel for POCO F1 (beryllium).
# PROFILE=std (default: GPU 710 MHz, all thermal zones) or PROFILE=oc (GPU 802 MHz, relaxed thermal).
# Needs an LLVM/clang toolchain (tested: Neutron clang 24) in $CLANG_DIR/bin, plus aarch64/arm GNU binutils prefixes.
# Output: out[-nosu]/arch/arm64/boot/Image.gz-dtb (flash with AnyKernel3).
set -e
VARIANT=${1:-ksun}; S=$(cd "$(dirname "$0")" && pwd)
case $VARIANT in ksun) O=out ;; nosu) O=out-nosu ;; *) echo "usage: $0 [ksun|nosu]"; exit 1 ;; esac
PROFILE=${PROFILE:-std}
case $PROFILE in std) DTCF= ;; oc) DTCF=-DFSOCIETY_OC; O=$O-oc ;; *) echo "PROFILE=std|oc"; exit 1 ;; esac
[ -n "$CLANG_DIR" ] && export PATH=$CLANG_DIR/bin:$PATH
[ $VARIANT = ksun ] && [ ! -f "$S/KernelSU/kernel/Makefile" ] && git -C "$S" submodule update --init
M="make -C $S -j$(nproc) O=$O ARCH=arm64 SUBARCH=arm64 LLVM=1 LLVM_IAS=1 CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm HOSTCC=gcc HOSTCXX=g++ OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip CROSS_COMPILE=aarch64-linux-gnu- CROSS_COMPILE_ARM32=arm-linux-gnueabi- CLANG_TRIPLE=aarch64-linux-gnu- FSOCIETY_DTC_CPP_FLAGS=$DTCF"
export KCFLAGS="-Wno-error"
$M beryllium_defconfig
if [ $VARIANT = ksun ]; then
  # KSU_SUSFS_TRY_UMOUNT off: SUSFS v2.2.0 dropped susfs_add_try_umount that KernelSU-Next legacy-susfs-v2 still calls
  "$S/scripts/config" --file "$S/$O/.config" --set-str LOCALVERSION "-fsociety" -e KSU -e KSU_MANUAL_HOOK -e KSU_SUSFS -d KSU_SUSFS_TRY_UMOUNT
else
  "$S/scripts/config" --file "$S/$O/.config" --set-str LOCALVERSION "-fsociety-nosu" -d KSU -d KSU_SUSFS
fi
$M olddefconfig
$M
ls -la "$S/$O/arch/arm64/boot/Image.gz-dtb"
