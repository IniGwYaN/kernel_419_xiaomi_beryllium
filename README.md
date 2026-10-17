# fsociety kernel 4.19 — POCO F1 (beryllium)

Custom 4.19.325 kernel for the Xiaomi POCO F1, based on **Aozora 4.19** by Kaiyaa77,
with fixes for the known "4.19 bugs" on SDM845.

Two root variants, each in two profiles:

| Variant | Root |
|---|---|
| **KSUN** | KernelSU-Next 3.4.2 legacy (UAPI 5) + SUSFS 2.3.0 — use the KernelSU Next manager **v3.4.2** (normal or spoofed) |
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
- **Android 17 apps:** `CAP_CHECKPOINT_RESTORE` (cap 40) backported. A17's Rust native app zygote
  aborts when the cap bounding set stops at 39 (Chrome/Brave force close).
- **Namespaces/cgroups:** PID/IPC namespaces, SysV IPC, POSIX mqueue, pids/devices/net_prio cgroups on.
- **Double tap to wake (FocalTech panels):** the touch driver now enters gesture mode on the early
  blank event, before the in-cell panel powers off, and keeps the panel up like the Novatek driver.
  Before, every screen-off ended in i2c NACKs and DT2W never worked.
- **Battery:** the capacity read never fails any more (healthd turns a failed read into 0 % and
  Android powers off: "dies with charge left"); the charger/FG awake votes hold real wakelocks again
  instead of 500 ms timeouts; every cycle-count bucket is cleared in SRAM.
- **QC2 thermal table:** level 10 was 10.255 A (Xiaomi typo), now 1.025 A.
- **Per-app battery stats:** `CONFIG_PROFILING` so `/proc/uid_cputime` and `/proc/uid_io` exist.
- **KernelSU-Next** (official `legacy` branch, submodule, + SUSFS glue) with SUSFS 2.3.0:
  hides sus mounts from A17 `zygote_next` children, `no_su` process flag, init.rc size hook for A17
  (`ksu_handle_newfstat_ret`), all SUSFS features on.

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
- @Kyy / kyzuna28: Kyzuna 4.19 (thermal/charger DT)
- KernelSU-Next team (rifsxd and contributors)
- star-star-dev: SUSFS glue for KernelSU-Next legacy
- simonpunk: SUSFS
- LineageOS (Bruno Martins): `qcom,force-warm-reboot` idea on SDM845
- Sultan Alsawaf: FG cycle-count clear fix
- diphons: SDM845 4.19 port
- The Linux kernel and CodeLinaro / Qualcomm sources

Licensed under GPL-2.0, see `COPYING`.
