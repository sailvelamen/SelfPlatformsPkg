/** @file

  Copyright (c) 2020 - 2024, Ampere Computing LLC. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>

#include <Library/DebugLib.h>
// #include <Library/DxeServicesTableLib.h>
// #include <Library/HobLib.h>
#include <Library/I2cLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
// #include <Library/UefiBootServicesTableLib.h>
// #include <Library/UefiRuntimeLib.h>

//
// The base address of DW I2C
//
#define I2C_BASE_ADDRESS_LIST  0x10050000, 0x12050000

//
// Timeout interval
//
// The interval is equal to the 10 times the signaling period
// for the highest I2C transfer speed used in the system.
//
#define DW_POLL_INTERVAL_US(x)  (10 * (1000000 / (x)))

#define DW_MAX_STATUS_POLL_COUNT    100
#define DW_I2C_MAXIMUM_SPEED_HZ  400000

//
// Registers
//
#define DW_IC_CON                  0x00
#define DW_IC_TAR                  0x04
#define DW_IC_SAR                  0x08
#define DW_IC_DATA_CMD             0x10
#define DW_IC_SS_SCL_HCNT          0x14
#define DW_IC_SS_SCL_LCNT          0x18
#define DW_IC_INTR_MASK            0x30
#define DW_IC_RAW_INTR_STAT        0x34
#define DW_IC_RX_TL                0x38
#define DW_IC_TX_TL                0x3c
#define DW_IC_CLR_STOP_DET         0x60
#define DW_IC_ENABLE               0x6c
#define DW_IC_STATUS               0x70
#define DW_IC_SDA_HOLD             0x7c
#define DW_IC_ENABLE_STATUS        0x9c
#define DW_IC_COMP_PARAM_1         0xf4
#define DW_IC_DATA_CMD_DAT_MASK    0xFF
#define DW_IC_COMP_PARAM_1_RX_BUFFER_DEPTH(x) ((((x) >> 8) & 0xFF) + 1)
#define DW_IC_COMP_PARAM_1_TX_BUFFER_DEPTH(x) ((((x) >> 16) & 0xFF) + 1)

#define DW_IC_CON_SPEED_STD         BIT1
#define DW_IC_DATA_CMD_CMD          BIT8
#define DW_IC_DATA_CMD_STOP         BIT9

/* i2c enable register definitions */
#define IC_ENABLE_0B		0x0001

/* i2c control register definitions */
#define IC_CON_SD		      0x0040
#define IC_CON_RE		      0x0020
#define IC_CON_SPD_MSK		0x0006
#define IC_CON_SPD_SS		  0x0002
#define IC_CON_SPD_FS		  0x0004
#define IC_CON_SPD_HS		  0x0006
#define IC_CON_MM		      0x0001

/* i2c interrupt status register definitions */
#define IC_STOP_DET		0x0200

/* fifo threshold register definitions */
#define IC_TL0			0x00
#define IC_RX_TL		IC_TL0
#define IC_TX_TL		IC_TL0

/* i2c status register  definitions */
#define IC_STATUS_TFE		0x0004
#define IC_STATUS_RFNE		0x0008
#define IC_STATUS_MA		0x0020

/* Worst case timeout for 1 byte is kept as 2ms */
#define I2C_BYTE_TO		(1000/500)
#define I2C_STOPDET_TO		(1000/500)
#define I2C_BYTE_TO_BB		(I2C_BYTE_TO * 16)

// //
// // Private I2C bus data
// //
// typedef struct {
//   UINTN      Base;
//   UINT32     BusSpeed;
//   UINT32     RxFifo;
//   UINT32     TxFifo;
//   UINT32     PollingTime;
//   UINT32     Enabled;
//   BOOLEAN    IsSmbus;
//   BOOLEAN    PecCheck;
// } DW_I2C_CONTEXT_T;


//
// I2C SCL counter macros
//
typedef enum {
  I2cSpeedModeStandard = 0,
  I2cSpeedModeFast,
} I2C_SPEED_MODE;

typedef enum {
  I2cSclSpkLen = 0,
  I2cSclHcnt,
  I2cSclLcnt,
} I2C_SCL_PARAM;

