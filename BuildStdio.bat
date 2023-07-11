CHCP 65001
qemu-system-x86_64 -pflash "Build\SelfOvmf3264\DEBUG_VS2019\FV\OVMF.fd" -hda fat:rw:"Build\dbgOvmfX64\hda-contents" -serial mon:stdio -net none -m 1024 -M q35,smm=on