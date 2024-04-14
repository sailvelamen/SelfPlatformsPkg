# SelfPlatformsPkg

#### 介绍
Self X86 OVMF Package
Self RISC-V JH7110 Package

#### 软件架构
项目来自 edk2/OvmfPkg/OvmfPkgIa32X64.dsc 和 OvmfPkgIa32X64.fdf


#### 安装教程

1.  安装 ASL NASM
2.  复制 SelfPlatformsPkg 到工作目录

#### 使用说明

1.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc # 直接编译
2.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT # 开启 log 输出
3.  build -a IA32 -a X64 -b DEBUG -p SelfPlatformsPkg\X86\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT -D SMM_REQUIRE=TRUE # 开启 SMM 和 log 输出