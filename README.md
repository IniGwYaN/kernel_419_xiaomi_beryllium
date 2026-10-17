# fsociety kernel 4.19 — POCO F1 (beryllium)

Custom 4.19.325 kernel for the Xiaomi POCO F1, based on **Aozora 4.19** by Kaiyaa77,
with fixes for the known "4.19 bugs" on SDM845.

Two root variants, each in two profiles:

| Variant | Root |
|---|---|
| **KSUN** | KernelSU-Next + SUSFS (use the KernelSU Next manager v3.3.0) |
| **NoSU** | no root |

| Profile | GPU max | Thermal |
|---|---|---|
| **STD** | 710 MHz (stock) | all thermal zones on |
| **OC** | 802 MHz | the 14 zones Aozora disables are off (SoC critical trips + LMH stay) |

## Changes on top of Aozora

- **Reboot fix:** every reboot is a warm reset via PS_HOLD (`qcom,force-warm-reboot`).
  On 4.19 the PMIC hard reset made some F1s power off instead of rebooting.
- **Bootloop fix with newer clang:** dropped `-mllvm -always-rename-promoted-locals=false`
  (ThinLTO then mixed up same-named static symbols, kernel panic at boot).
- **Thermal zones kept:** `beryllium-p0.dtsi` from Kyzuna (Aozora disables many of them).
- **Charger:** unknown APSD result gets 500 mA instead of 0 mA.
- **KernelSU-Next** (kyzuna28 `legacy-susfs-v2`, submodule) with the SUSFS inline hooks adapted.

Note: the version string shows `5.10.245` (Aozora's KERNELVERSION spoof); the real base is 4.19.325.

## Build

```sh
git clone --recursive <this repo>
cd <repo>
CLANG_DIR=/path/to/clang ./build_fsociety.sh ksun              # or: nosu
CLANG_DIR=/path/to/clang PROFILE=oc ./build_fsociety.sh ksun   # OC profile
```

Tested with Neutron clang 24. The output is `out/arch/arm64/boot/Image.gz-dtb`
(`out-nosu/` for NoSU); flash it with AnyKernel3.

## Credits

- Kaiyaa77: Aozora 4.19 (base)
- @Kyy / kyzuna28: Kyzuna 4.19 and the KernelSU-Next fork
- KernelSU-Next team
- simonpunk: SUSFS
- diphons: SDM845 4.19 port
- The Linux kernel and CodeLinaro / Qualcomm sources

Licensed under GPL-2.0, see `COPYING`.
