# SelfPlatformsPkg

#### 介绍
```
SelfPlatformsPkg
├── ARM                             # ARM #
│   ├── Build_ArmVirtQemu_Binary.sh  # 编译Arm平台固件
│   └── Run_ArmVirtQemu-AARCH64.sh   # Qemu运行Arm固件
├── RISC-V                          # RISC-v #
│   ├── Build_JH7110_Binary.sh       # 编译JH7110平台固件 (edk2: https://github.com/starfive-tech/edk2)
│   ├── Build_U540_Binary.sh         # 编译U540平台固件
│   ├── JH7110Board                  # 自定义设备树
│   ├── Library
│   ├── Run_SiFiveU540_RISC-V.sh     # Qemu运行SiFiveU540
│   └── Universal
└── X86                             # X86 #
    ├── Build_OvmfIa32X64_Binary.sh  # 编译OVMF平台固件
    ├── Run_OvmfIa32X64_Log.sh       # Qemu运行OVMF固件
    ├── Run_OvmfIa32X64_Stdio.bat
    ├── SelfOvmfIa32X64.dsc          # 自定义OVMF
    └── SelfOvmfIa32X64.fdf
```

#### 使用说明
设置环境变量：
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

编译固件(e.g.):
```
source SelfPlatformsPkg/X86/Build_OvmfIa32X64_Binary.sh
```
运行固件(e.g.):
```
source SelfPlatformsPkg/X86/Run_OvmfIa32X64_Stdio.bat
```
