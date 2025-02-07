/** @file
  Memory Detection for Virtual Machines.

  Copyright (c) 2021, Hewlett Packard Enterprise Development LP. All rights reserved.<BR>
  Copyright (c) 2006 - 2014, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

  MemDetect.c

**/

//
// The package level header files this module uses
//
#include <PiPei.h>

//
// The Library classes this module consumes
//
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/ResourcePublicationLib.h>
#include <Library/RiscVFirmwareContextLib.h>
#include <Library/I2cLib.h>
#include <libfdt.h>
#include <Guid/FdtHob.h>
#include "Platform.h"

#define STARFIVE_JH7110_EEPROM_DDRINFO_OFFSET	91
#define I2C_EEPROM_ADDR (FixedPcdGet8 (PcdI2cEepromAddress))
#define I2C_Controller "i2c5"

#ifndef DW_I2C_LIB_H_
#define DW_I2C_LIB_H_
//
// The maximum number of I2C bus
//
#define I2C_MAX_BUS_NUM  2
#endif /* DW_I2C_LIB_H_ */

EFI_STATUS
EFIAPI
GetBaseAddress (
  IN OUT UINT64 *I2CBase,
  IN OUT UINT64 *MemBase
  )
{
  INT32                               Len;
  INT32                               Node;
  fdt32_t                             Size;
  UINT64                              reg;
  UINT32                              ParentOffset, ChosenOffset, Err;
  VOID                                *DtbBlob;
  VOID                                *FdtPointer;
  CONST CHAR8                         *NodePath;
  CONST CHAR8                         *DeviceType;
  CONST UINT64                        *RegProp;
  CONST UINT64                        *PropReg;
  EFI_RISCV_OPENSBI_FIRMWARE_CONTEXT  *FirmwareContext;

  //
  // Get DeviceTree Address
  //
  FirmwareContext = NULL;
  GetFirmwareContextPointer (&FirmwareContext);
  if (FirmwareContext == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: OpenSBI Firmware Context is NULL\n", __func__));
    return EFI_UNSUPPORTED;
  }

  FdtPointer = (VOID *)FirmwareContext->FlattenedDeviceTree;
  if (FdtPointer == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Invalid FDT pointer\n", __func__));
    return EFI_UNSUPPORTED;
  }
  DtbBlob = NULL;
  DEBUG ((DEBUG_ERROR, "%a: Build FDT HOB - FDT at address: 0x%x \n", __func__, FdtPointer));
  DtbBlob = FdtPointer;
  ASSERT (DtbBlob != NULL);
  ASSERT (fdt_check_header (DtbBlob) == 0);

  Size = fdt_totalsize (DtbBlob);
  Err  = fdt_open_into (DtbBlob, DtbBlob, Size + 32);
  if (Err < 0) {
    DEBUG ((
      DEBUG_ERROR,
      "Device Tree can't be expanded to accommodate new node\n",
      __func__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get I2C reg address
  //
  NodePath = fdt_get_alias (DtbBlob, I2C_Controller);
  if (NodePath == NULL) {
    DEBUG ((DEBUG_ERROR, "%a:%d RETURN_NOT_FOUND \n", __FUNCTION__, __LINE__));
    return RETURN_NOT_FOUND;
  }

  ChosenOffset = fdt_path_offset (DtbBlob, NodePath);
  if (ChosenOffset < 0) {
    DEBUG ((DEBUG_ERROR, "%a:%d Node not found:%d \n", __FUNCTION__, __LINE__, ChosenOffset));
    return EFI_NOT_FOUND;
  }

  RegProp = fdt_getprop (DtbBlob, ChosenOffset, "reg", &Len);
  if ((RegProp == NULL) && (Len != (2 * sizeof (fdt64_t)))) {
    DEBUG ((DEBUG_ERROR, "%a:%d RegProp EFI_NOT_FOUND \n", __FUNCTION__, __LINE__));
    return EFI_NOT_FOUND;
  }
  reg = fdt64_to_cpu (ReadUnaligned64 (RegProp));
  DEBUG ((DEBUG_ERROR, "%a:%d I2C reg = 0x%x Len = %d \n", __FUNCTION__, __LINE__, reg, Len));
  *I2CBase = reg;

  //
  // Get Memory Base Address
  //
  ParentOffset = fdt_path_offset(FdtPointer, "/");
  if (ParentOffset < 0) {
    DEBUG ((DEBUG_ERROR, "%a:%d Node not found:%d \n", __FUNCTION__, __LINE__, ParentOffset));
    return EFI_NOT_FOUND;
  }

  fdt_for_each_subnode (Node, FdtPointer, ParentOffset) {
    DeviceType = fdt_getprop (FdtPointer, Node, "device_type", NULL);
    if (DeviceType && AsciiStrCmp (DeviceType, "memory") == 0) {
      DEBUG ((DEBUG_INFO, "Found memory node at offset: %d\n", Node));

      PropReg = fdt_getprop(FdtPointer, Node, "reg", &Len);
      if (!PropReg) {
          DEBUG((DEBUG_ERROR, "Error: 'reg' property not found.\n"));
          return EFI_NOT_FOUND;
      }

      reg = fdt64_to_cpu (ReadUnaligned64 (PropReg));
      DEBUG ((DEBUG_ERROR, "%a:%d Mem reg = 0x%x Len = %d \n", __FUNCTION__, __LINE__, reg, Len));
      *MemBase = reg;
      return EFI_SUCCESS;
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DetectMem (
  UINT64  *MemBase,
  UINT64  *MemSize
  )
{
  EFI_STATUS        Status;
  UINTN             DataLen;
  UINT8             ReadTemp[0x90];
  UINT8             WriteTemp[0x90];
  UINT64            I2CBase;
  DW_I2C_CONTEXT_T  mI2cBusList[I2C_MAX_BUS_NUM];

  UINT32 Bus = FixedPcdGet8 (PcdSmbusI2cBusNumber);
  UINTN  BusSpeed = FixedPcdGet32 (PcdSmbusI2cBusSpeed);

  I2CBase = 0;
  GetBaseAddress (&I2CBase, MemBase);
  mI2cBusList[Bus].Base = (UINTN)I2CBase;
  DEBUG ((DEBUG_ERROR, "%a %d 0x%x \n", __FUNCTION__, __LINE__, mI2cBusList[Bus].Base));

  //
  // Switch to correct I2C bus and speed
  //
  Status = I2cProbe (Bus, BusSpeed, FALSE, FALSE, mI2cBusList);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d Status = %r \n", __FUNCTION__, __LINE__, Status));
    // return EFI_DEVICE_ERROR;
  }
  WriteTemp[0] = 0x0;
  DataLen      = 0x90;
  Status       = I2cRead (
                    Bus,
                    I2C_EEPROM_ADDR,
                    WriteTemp,
                    0,
                    ReadTemp,
                    (UINT32 *)&DataLen,
                    mI2cBusList
                    );
  if (EFI_ERROR (Status)) {
    if (Status != EFI_TIMEOUT) {
      Status = EFI_DEVICE_ERROR;
      DEBUG ((DEBUG_ERROR, "%a:%d Status = %r \n", __FUNCTION__, __LINE__, Status));
    }
  }
  // DEBUG ((DEBUG_INFO, "SmBus read: \n"));
  // for (UINTN Idx = 0; Idx < DataLen; Idx++) {
  //   DEBUG ((DEBUG_INFO, " %02x ", ReadTemp[Idx]));
  //   if ((Idx + 1) % 16 == 0)
  //     DEBUG ((DEBUG_INFO, "\n"));
  // }
  // DEBUG ((DEBUG_INFO, "\n"));

  *MemSize = ReadTemp[STARFIVE_JH7110_EEPROM_DDRINFO_OFFSET];
  *MemSize = AsciiStrHexToUintn ((CHAR8 *)MemSize);
  DEBUG ((DEBUG_ERROR, "%a:%d MemSize = 0x%x \n", __FUNCTION__, __LINE__, *MemSize));
  *MemSize = *MemSize << 30;

  return EFI_SUCCESS;
}

/**
  Publish PEI core memory.

  @return EFI_SUCCESS     The PEIM initialized successfully.

**/
EFI_STATUS
PublishPeiMemory (
  EFI_PHYSICAL_ADDRESS  *MemoryBase,
  UINT64                *MemorySize
  )
{
  EFI_STATUS            Status;
  UINT64                MemSize;
  UINT64                MemBase;

  DetectMem (&MemBase, &MemSize);

  //
  // TODO: This value should come from platform
  // configuration or the memory sizing code.
  //
  *MemoryBase = MemBase + 0x1000000UL;
  *MemorySize = MemSize - 0x1000000UL; // MemSize - 16MB

  DEBUG ((DEBUG_INFO, "%a: MemoryBase:0x%x MemorySize:0x%lx\n", __func__, *MemoryBase, *MemorySize));

  //
  // Publish this memory to the PEI Core
  //
  Status = PublishSystemMemory (*MemoryBase, *MemorySize);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Publish system RAM and reserve memory regions.

**/
VOID
InitializeRamRegions (
  EFI_PHYSICAL_ADDRESS  *MemoryBase,
  UINT64                *MemorySize
  )
{
  //
  // TODO: This value should come from platform
  // configuration or the memory sizing code.
  //
  AddMemoryRangeHob (*MemoryBase, *MemoryBase + *MemorySize);
}
