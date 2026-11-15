### AnyKernel3 Ramdisk Mod Script
## osm0sis @ xda-developers

### AnyKernel setup
# begin properties
properties() { '
kernel.string=AKEDORE kernel 4.19 v1 NoSU UMUM
do.devicecheck=1
do.modules=0
do.systemless=1
do.cleanup=1
do.cleanuponabort=0
device.name1=beryllium
supported.versions=
supported.patchlevels=
'; } # end properties

### AnyKernel install
# begin attributes
attributes() {
set_perm_recursive 0 0 755 644 $ramdisk/*;
set_perm_recursive 0 0 750 750 $ramdisk/init* $ramdisk/sbin;
} # end attributes


## boot shell variables
block=/dev/block/bootdevice/by-name/boot;
is_slot_device=0;
ramdisk_compression=auto;
patch_vbmeta_flag=auto;

# import functions/variables and setup patching - see for reference (DO NOT REMOVE)
. tools/ak3-core.sh && attributes;

# boot install
dump_boot; # use split_boot to skip ramdisk unpack, e.g. for devices with init_boot ramdisk

# kernel-line guard: the kernel in boot tells which vendor line this ROM is (our 4.19 reports 5.10.245)
# strings, not tr -c '[:print:]': recovery tr (toybox and busybox) prints nothing for that (OFox, 2026-10-27)
kver=$(strings $split_img/kernel 2>/dev/null | grep -m1 '^Linux version [0-9]' | cut -d' ' -f3);
[ -z "$kver" ] && kver=$(gzip -dc < $split_img/kernel 2>/dev/null | strings | grep -m1 '^Linux version [0-9]' | cut -d' ' -f3);
case "$kver" in
  4.9.*) kline=4.9;;
  4.19.*|5.10.*) kline=4.19;;
  *) kline=;;
esac;
ui_print "- Kernel in boot: ${kver:-unknown}";
if [ -n "$kline" ] && [ "$kline" != "4.19" ]; then
  abort "! This zip is for 4.19 ROMs, this ROM is $kline. Use the AKEDORE $kline zip.";
fi;

ui_print "- Unpacking boot image";
ui_print "- Flashing AKEDORE kernel";

write_boot; # use flash_boot to skip ramdisk repack, e.g. for devices with init_boot ramdisk

ui_print "- Done. Reboot to system";
ui_print " ";
## end boot install


## init_boot shell variables
#block=init_boot;
#is_slot_device=1;
#ramdisk_compression=auto;
#patch_vbmeta_flag=auto;

# reset for init_boot patching
#reset_ak;

# init_boot install
#dump_boot; # unpack ramdisk since it is the new first stage init ramdisk where overlay.d must go

#write_boot;
## end init_boot install


## vendor_kernel_boot shell variables
#block=vendor_kernel_boot;
#is_slot_device=1;
#ramdisk_compression=auto;
#patch_vbmeta_flag=auto;

# reset for vendor_kernel_boot patching
#reset_ak;

# vendor_kernel_boot install
#split_boot; # skip unpack/repack ramdisk, e.g. for dtb on devices with hdr v4 and vendor_kernel_boot

#flash_boot;
## end vendor_kernel_boot install


## vendor_boot shell variables
#block=vendor_boot;
#is_slot_device=1;
#ramdisk_compression=auto;
#patch_vbmeta_flag=auto;

# reset for vendor_boot patching
#reset_ak;

# vendor_boot install
#dump_boot; # use split_boot to skip ramdisk unpack, e.g. for dtb on devices with hdr v4 but no vendor_kernel_boot

#write_boot; # use flash_boot to skip ramdisk repack, e.g. for dtb on devices with hdr v4 but no vendor_kernel_boot
## end vendor_boot install