STATIC UINT32   I2cSclParam[][3] = {
  /* SPK_LEN, HCNT, LCNT */
  [I2cSpeedModeStandard] = { 15, 74, 363 },   // SS (Standard Speed)
  [I2cSpeedModeFast]     = { 10, 52,  89 },   // FS (Fast Speed)
};

STATIC UINTN             mI2cBaseArray[I2C_MAX_BUS_NUM] = {I2C_BASE_ADDRESS_LIST};
STATIC UINTN             mI2cClock                      = 49500000U;

UINT64
EFIAPI
GetTimeInMilliSecond (
  IN      UINT64  Ticks
  )
{
  // DEBUG ((DEBUG_INFO, "%a: Ticks = 0x%llx \n", __FUNCTION__, Ticks));
  return GetTimeInNanoSecond (Ticks) / 1000000U; 
}

/**
 Initialize I2C Bus
 **/
VOID
I2cHWInit (
  UINT32  Bus,
  DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINT32  Param;
  DEBUG ((DEBUG_ERROR, "%a:%d  \n", __FUNCTION__, __LINE__));
  mI2cBusList[Bus].Base = mI2cBaseArray[Bus];

  Param = MmioRead32 (mI2cBusList[Bus].Base + DW_IC_COMP_PARAM_1);

  mI2cBusList[Bus].PollingTime = DW_POLL_INTERVAL_US (mI2cBusList[Bus].BusSpeed);
  mI2cBusList[Bus].RxFifo      = DW_IC_COMP_PARAM_1_RX_BUFFER_DEPTH (Param);
  mI2cBusList[Bus].TxFifo      = DW_IC_COMP_PARAM_1_TX_BUFFER_DEPTH (Param);
  mI2cBusList[Bus].Enabled     = 0;

  DEBUG ((
    DEBUG_VERBOSE,
    "%a: Bus %d, Rx_Buffer %d, Tx_Buffer %d PollingTime 0x%lx\n",
    __func__,
    Bus,
    mI2cBusList[Bus].RxFifo,
    mI2cBusList[Bus].TxFifo,
    mI2cBusList[Bus].PollingTime
    ));
}

/**
 Enable or disable I2C Bus
 */
VOID
I2cEnable (
  UINT32  Bus,
  UINT32  Enable,
  DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINT32  I2cStatusCnt;
  UINTN   Base;

  Base                     = mI2cBusList[Bus].Base;
  I2cStatusCnt             = DW_MAX_STATUS_POLL_COUNT;
  mI2cBusList[Bus].Enabled = Enable;

  MmioWrite32 (Base + DW_IC_ENABLE, Enable);

  do {
    if ((MmioRead32 (Base + DW_IC_ENABLE_STATUS) & 0x01) == Enable) {
      break;
    }

    MicroSecondDelay (mI2cBusList[Bus].PollingTime);
  } while (I2cStatusCnt-- != 0);

  if (I2cStatusCnt == 0) {
    DEBUG ((DEBUG_ERROR, "%a: Enable/disable timeout\n", __func__));
  }

  if ((Enable == 0) || (I2cStatusCnt == 0)) {
    /* Unset the target adddress */
    MmioWrite32 (Base + DW_IC_TAR, 0);
    mI2cBusList[Bus].Enabled = 0;
  }
}

/**
 Setup Slave address
 **/
VOID
I2cSetSlaveAddr (
  UINT32  Bus,
  UINT32  SlaveAddr,
  DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINTN   Base;
  UINT32  OldEnableStatus;

  Base            = mI2cBusList[Bus].Base;
  OldEnableStatus = mI2cBusList[Bus].Enabled;
  DEBUG ((DEBUG_INFO, "%a: SlaveAddr = 0x%x\n", __FUNCTION__, SlaveAddr));
  I2cEnable (Bus, 0, mI2cBusList);
  MmioWrite32 (Base + DW_IC_TAR, SlaveAddr);
  // if (OldEnableStatus != 0) {
    I2cEnable (Bus, 1, mI2cBusList);
  // }
}

