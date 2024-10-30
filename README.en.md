# SelfPlatformsPkg

#### Description
```
SelfPlatformsPkg
├── ARMARM_Script                             # ARM
│   ├── Build_ArmVirtQemu_Binary.sh  # build firmware for QemuArm
│   └── Run_ArmVirtQemu-AARCH64.sh   # Qemu runs Arm firmware
├── RISC-V_Script                          # RISC-v
│   ├── Build_JH7110_Binary.sh       # build firmware for JH7110 platform (edk2: https://github.com/starfive-tech/edk2)
│   ├── Build_U540_Binary.sh         # build firmware for U540 platform
│   └── Run_SiFiveU540_RISC-V.sh     # Qemu runs SiFive U540
├── X86_Script                             # X86
│   ├── Build_OvmfIa32X64_Binary.sh  # build OVMF platform firmware
│   ├── Run_OvmfIa32X64_Log.sh       # Qemu runs OVMF firmware
│   ├── Run_OvmfIa32X64_Stdio.bat
│   ├── SelfOvmfIa32X64.dsc          # Customize OVMF firmware
│   └── SelfOvmfIa32X64.fdf
└── Platform
    ├── SiFive/U5SeriesPkg/FreedomU540HiFiveUnleashedBoard  # Customize U540
    └── SelfPlatformsPkg/Platform/Starfive/JH7110SeriesPkg  # Customize JH7110 (Adapt to Tianocore's edk2 and edk2 platforms)
```

#### Instructions

Set environment variables:
```shell
export WORKSPACE=$PWD
export PYTHON_COMMAND=/usr/bin/python3
export EDK_TOOLS_PATH=$WORKSPACE/edk2/BaseTools
export CONF_PATH=$WORKSPACE/edk2/Conf
export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms:$WORKSPACE/edk2-non-osi:$WORKSPACE/edk2-libc:$WORKSPACE/SelfPkg:$WORKSPACE/SelfPlatformsPkg

NUM_CPUS=$((`getconf _NPROCESSORS_ONLN` + 2))
export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
# export GCC5_RISCV64_PREFIX=riscv64-linux-gnu-
export GCC5_RISCV64_PREFIX=riscv64-unknown-elf-

echo "NUM_CPUS            = "$NUM_CPUS
echo "GCC5_RISCV64_PREFIX = "$GCC5_RISCV64_PREFIX
echo "GCC5_AARCH64_PREFIX = "$GCC5_AARCH64_PREFIX
echo ""
source edk2/edksetup.sh
```
Place the SelfPlatformsPkg folder in the same directory as edk2 and edk2 platforms

Build firmware (e.g.):
```
source SelfPlatformsPkg/X86/Build_OvmfIa32X64_Binary.sh
```
Run firmware (e.g.):
```
source SelfPlatformsPkg/X86/Run_OvmfIa32X64_Stdio.bat
```