# SelfPlatformsPkg

#### Description
```
SelfPlatformsPkg
├── ARM                             # ARM
│   ├── Build_ArmVirtQemu_Binary.sh  # build firmware for QemuArm
│   └── Run_ArmVirtQemu-AARCH64.sh   # Qemu runs Arm firmware
├── RISC-V                          # RISC-v
│   ├── Build_JH7110_Binary.sh       # build firmware for JH7110 platform (edk2: https://github.com/starfive-tech/edk2)
│   ├── Build_U540_Binary.sh         # build firmware for U540 platform
│   ├── JH7110Board                  # Customize Device Tree
│   ├── Library
│   ├── Run_SiFiveU540_RISC-V.sh     # Qemu runs SiFive U540
│   └── Universal
└── X86                             # X86
    ├── Build_OvmfIa32X64_Binary.sh  # build OVMF platform firmware
    ├── Run_OvmfIa32X64_Log.sh       # Qemu runs OVMF firmware
    ├── Run_OvmfIa32X64_Stdio.bat
    ├── SelfOvmfIa32X64.dsc          # Customize OVMF firmware
    └── SelfOvmfIa32X64.fdf
```

#### Instructions

Set environment variables:
```shell
WORKAREA=$PWD
export EDK_TOOLS_PATH=$WORKAREA/edk2/BaseTools
export CONF_PATH=$WORKAREA/edk2/Conf
export PACKAGES_PATH=$WORKAREA/edk2:$WORKAREA/edk2-platforms:$WORKAREA/edk2-non-osi:$WORKAREA/edk2-libc:$WORKAREA/SelfPkg:$WORKAREA/SelfPlatformsPkg
echo ""
NUM_CPUS=$((`getconf _NPROCESSORS_ONLN` + 2))
echo "NUM_CPUS            = "$NUM_CPUS
export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
echo "GCC5_AARCH64_PREFIX = "$GCC5_AARCH64_PREFIX
export GCC5_RISCV64_PREFIX=riscv64-linux-gnu-
echo "GCC5_RISCV64_PREFIX = "$GCC5_RISCV64_PREFIX
echo ""
source edk2/edksetup.sh
```

Build firmware (e.g.):
```
source SelfPlatformsPkg/X86/Build_OvmfIa32X64_Binary.sh
```
Run firmware (e.g.):
```
source SelfPlatformsPkg/X86/Run_OvmfIa32X64_Stdio.bat
```