VOID
I2cSclInit (
  UINT32  Bus,
  UINT32  I2cClkFreq,
  UINT32  I2cSpeed,
  DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINT16  IcCon;
  UINTN   Base;
  UINT32  I2cSpeedKhz;
  UINTN   ena;

  Base        = mI2cBusList[Bus].Base;
  I2cSpeedKhz = I2cSpeed / 1000;

  DEBUG ((
    DEBUG_VERBOSE,
    "%a: Bus %d I2cClkFreq %d I2cSpeed %d\n",
    __func__,
    Bus,
    I2cClkFreq,
    I2cSpeed
    ));

	/* Get enable setting for restore later */
	ena = MmioRead32 (Base + DW_IC_ENABLE) & IC_ENABLE_0B;

  /* to set speed cltr must be disabled */
  I2cEnable (Bus, 0, mI2cBusList);

  IcCon = (MmioRead32 (Base + DW_IC_CON) & (~IC_CON_SPD_MSK));

  IcCon |= DW_IC_CON_SPEED_STD;
  // Standard speed mode
  MmioWrite32 (Base + DW_IC_SS_SCL_HCNT, I2cSclParam[I2cSpeedModeStandard][I2cSclHcnt]);
  MmioWrite32 (Base + DW_IC_SS_SCL_LCNT, I2cSclParam[I2cSpeedModeStandard][I2cSclLcnt]);
  DEBUG ((DEBUG_INFO, "%a: Standard speed mode \n", __FUNCTION__));

  MmioWrite32 (Base + DW_IC_CON, IcCon);

  /* Configure SDA Hold Time if required */
  MmioWrite32 (Base + DW_IC_SDA_HOLD, I2cSclParam[I2cSpeedModeStandard][I2cSclSpkLen]);

	/* Restore back i2c now speed set */
	if (ena == IC_ENABLE_0B) {
		I2cEnable (Bus, 1, mI2cBusList);
	}
}

