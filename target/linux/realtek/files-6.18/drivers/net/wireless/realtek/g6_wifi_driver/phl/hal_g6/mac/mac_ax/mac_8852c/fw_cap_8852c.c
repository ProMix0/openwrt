/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/
#include "fw_cap_8852c.h"
#include "../../fw_ax/rtl8852c/hal8852c_fw_cap.h"

#if MAC_AX_8852C_SUPPORT

const char *fw_cap_8852c[] = {"BEACON", "MBSSID", "MLO", "MLO_DM", "MC_MLO", "EMLSR",
			     "TWT_AP", "SMPS", "F2PCMD", "MU_MIMO",  "SOUND_MER", "UL_OFDMA",
			     "DL_OFDMA"};

u32 get_fw_cap_8852c(struct mac_ax_adapter *adapter, struct mac_fw_cap_info *info)
{
#ifdef PHL_FEATURE_AP
	info->is_support[TWT_AP_SEL] = FW_CONFIG_TWT_AP;
	info->is_support[F2PCMD_SEL] = FW_CONFIG_F2PCMD;
	info->is_support[MU_MIMO_SEL] = FW_CONFIG_MU;
	info->is_support[SOUND_MER_SEL] = FW_CONFIG_SND;

	info->version[TWT_AP_SEL] = TWT_AP_VER;
	info->version[F2PCMD_SEL] = F2PCMD_VER;
	info->version[MU_MIMO_SEL] = MU_VER;
	info->version[SOUND_MER_SEL] = SND_VER;

	info->sub_version[TWT_AP_SEL] = TWT_AP_SUBVER;
	info->sub_version[F2PCMD_SEL] = F2PCMD_SUBVER;
	info->sub_version[MU_MIMO_SEL] = MU_SUBVER;
	info->sub_version[SOUND_MER_SEL] = SND_SUBVER;
#endif
	return MACSUCCESS;
}

u32 dump_fw_cap_8852c(struct mac_ax_adapter *adapter,  struct mac_fw_cap_info *info,
		      enum mac_fw_feat_sel sel)
{
	u8 idx;

	if (sel == ALL_FEAT_SEL) {
		for (idx = BEACON_SEL; idx < ALL_FEAT_SEL; idx++) {
			PLTFM_MSG_ALWAYS("Feature %s = %x, version = %x\n", fw_cap_8852c[idx],
					 info->is_support[idx], info->version[idx]);
		}
	} else if (sel == LAST_FEAT_SEL) {
		PLTFM_MSG_ALWAYS("Feature selection Error = %x\n", sel);
	} else {
		PLTFM_MSG_ALWAYS("Feature %s = %x, version = %x\n", fw_cap_8852c[sel],
				 info->is_support[sel], info->version[sel]);
	}

	return MACSUCCESS;
}
#endif /* MAC_AX_8852C_SUPPORT */
