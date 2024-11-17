/** @file
 *
 * PCI Host Bridge Library instance for StarFive JH7110 SOC
 *
 * Copyright (c) 2023, Minda Chen <minda.chen@starfivetech.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 * This module initializes the Pci as close to a standard
 * PCI root complex as possible. The general information
 * for this driver was sourced from.
 *
 *
 **/
#include <Base.h>
#include <IndustryStandard/JH7110.h>
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <PiDxe.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Library/TimerLib.h>

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((UINT32)(((n) >> 16) >> 16))
/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((UINT32)((n) & 0xffffffff))

#define RegWrite(addr,data)            MmioWrite32((addr), (data))
#define RegRead(addr,data)             ((data) = MmioRead32 (addr))

#define SYS_SYSCON_BASE 0x13030000

#define STG_SYSCON_K_RP_NEP_MASK               (1 << 8)
#define STG_SYSCON_CKREF_SRC_SHIFT             18
#define STG_SYSCON_CKREF_SRC_MASK              (0x3 << 18)
#define STG_SYSCON_CLKREQ_MASK			(1 << 22)
#define STG_SYSCON_BASE 0x10240000
#define SYS_CLK_BASE 0x13020000
#define STG_CLK_BASE 0x10230000
#define SYS_CLK_NOC_OFFSET 0x98
#define STG_PCIE_CLK_OFFSET 0x20
#define STG_PCIE_CLKS 0xc
#define STG_PCIE_RESET_OFFSET 0x74
#define SYS_GPIO_BASE 0x13040000

#define PREF_MEM_WIN_64_SUPPORT         (1 << 3)
#define PMSG_LTR_SUPPORT                (1 << 2)
#define PDLA_LINK_SPEED_GEN2            (1 << 12)
#define PLDA_FUNCTION_DIS               (1 << 15)
#define PLDA_FUNC_NUM                   4
#define PLDA_PHY_FUNC_SHIFT             9
#define PLDA_RP_ENABLE                  1

#define PCIE_BASIC_STATUS               0x018
#define PCIE_CFGNUM                     0x140
#define IMASK_LOCAL                     0x180
#define ISTATUS_LOCAL                   0x184
#define IMSI_ADDR                       0x190
#define ISTATUS_MSI                     0x194
#define CFG_SPACE                       0x1000
#define GEN_SETTINGS                    0x80
#define PCIE_PCI_IDS                    0x9C
#define PCIE_WINROM                     0xFC
#define PMSG_SUPPORT_RX                 0x3F0
#define PCI_MISC                        0xB4

#define STG_SYSCON_AXI4_SLVL_ARFUNC_MASK        0x7FFF00
#define STG_SYSCON_AXI4_SLVL_ARFUNC_SHIFT       0x8
#define STG_SYSCON_AXI4_SLVL_AWFUNC_MASK        0x7FFF
#define STG_SYSCON_AXI4_SLVL_AWFUNC_SHIFT       0x0


#define XR3PCI_ATR_AXI4_SLV0            0x800
#define XR3PCI_ATR_SRC_ADDR_LOW         0x0
#define XR3PCI_ATR_SRC_ADDR_HIGH        0x4
#define XR3PCI_ATR_TRSL_ADDR_LOW        0x8
#define XR3PCI_ATR_TRSL_ADDR_HIGH       0xc
#define XR3PCI_ATR_TRSL_PARAM           0x10
#define XR3PCI_ATR_TABLE_OFFSET         0x20
#define XR3PCI_ATR_MAX_TABLE_NUM        8

#define XR3PCI_ATR_SRC_ADDR_MASK        0xfffff000
#define XR3PCI_ATR_TRSL_ADDR_MASK       0xfffff000
#define XR3PCI_ATR_SRC_WIN_SIZE_SHIFT   1
#define XR3_PCI_ECAM_SIZE		28

