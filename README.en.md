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


#### Gitee Feature

1.  You can use Readme\_XXX.md to support different languages, such as Readme\_en.md, Readme\_zh.md
2.  Gitee blog [blog.gitee.com](https://blog.gitee.com)
3.  Explore open source project [https://gitee.com/explore](https://gitee.com/explore)
4.  The most valuable open source project [GVP](https://gitee.com/gvp)
5.  The manual of Gitee [https://gitee.com/help](https://gitee.com/help)
6.  The most popular members  [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
