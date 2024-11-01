qemu-system-aarch64 \
    -M virt -cpu cortex-a72 -smp 4,cores=2,threads=2,sockets=1 -m 4096 \
    -bios "Build/ArmVirtQemu-AARCH64/DEBUG_GCC5/FV/QEMU_EFI.fd" \
    -device virtio-gpu-pci -device nec-usb-xhci -device usb-mouse -device usb-kbd \
    -net none -serial stdio \
    -hda fat:rw:"Build/dbgQemuEmulator/hda-contents"
#     -net none -serial file:"Build/dbgQemuEmulator/hda-contents/ArmVirtQemu-AARCH64_4G.log" \
#     -drive file="Build/dbgQemuEmulator/debian-12-nocloud-arm64.qcow2" \
