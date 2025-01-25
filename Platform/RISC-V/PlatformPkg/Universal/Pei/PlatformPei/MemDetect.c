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
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/ResourcePublicationLib.h>
#include <Library/I2cLib.h>

#include "Platform.h"

#define STARFIVE_JH7110_EEPROM_DDRINFO_OFFSET	91
#define I2C_EEPROM_ADDR (FixedPcdGet8 (PcdI2cEepromAddress))

UINT64 DetectMem (
  VOID
)
{
  EFI_STATUS  Status;
  UINTN       DataLen;
  UINT8       ReadTemp[0x90];
  UINT8       WriteTemp[0x90];
  UINT64      memsize;
  DW_I2C_CONTEXT_T  mI2cBusList[I2C_MAX_BUS_NUM];

  UINT32 Bus = FixedPcdGet8 (PcdSmbusI2cBusNumber);
  UINTN  BusSpeed = FixedPcdGet32 (PcdSmbusI2cBusSpeed);

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

  memsize = ReadTemp[STARFIVE_JH7110_EEPROM_DDRINFO_OFFSET];
  memsize = AsciiStrHexToUintn ((CHAR8 *)&memsize);
  DEBUG ((DEBUG_ERROR, "%a:%d memsize = 0x%x \n", __FUNCTION__, __LINE__, memsize));

  return memsize << 30;
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
  UINT64                memsize;

  memsize = DetectMem ();

  //
  // TODO: This value should come from platform
  // configuration or the memory sizing code.
  //
  *MemoryBase = 0x40000000UL + 0x1000000UL;
  *MemorySize = memsize - 0x1000000UL; // memsize - 16MB

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