#define IDS_PCI_TO_PCI_BRIDGE           0x060400
#define IDS_CLASS_CODE_SHIFT            8
#define SYS_GPIO_OUTPUT_OFF		0x40

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0			0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1			0x14	/* 32 bits [htype 0,1 only] */
#define PHY_KVCO_FINE_TUNE_LEVEL	0x91
#define PHY_KVCO_FINE_TUNE_SIGNALS	0xc
#define PCIE_KVCO_LEVEL_OFF			0x28
#define PCIE_KVCO_TUNE_SIGNAL_OFF	0x80
#define XR3PCI_ATR_TRSL_DIR			(1 << 22)
#define CONFIG_SPACE_ADDR_OFFSET    0x1000
/* PCIe Master table init defines */
#define ATR0_PCIE_WIN0_SRCADDR_PARAM  0x600
#define ATR0_PCIE_ATR_SIZE			  0x25
#define ATR0_PCIE_ATR_SIZE_SHIFT	  1
#define ATR0_PCIE_WIN0_SRC_ADDR		  0x604

UINT32 AtrTableNum;
UINT64 PCIE_CFG_BASE[2] = {0x940000000, 0x9c0000000};
UINT64 PCI_MEMREGION_32[2] = {0x30000000, 0x38000000};
UINT64 PCI_MEMREGION_64[2] = {0x900000000, 0x980000000};
UINT64 PCI_MEMREGION_SIZE[2] = {27, 30};
UINT32 STG_ARFUNC_OFFSET[2] = {0xc0, 0x270};
UINT32 STG_AWFUNC_OFFSET[2] = {0xc4, 0x274};
UINT32 STG_RP_REP_OFFSET[2] = {0x130, 0x2e0};
UINT32 PCIE_GPIO[2] = {26, 28};
UINT64 PCIE_PHY_BASE[2] = {0x10210000, 0x10220000};

VOID PciePhyWrite(UINT32 Port, UINTN Offset, UINT32 Value)
{
    UINT64 base = PCIE_PHY_BASE[Port];

    RegWrite((UINT64)base + Offset, Value);
}

static inline UINT64 get_pcie_reg_base(UINT32 Port)
{
    return PCIE_REG_BASE + Port * 0x1000000;
}

VOID PcieRegWrite(UINT32 Port, UINTN Offset, UINT32 Value)
{
    UINT64 base = get_pcie_reg_base(Port);

    RegWrite((UINT64)base + Offset, Value);
}

UINT32 PcieRegRead(UINT32 Port, UINTN Offset)
{
    UINT32 Value = 0;
    UINT64 base = get_pcie_reg_base(Port);

    RegRead((UINT64)base + Offset, Value);
    return Value;
}

static VOID PcieUpdatebits(UINT64 base, UINTN Offset, UINT32 mask, UINT32 val)
{
    UINT32 Value = 0;

    Value = MmioRead32((UINT64)base + Offset);
    Value &= ~mask;
    Value |= val;
    MmioWrite32((UINT64)base + Offset, Value);
}

