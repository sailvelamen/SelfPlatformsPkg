# SelfPlatformsPkg

#### Description
Self X86 OVMF Package
Self RISC-V JH7110 Package

#### Software Architecture
Project from edk2/OvmfPkg/OvmfPkgIa32X64.dsc 和 OvmfPkgIa32X64.fdf

#### Installation

1.  Install ASL NASM
2.  Copy SelfPlatformsPkg to Workspace

#### Instructions

1.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc # 直接编译
2.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT # 开启 log 输出
3.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT -D SMM_REQUIRE=TRUE # 开启 SMM 和 log 输出