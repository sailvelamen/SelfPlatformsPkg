/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright 2022 StarFive, Inc <xingyu.wu@starfivetech.com>
 */

#ifndef __DT_BINDINGS_CLOCK_STARFIVE_JH7110_VOUT_H__
#define __DT_BINDINGS_CLOCK_STARFIVE_JH7110_VOUT_H__

/* regisger */
#define JH7110_APB						0
#define JH7110_DC8200_PIX0					1
#define JH7110_DSI_SYS						2
#define JH7110_TX_ESC						3
#define JH7110_U0_DC8200_CLK_AXI				4
#define JH7110_U0_DC8200_CLK_CORE				5
#define JH7110_U0_DC8200_CLK_AHB				6
#define JH7110_U0_DC8200_CLK_PIX0				7
#define JH7110_U0_DC8200_CLK_PIX1				8
#define JH7110_DOM_VOUT_TOP_LCD_CLK				9
#define JH7110_U0_CDNS_DSITX_CLK_APB				10
#define JH7110_U0_CDNS_DSITX_CLK_SYS				11
#define JH7110_U0_CDNS_DSITX_CLK_DPI				12
#define JH7110_U0_CDNS_DSITX_CLK_TXESC				13
#define JH7110_U0_MIPITX_DPHY_CLK_TXESC				14
#define JH7110_U0_HDMI_TX_CLK_MCLK				15
#define JH7110_U0_HDMI_TX_CLK_BCLK				16
#define JH7110_U0_HDMI_TX_CLK_SYS				17

#define JH7110_CLK_VOUT_REG_END					18

/* other */
#define JH7110_DISP_ROOT					18
#define JH7110_DISP_AXI						19
#define JH7110_DISP_AHB						20
#define JH7110_HDMI_PHY_REF					21
#define JH7110_HDMITX0_MCLK					22
#define JH7110_HDMITX0_SCK					23

#define JH7110_MIPI_DPHY_REF					24
#define JH7110_U0_PCLK_MUX_BIST_PCLK				25
#define JH7110_DISP_APB						26
#define JH7110_U0_PCLK_MUX_FUNC_PCLK				27
#define JH7110_U0_DOM_VOUT_CRG_PCLK				28
#define JH7110_U0_DOM_VOUT_SYSCON_PCLK				29
#define JH7110_U0_SAIF_AMBA_DOM_VOUT_AHB_DEC_CLK_AHB		30
#define JH7110_U0_AHB2APB_CLK_AHB				31
#define JH7110_U0_P2P_ASYNC_CLK_APBS				32
#define JH7110_U0_CDNS_DSITX_CLK_RXESC				33
#define JH7110_U0_CDNS_DSITX_CLK_TXBYTEHS			34
#define JH7110_U0_MIPITX_DPHY_CLK_SYS				35
#define JH7110_U0_MIPITX_DPHY_CLK_DPHY_REF			36
#define JH7110_U0_MIPITX_APBIF_PCLK				37
#define JH7110_HDMI_TX_CLK_REF					38
#define JH7110_U0_DC8200_CLK_PIX0_OUT				39
#define JH7110_U0_DC8200_CLK_PIX1_OUT				40

#define JH7110_CLK_VOUT_END					41

/* external clocks */
#define JH7110_HDMITX0_PIXELCLK			(JH7110_CLK_VOUT_END + 0)
#define JH7110_MIPITX_DPHY_RXESC		(JH7110_CLK_VOUT_END + 1)
#define JH7110_MIPITX_DPHY_TXBYTEHS		(JH7110_CLK_VOUT_END + 2)



#endif /* __DT_BINDINGS_CLOCK_STARFIVE_JH7110_H__ */