STATIC
VOID PcieFuncSet(
    IN UINT32 Port)
{
	int i;
	UINT32 Value;
	UINT64 base = get_pcie_reg_base(Port);

	/* Disable physical functions except #0 */
	for (i = 1; i < PLDA_FUNC_NUM; i++) {
		PcieUpdatebits(STG_SYSCON_BASE, STG_ARFUNC_OFFSET[Port],
			STG_SYSCON_AXI4_SLVL_ARFUNC_MASK,
			(i << PLDA_PHY_FUNC_SHIFT) <<
			STG_SYSCON_AXI4_SLVL_ARFUNC_SHIFT);
		PcieUpdatebits(STG_SYSCON_BASE, STG_AWFUNC_OFFSET[Port],
			STG_SYSCON_AXI4_SLVL_AWFUNC_MASK,
			(i << PLDA_PHY_FUNC_SHIFT) <<
			STG_SYSCON_AXI4_SLVL_AWFUNC_SHIFT);
		PcieUpdatebits(base, PCI_MISC,
			PLDA_FUNCTION_DIS, PLDA_FUNCTION_DIS);
	}

	PcieUpdatebits(STG_SYSCON_BASE, STG_ARFUNC_OFFSET[Port],
			STG_SYSCON_AXI4_SLVL_ARFUNC_MASK, 0);
	PcieUpdatebits(STG_SYSCON_BASE, STG_AWFUNC_OFFSET[Port],
			STG_SYSCON_AXI4_SLVL_AWFUNC_MASK, 0);

	/* Enable root port*/
	PcieUpdatebits(base, GEN_SETTINGS,
		PLDA_RP_ENABLE, PLDA_RP_ENABLE);

	/*plda_pcie_write_rc_bar*/
	RegWrite(base + CONFIG_SPACE_ADDR_OFFSET + PCI_BASE_ADDRESS_0, 0x0);
	RegWrite(base + CONFIG_SPACE_ADDR_OFFSET + PCI_BASE_ADDRESS_1, 0x0);

	/* PCIe PCI Standard Configuration Identification Settings. */
	/* set class code and reserve revision id */
    Value = (IDS_PCI_TO_PCI_BRIDGE << IDS_CLASS_CODE_SHIFT);
	PcieRegWrite(Port, PCIE_PCI_IDS, Value);

	/*
	 * The LTR message forwarding of PCIe Message Reception was set by core
	 * as default, but the forward id & addr are also need to be reset.
	 * If we do not disable LTR message forwarding here, or set a legal
	 * forwarding address, the kernel will get stuck after the driver probe.
	 * To workaround, disable the LTR message forwarding support on
	 * PCIe Message Reception.
	 */
	PcieUpdatebits(base, PMSG_SUPPORT_RX,
		PMSG_LTR_SUPPORT, 0);

	/* Prefetchable memory window 64-bit addressing support */
	PcieUpdatebits(base, PCIE_WINROM,
		PREF_MEM_WIN_64_SUPPORT, PREF_MEM_WIN_64_SUPPORT);
}

STATIC
VOID
PcieSTGInit(
    IN UINT32 Port)
{

    PcieUpdatebits(STG_SYSCON_BASE, STG_RP_REP_OFFSET[Port],
		STG_SYSCON_K_RP_NEP_MASK, 
		STG_SYSCON_K_RP_NEP_MASK);
    PcieUpdatebits(STG_SYSCON_BASE, STG_AWFUNC_OFFSET[Port],
		STG_SYSCON_CKREF_SRC_MASK, 
		2 << STG_SYSCON_CKREF_SRC_SHIFT);
    PcieUpdatebits(STG_SYSCON_BASE, STG_AWFUNC_OFFSET[Port],
		STG_SYSCON_CLKREQ_MASK, 
		STG_SYSCON_CLKREQ_MASK);
}

STATIC
VOID
PcieClockInit(
    IN UINT32 Port)
{
	RegWrite(STG_CLK_BASE + STG_PCIE_CLK_OFFSET
		+ Port * STG_PCIE_CLKS, 1 << 31); /*axi mst0*/
	RegWrite(STG_CLK_BASE + STG_PCIE_CLK_OFFSET
		+ Port * STG_PCIE_CLKS + 4, 1 << 31); /* apb */
	RegWrite(STG_CLK_BASE + STG_PCIE_CLK_OFFSET
		+ Port * STG_PCIE_CLKS + 8, 1 << 31); /* tl0 */
}


STATIC
VOID
PcieResetDeassert(
    IN UINT32 Port)
{
	UINT32 Portoffset = Port * 6 + 11;

	PcieUpdatebits(STG_CLK_BASE, STG_PCIE_RESET_OFFSET,
		0x3f << (Portoffset), 0); /*reset all*/
}

VOID
PcieResetAssert(
    IN UINT32 Port)
{
	UINT32 Portoffset = Port * 6 + 11;

	PcieUpdatebits(STG_CLK_BASE, STG_PCIE_RESET_OFFSET,
		0x3f << (Portoffset), 0x3f << (Portoffset)); /*axi mst0*/
}

STATIC
VOID PcieGpioResetSet(
    IN UINT32 Port,
    IN UINT32 Value)
{
	UINT32 remain, mask;

	remain = PCIE_GPIO[Port] & 0x3;
	mask = 0xff << (remain * 8);
	PcieUpdatebits(SYS_GPIO_BASE, SYS_GPIO_OUTPUT_OFF + (PCIE_GPIO[Port] & 0xfffc),
		mask, Value << (remain * 8));
}

