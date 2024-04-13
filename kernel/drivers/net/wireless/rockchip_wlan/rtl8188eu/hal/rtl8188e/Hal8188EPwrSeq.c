/* SPDX-License-Identifier: GPL-2.0 */
/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#include "Hal8188EPwrSeq.h"
#include <rtl8188e_hal.h>

/*
    drivers should parse below arrays and do the corresponding actions
*/
/* 3 Power on  Array */
WLAN_PWR_CFG rtl8188E_power_on_flow[RTL8188E_TRANS_CARDEMU_TO_ACT_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};

/* 3Radio off Array */
WLAN_PWR_CFG rtl8188E_radio_off_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_END
};

/* 3Card Disable Array */
WLAN_PWR_CFG rtl8188E_card_disable_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_CARDDIS
	RTL8188E_TRANS_END
};

/* 3 Card Enable Array */
WLAN_PWR_CFG rtl8188E_card_enable_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_CARDDIS_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};

/* 3Suspend Array */
WLAN_PWR_CFG rtl8188E_suspend_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_CARDEMU_TO_SUS_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_SUS
	RTL8188E_TRANS_END
};

/* 3 Resume Array */
WLAN_PWR_CFG rtl8188E_resume_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_CARDEMU_TO_SUS_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_SUS_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_ACT
	RTL8188E_TRANS_END
};


/* 3HWPDN Array */
WLAN_PWR_CFG rtl8188E_hwpdn_flow[RTL8188E_TRANS_ACT_TO_CARDEMU_STEPS + RTL8188E_TRANS_CARDEMU_TO_PDN_STEPS + RTL8188E_TRANS_END_STEPS] = {
	RTL8188E_TRANS_ACT_TO_CARDEMU
	RTL8188E_TRANS_CARDEMU_TO_PDN
	RTL8188E_TRANS_END
};

/* 3 Enter LPS */
WLAN_PWR_CFG rtl8188E_enter_lps_flow[RTL8188E_TRANS_ACT_TO_LPS_STEPS + RTL8188E_TRANS_END_STEPS] = {
	/* FW behavior */
	RTL8188E_TRANS_ACT_TO_LPS
	RTL8188E_TRANS_END
};

/* 3 Leave LPS */
WLAN_PWR_CFG rtl8188E_leave_lps_flow[RTL8188E_TRANS_LPS_TO_ACT_STEPS + RTL8188E_TRANS_END_STEPS] = {
	/* FW behavior */
	RTL8188E_TRANS_LPS_TO_ACT
	RTL8188E_TRANS_END
};
