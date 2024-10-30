# SelfPlatformsPkg

#### 介绍
```
SelfPlatformsPkg
├── ARM_Script                             # ARM #
│   ├── Build_ArmVirtQemu_Binary.sh  # 编译Arm平台固件
│   └── Run_ArmVirtQemu-AARCH64.sh   # Qemu运行Arm固件
├── RISC-V_Script                          # RISC-v #
│   ├── Build_JH7110_Binary.sh       # 编译StarFive JH7110平台固件 (edk2: https://github.com/starfive-tech/edk2)
│   ├── Build_U540_Binary.sh         # 编译SiFive U540平台固件
│   └── Run_SiFiveU540_RISC-V.sh     # Qemu运行SiFiveU540
├── X86_Script                             # X86 #
│   ├── Build_OvmfIa32X64_Binary.sh  # 编译OVMF平台固件
│   ├── Run_OvmfIa32X64_Log.sh       # Qemu运行OVMF固件
│   ├── Run_OvmfIa32X64_Stdio.bat
│   ├── SelfOvmfIa32X64.dsc          # 自定义OVMF
│   └── SelfOvmfIa32X64.fdf
└── Platform
    ├── SiFive/U5SeriesPkg/FreedomU540HiFiveUnleashedBoard  # 自定义 U540
    └── SelfPlatformsPkg/Platform/Starfive/JH7110SeriesPkg  # 自定义 JH7110 (适配官方edk2、 edk2-platforms)

```

#### 使用说明
设置环境变量：
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
将 SelfPlatformsPkg 文件夹放到与edk2、edk2-platforms 同一目录

编译固件(e.g.):
```
source SelfPlatformsPkg/X86/Build_OvmfIa32X64_Binary.sh
```
运行固件(e.g.):
```
source SelfPlatformsPkg/X86/Run_OvmfIa32X64_Stdio.bat
```
