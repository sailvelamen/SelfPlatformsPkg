# SelfOvmfPkg

#### Description
Self OVMF Package

#### Software Architecture
Project from edk2/OvmfPkg/OvmfPkgIa32X64.dsc 和 OvmfPkgIa32X64.fdf

#### Installation

1.  Install ASL NASM
2.  Copy SelfOvmfPkg to Workspace

#### Instructions

1.  build -a IA32 -a X64 -p SelfOvmfPkg\SelfOvmfIa32X64.dsc # Direct build
2.  build -a IA32 -a X64 -p SelfOvmfPkg\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT # Enable log output
3.  build -a IA32 -a X64 -p SelfOvmfPkg\SelfOvmfIa32X64.dsc -D DEBUG_ON_SERIAL_PORT -D SMM_REQUIRE=TRUE # Enable SMM and log output