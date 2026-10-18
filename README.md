# fsociety kernel 4.19 — POCO F1 (beryllium)

Custom 4.19.325 kernel for POCO F1, based on **Aozora 4.19** by Kaiyaa77.

## Builds

| Build | Root |
|---|---|
| **KSU** | KernelSU-Next 33219 + SUSFS (use KernelSU Next manager **v3.3.0**) |
| **NoSU** | no root |

Profiles: **STD** (GPU 710 MHz, thermal on) · **OC** (GPU 802 MHz, relaxed thermal)

## Fixes

- **Phone powers off on reboot.** Fixed with a warm reboot (tested 10–15 reboots).
- **USB not detected after replugging.** Fixed (tested 10–15 replugs, USB + MTP show up right away).
- **Battery drops or shuts off with charge left.** Fuel-gauge fix included, not tested yet.
- **Android 17 bootloop (`bpfloader-failed`)** on ROMs like Infinity-X 4.0. Fixed.
- **Chrome/Brave crash on Android 17.** Fixed.
- **Double-tap-to-wake** on FocalTech panels (original LCD).
- KernelSU-Next + SUSFS fixes (Zygisk detection, modules at cold boot).

Full details: [CHANGELOG.md](CHANGELOG.md)

## Notes

- For **4.19 ROMs** only (not 4.9).
- The version shows 5.10.245 (Aozora's spoof); the real base is 4.19.325.

## Credits

- **Kaiyaa77** (@Kaiyaa77Ar): Aozora 4.19 base
- **Kyy** (@kyzuna28): Kyzuna dtsi + KernelSU-Next fork
- **KernelSU-Next team**
- **simonpunk**: SUSFS
- **diphons**: SDM845 4.19 port
- **LineageOS / Bruno Martins**: warm reboot
- **Sultan Alsawaf**: cycle-count fix
- **Stanislav Fomichev**: upstream BPF bind patch
- **Beryllium-Labs / jiro28**: 4.19 backport reference

License: GPL-2.0 · by UMRSRN (@umrsrn1318)