// STATIC
// VOID PcieAtrInit(
//     IN UINT32 Port,
//     IN UINT64 src_addr,
//     IN UINT64 trsl_addr,
//     IN UINT32 win_size,
//     IN UINT32 config)
// {
// 	UINT64 base = get_pcie_reg_base(Port) + XR3PCI_ATR_AXI4_SLV0;
// 	UINT32 Value;

// 	DEBUG ((DEBUG_INFO, "AtrTableNum = 0x%x \n", AtrTableNum));
// 	/* Support AXI4 Slave 0 Address Translation Tables 0-7. */
// 	if (AtrTableNum >= XR3PCI_ATR_MAX_TABLE_NUM)
// 		AtrTableNum = XR3PCI_ATR_MAX_TABLE_NUM - 1;
//     base +=  XR3PCI_ATR_TABLE_OFFSET * AtrTableNum;
// 	AtrTableNum++;

//         /* X3PCI_ATR_SRC_ADDR_LOW:
//          *   - bit 0: enable entry,
//          *   - bits 1-6: ATR window size: total size in bytes: 2^(ATR_WSIZE + 1)
//          *   - bits 7-11: reserved
//          *   - bits 12-31: start of source address
//          */
// 	Value = src_addr;
// 	//DEBUG ((DEBUG_ERROR, "addr low %x\n", Value));
// 	RegWrite(base + XR3PCI_ATR_SRC_ADDR_LOW,
// 		(Value & XR3PCI_ATR_SRC_ADDR_MASK) | ((win_size - 1) << 1) | 0x1);
// 	Value = src_addr >> 32;
// 	//DEBUG ((DEBUG_ERROR, "addr high %x\n", Value));
// 	RegWrite(base + XR3PCI_ATR_SRC_ADDR_HIGH, Value);
// 	Value = trsl_addr;
// 	RegWrite(base + XR3PCI_ATR_TRSL_ADDR_LOW, Value);
// 	Value = trsl_addr >> 32;
// 	RegWrite(base + XR3PCI_ATR_TRSL_ADDR_HIGH, Value);
// 	RegWrite(base + XR3PCI_ATR_TRSL_PARAM, config);

// 	RegRead(get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value);
// 	DEBUG ((DEBUG_INFO, "fanjix: 0x%x = 0x%x \n", get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value));
// 	Value |= (ATR0_PCIE_ATR_SIZE << ATR0_PCIE_ATR_SIZE_SHIFT);
// 	RegWrite(get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value);
// 	//
// 	RegRead(get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value);
// 	DEBUG ((DEBUG_INFO, "fanjix: 0x%x = 0x%x \n", get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value));
// 	//
// 	RegWrite(get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRC_ADDR, 0x0);
// 	Value = 0;
// 	RegRead(get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRC_ADDR, Value);
// 	DEBUG ((DEBUG_INFO, "fanjix: 0x%x = 0x%x \n", get_pcie_reg_base(Port) + ATR0_PCIE_WIN0_SRC_ADDR, Value));

// 	DEBUG ((DEBUG_INFO, "ATR entry: 0x%010llx %s 0x%010llx [0x%010llx] (param: 0x%06x)\n",
// 		src_addr, (config & XR3PCI_ATR_TRSL_DIR) ? L"<-" : L"->", trsl_addr, (UINT64)win_size, config
// 	));
// }

