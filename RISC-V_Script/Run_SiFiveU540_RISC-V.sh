qemu-system-riscv64 \
    -cpu sifive-u54 -machine sifive_u -m 4096 -smp cpus=4,maxcpus=4 \
    -bios "Build/SelfPlatformsPkg/FreedomU540HiFiveUnleashed/DEBUG_GCC5/FV/U540.fd" \
    -net none -serial stdio \
    -hda fat:rw:"Build/dbgQemuEmulator/hda-contents"
    # -serial file:"Build/dbgQemuEmulator/hda-contents/SiFiveU540_RISC-V.log" \
    # -drive file="/home/kubuntu/Downloads/OS_Images/RISC-V/ubuntu-24.04.1-preinstalled-server-riscv64+unmatched.img" \
