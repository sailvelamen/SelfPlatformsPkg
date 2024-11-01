qemu-system-x86_64 \
    -m 2048 -M q35,smm=on \
    -serial mon:stdio -net none \
    -pflash "Build/SelfPlatformsPkg/SelfOvmf3264/DEBUG_GCC5/FV/OVMF.fd" \
    -hda fat:rw:"Build/dbgQemuEmulator/hda-contents"