/* As the two host bridges in JH7110 soc have the same default
* address translation table, this cause the second root port can't
* access it's host bridge config space correctly.
* To workaround, config the ATR of host bridge config space by SW.
*/
STATIC
VOID Plda_Pcie_Setup_Window(
    IN UINT32 Port,
    IN UINT64 axi_addr,
    IN UINT64 pci_addr,
    IN UINT32 win_size,
    IN UINT32 config)
{
	UINT64 base = get_pcie_reg_base(Port);
	UINT32 Value, Index;

	Index = AtrTableNum * 32;
	AtrTableNum++;

	RegWrite(base + Index + XR3PCI_ATR_AXI4_SLV0 + XR3PCI_ATR_TRSL_PARAM, config);

	Value = lower_32_bits(axi_addr) | ((win_size - 1) << 1) | 0x1;
	RegWrite(base + Index + XR3PCI_ATR_AXI4_SLV0, Value);

	Value = upper_32_bits(axi_addr);
	RegWrite(base + Index + XR3PCI_ATR_AXI4_SLV0 + XR3PCI_ATR_SRC_ADDR_HIGH, Value);

	Value = lower_32_bits(pci_addr);
	RegWrite(base + Index + XR3PCI_ATR_AXI4_SLV0 + XR3PCI_ATR_TRSL_ADDR_LOW, Value);

	Value = upper_32_bits(pci_addr);
	RegWrite(base + Index + XR3PCI_ATR_TRSL_ADDR_HIGH, Value);

	Value |= (ATR0_PCIE_ATR_SIZE << ATR0_PCIE_ATR_SIZE_SHIFT);
	RegWrite(base + ATR0_PCIE_WIN0_SRCADDR_PARAM, Value);
	RegWrite(base + ATR0_PCIE_WIN0_SRC_ADDR, 0x0);

	DEBUG ((DEBUG_INFO, "ATR entry: axi_addr:0x%010llx %s pci_addr:0x%010llx win_size:[0x%010llx] (param: 0x%06x)\n",
		axi_addr, (config & XR3PCI_ATR_TRSL_DIR) ? L"<-" : L"->", pci_addr, (UINT64)(1 << win_size), config
	));
}

EFI_STATUS
EFIAPI
JH7110PciHostBridgeLibConstructor (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  for (UINT32 PortIndex = 0; PortIndex < 2; PortIndex++)
  {
	DEBUG ((DEBUG_ERROR, "PCIe RootBridge constructor\n"));
		
	DEBUG ((DEBUG_INFO, "%a: phy_kvco_gain_set!\n", __FUNCTION__));
	PciePhyWrite(PortIndex, PCIE_KVCO_LEVEL_OFF, PHY_KVCO_FINE_TUNE_LEVEL);
	PciePhyWrite(PortIndex, PCIE_KVCO_TUNE_SIGNAL_OFF, PHY_KVCO_FINE_TUNE_SIGNALS);

	PcieSTGInit(PortIndex);
	RegWrite(SYS_CLK_BASE + SYS_CLK_NOC_OFFSET, 1 << 31);
	PcieClockInit(PortIndex);
	PcieResetDeassert(PortIndex);
	PcieGpioResetSet(PortIndex, 0);
	PcieFuncSet(PortIndex);

	AtrTableNum = 0;

	// PcieAtrInit(PortIndex, PCIE_CFG_BASE[PortIndex],
	// 	0, XR3_PCI_ECAM_SIZE, 1);
	// PcieAtrInit(PortIndex, PCI_MEMREGION_32[PortIndex],
	// 	PCI_MEMREGION_32[PortIndex], PCI_MEMREGION_SIZE[0], 0);
	// PcieAtrInit(PortIndex, PCI_MEMREGION_64[PortIndex],
	// 	PCI_MEMREGION_64[PortIndex], PCI_MEMREGION_SIZE[1], 0);
	Plda_Pcie_Setup_Window(PortIndex, PCIE_CFG_BASE[PortIndex], 0, 0x18, 1);
	Plda_Pcie_Setup_Window(PortIndex, PCI_MEMREGION_32[PortIndex], PCI_MEMREGION_32[PortIndex], 0x1B, 0);
	Plda_Pcie_Setup_Window(PortIndex, PCI_MEMREGION_64[PortIndex], PCI_MEMREGION_64[PortIndex], 0x1E, 0);


	PcieGpioResetSet(PortIndex, 1);
	MicroSecondDelay(300);

	DEBUG ((DEBUG_ERROR, "PCIe port %d init\n", PortIndex));
  }
  return EFI_SUCCESS;
}