EFI_STATUS
I2cInit (
  UINT32  Bus,
  UINTN   BusSpeed,
  DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINTN  Base;
  DEBUG ((DEBUG_ERROR, "%a:%d  \n", __FUNCTION__, __LINE__));

  mI2cBusList[Bus].BusSpeed = BusSpeed;
  I2cHWInit (Bus, mI2cBusList);
  Base = mI2cBusList[Bus].Base;

  /* Disable the adapter and interrupt */
  I2cEnable (Bus, 0, mI2cBusList);
  MmioWrite32 (Base + DW_IC_CON, IC_CON_SD | IC_CON_RE | IC_CON_SPD_FS | IC_CON_MM);
  MmioWrite32 (Base + DW_IC_RX_TL, IC_RX_TL);
  MmioWrite32 (Base + DW_IC_TX_TL, IC_TX_TL);
  MmioWrite32 (Base + DW_IC_INTR_MASK, IC_STOP_DET);

  /* Set standard and fast speed divider for high/low periods */
  I2cSclInit (Bus, mI2cClock, BusSpeed, mI2cBusList);
  MmioWrite32 (Base + DW_IC_SAR, 0x0);

  /* Enable i2c */
  I2cEnable (Bus, 1, mI2cBusList);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
I2cProbe (
  IN UINT32   Bus,
  IN UINTN    BusSpeed,
  IN BOOLEAN  IsSmbus,
  IN BOOLEAN  PecCheck,
  IN DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  DEBUG ((DEBUG_ERROR, "%a:%d entry! \n", __FUNCTION__, __LINE__));

  if (  (Bus >= I2C_MAX_BUS_NUM)
     || (BusSpeed > DW_I2C_MAXIMUM_SPEED_HZ))
  {
    return EFI_INVALID_PARAMETER;
  }

  mI2cBusList[Bus].IsSmbus  = IsSmbus;
  mI2cBusList[Bus].PecCheck = PecCheck;

  return I2cInit (Bus, BusSpeed, mI2cBusList);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
EFI_STATUS
EFIAPI
I2c_Wait_For_BB (
  IN     UINT32  Bus,
  IN DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  UINTN   Base;
  UINT64  start_time_rx;

  DEBUG((DEBUG_INFO, "%a: Entry \n", __FUNCTION__));
  Base          = mI2cBusList[Bus].Base;
  start_time_rx = GetPerformanceCounter ();

	while ((MmioRead32 (Base + DW_IC_STATUS) & IC_STATUS_MA) ||
         !(MmioRead32 (Base + DW_IC_STATUS) & IC_STATUS_TFE)) {

		/* Evaluate timeout */
		if (GetTimeInMilliSecond(GetPerformanceCounter () - start_time_rx) > (UINT32)I2C_BYTE_TO_BB) {
      DEBUG((DEBUG_INFO, "Timed out waiting for bus\n"));
			return EFI_TIMEOUT;
    }
	}
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
I2cRead (
  IN     UINT32  Bus,
  IN     UINT32  SlaveAddr,
  IN     UINT8   *BufCmd,
  IN     UINT32  CmdLength,
  IN OUT UINT8   *Buf,
  IN OUT UINT32  *ReadLength,
  IN DW_I2C_CONTEXT_T mI2cBusList[]
  )
{
  EFI_STATUS Status;
  UINT64  start_time_rx = 0;
  BOOLEAN active        = FALSE;
  UINTN   Base          = mI2cBusList[Bus].Base;
  UINTN   len           = *ReadLength;

  if (  (Bus >= I2C_MAX_BUS_NUM)
     || (Buf == NULL)
     || (ReadLength == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Status = I2c_Wait_For_BB (Bus, mI2cBusList);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  I2cSetSlaveAddr (Bus, SlaveAddr, mI2cBusList);

  start_time_rx = GetPerformanceCounter ();
	while (len) {
		if (!active) {
			/*
			 * Avoid writing to ic_cmd_data multiple times
			 * in case this loop spins too quickly and the
			 * ic_status RFNE bit isn't set after the first
			 * write. Subsequent writes to ic_cmd_data can
			 * trigger spurious i2c transfer.
			 */
			if (len == 1)
        MmioWrite32 (Base + DW_IC_DATA_CMD, DW_IC_DATA_CMD_CMD | DW_IC_DATA_CMD_STOP);
			else
        MmioWrite32 (Base + DW_IC_DATA_CMD, DW_IC_DATA_CMD_CMD);
			active = TRUE;
		}
  
		if (MmioRead32 (Base + DW_IC_STATUS) & IC_STATUS_RFNE) {
      *Buf++ = MmioRead32 (Base + DW_IC_DATA_CMD) & DW_IC_DATA_CMD_DAT_MASK;
			len--;
			start_time_rx = GetPerformanceCounter ();
			active = FALSE;
		} else if (GetTimeInMilliSecond (GetPerformanceCounter () - start_time_rx) > I2C_BYTE_TO) {
      DEBUG((DEBUG_INFO, "Timed out\n"));
			return EFI_TIMEOUT;
		}
	}
  DEBUG((DEBUG_INFO, "finish\n"));
  start_time_rx = GetPerformanceCounter ();
	while (1) {
		if ((MmioRead32 (Base + DW_IC_RAW_INTR_STAT) & IC_STOP_DET)) {
			MmioRead32 (Base + DW_IC_CLR_STOP_DET);
			break;
		} else if (GetTimeInMilliSecond(GetPerformanceCounter () - start_time_rx) > I2C_STOPDET_TO) {
      DEBUG((DEBUG_INFO, "Timed out STOP\n"));
			break;
		}
	}

  Status = I2c_Wait_For_BB (Bus, mI2cBusList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG((DEBUG_INFO, "i2c_flush_rxfifo\n"));
	while (MmioRead32 (Base + DW_IC_STATUS) & IC_STATUS_RFNE)
		MmioRead32 (Base + DW_IC_DATA_CMD);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
I2cLibConstructor (
  VOID
  )
{
  // VOID               *Hob;
  // PLATFORM_INFO_HOB  *PlatformHob;

  // /* Get I2C Clock from the Platform HOB */
  // Hob = GetFirstGuidHob (&gPlatformInfoHobGuid);
  // if (Hob == NULL) {
  //   return EFI_NOT_FOUND;
  // }

  // PlatformHob = (PLATFORM_INFO_HOB *)GET_GUID_HOB_DATA (Hob);
  // mI2cClock   = PlatformHob->AhbClk;

  // mI2cClock = 49500000U;
  // ASSERT (mI2cClock != 0);

  return EFI_SUCCESS;
}
