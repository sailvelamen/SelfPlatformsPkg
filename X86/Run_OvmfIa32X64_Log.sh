qemu-system-x86_64 -pflash "Build/SelfPlatformsPkg/SelfOvmf3264/DEBUG_GCC5/FV/OVMF.fd" -hda fat:rw:"Build/dbgQemuEmulator/hda-contents" -serial file:"Build/dbgQemuEmulator/hda-contents/OvmfPkgIa32X64_4G.log" -net none -m 4096 -M q35,smm=on 