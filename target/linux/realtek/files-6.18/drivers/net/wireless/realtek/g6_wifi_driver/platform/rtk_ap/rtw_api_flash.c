/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _HAL_API_FLASH_C_
#include <drv_types.h>
#include <../phl/phl_headers.h>
#include <../phl/hal_g6/hal_general_def.h>
#include <../phl_api_drv.h>
#include "rtw_api_flash.h"
#include "flash/rtw_flash.h"
#include "flash/rtw_flash_export.h"

int _atoi(char *s, int base)
{
	int k = 0;
	int sign = 1;
	if (NULL == s){
		return 0;
	}

	k = 0;
	if (base == 10) {
		if(*s== '-') {
			sign = -1;
			s++;
		}
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
		k *= sign;
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				//RTW_ERR("error hex format! %c\n", s);
				return k;
			}

			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

u32 rtw_nvm_get_info(void *d, u32 info_type, void *value, u8 size)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	return rtw_flash_get_info(dvobj->nvm, (enum rtw_efuse_info )info_type, value, size);
}

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
void rtw_load_phy_file_path (struct dvobj_priv *dvobj);
#endif

u32 rtw_nvm_set_info(void *dev, u8 *cmd, u8 *data)
{
	u32 status = _SUCCESS;
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	enum rtw_efuse_info query_id;
	static u32 content_size;

	u32 query_offset;
	u8 query_size;
	u8 i, value, len = 1;
	u8 temp[3];

	query_id = rtw_hal_flash_lookup_id(dvobj, cmd, &query_offset,
	                                   &query_size);

	RTW_INFO(FUNC_DEV_FMT" %s (%u) = %s\n",
	         FUNC_DEV_ARG(dvobj), cmd, query_id, data);

	if ((enum rtw_flash_info)query_id != FLASH_INFO_ID_ERROR) {
		switch (query_id) {
		case EFUSE_INFO_COSTUM_PARA_PATH:
			rtw_flash_set_para_path(dvobj->nvm, data);
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			rtw_load_phy_file_path(dvobj);
#endif
			break;

		case EFUSE_INFO_RF_2G_CCK_A_TSSI_DE_1 ... EFUSE_INFO_RF_2G_CCK_A_TSSI_DE_6:
		case EFUSE_INFO_RF_2G_CCK_B_TSSI_DE_1 ... EFUSE_INFO_RF_2G_CCK_B_TSSI_DE_6:
		case EFUSE_INFO_RF_2G_CCK_C_TSSI_DE_1 ... EFUSE_INFO_RF_2G_CCK_C_TSSI_DE_6:
		case EFUSE_INFO_RF_2G_CCK_D_TSSI_DE_1 ... EFUSE_INFO_RF_2G_CCK_D_TSSI_DE_6:
			len = 6;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				for (i = 0; i < len; i++) {
					_rtw_memset(temp, 0, sizeof(temp));
					strncpy(temp, data+i*2, 2);
					value = _atoi(temp, 16);
					rtw_flash_set_info(dvobj->nvm, query_id, &value,
									query_size, query_offset+i);
				}

			break;
		case EFUSE_INFO_RF_2G_BW40M_A_TSSI_DE_1 ... EFUSE_INFO_RF_2G_BW40M_A_TSSI_DE_5:
		case EFUSE_INFO_RF_2G_BW40M_B_TSSI_DE_1 ... EFUSE_INFO_RF_2G_BW40M_B_TSSI_DE_5:
		case EFUSE_INFO_RF_2G_BW40M_C_TSSI_DE_1 ... EFUSE_INFO_RF_2G_BW40M_C_TSSI_DE_5:
		case EFUSE_INFO_RF_2G_BW40M_D_TSSI_DE_1 ... EFUSE_INFO_RF_2G_BW40M_D_TSSI_DE_5:
			len = 5;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				for (i = 0; i < len; i++) {
					_rtw_memset(temp, 0, sizeof(temp));
					strncpy(temp, data+i*2, 2);
					value = _atoi(temp, 16);
					rtw_flash_set_info(dvobj->nvm, query_id,
									&value, query_size,
									query_offset+i);
				}

			break;
		case EFUSE_INFO_RF_5G_BW40M_A_TSSI_DE_1 ... EFUSE_INFO_RF_5G_BW40M_A_TSSI_DE_14:
		case EFUSE_INFO_RF_5G_BW40M_B_TSSI_DE_1 ... EFUSE_INFO_RF_5G_BW40M_B_TSSI_DE_14:
		case EFUSE_INFO_RF_5G_BW40M_C_TSSI_DE_1 ... EFUSE_INFO_RF_5G_BW40M_C_TSSI_DE_14:
		case EFUSE_INFO_RF_5G_BW40M_D_TSSI_DE_1 ... EFUSE_INFO_RF_5G_BW40M_D_TSSI_DE_14:
			len=14;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				for(i=0; i<len; i++){
					_rtw_memset(temp, 0, sizeof(temp));
					strncpy(temp, data+i*2, 2);
					value = _atoi(temp, 16);
					rtw_flash_set_info(dvobj->nvm, query_id, &value, query_size, query_offset+i);
				}

			break;
		/* RF 2G CCK TSSI SLOPE K */
		case EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_A_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_B_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_C_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_GAIN_DIFF_D_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_A_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_B_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_C_6:
		case EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_1 ... EFUSE_INFO_RF_2G_CCK_TSSI_CW_DIFF_D_6:
			len=6;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				rtw_flash_set_tssi_slp_info(dvobj->nvm, query_id, data, TSSI_2G_CCK_SIZE);
			break;

		/* RF 2G TSSI SLOPE K */
		case EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_A_1 ... EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_A_5:
		case EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_B_1 ... EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_B_5:
		case EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_C_1 ... EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_C_5:
		case EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_D_1 ... EFUSE_INFO_RF_2G_TSSI_GAIN_DIFF_D_5:
		case EFUSE_INFO_RF_2G_TSSI_CW_DIFF_A_1 ... EFUSE_INFO_RF_2G_TSSI_CW_DIFF_A_5:
		case EFUSE_INFO_RF_2G_TSSI_CW_DIFF_B_1 ... EFUSE_INFO_RF_2G_TSSI_CW_DIFF_B_5:
		case EFUSE_INFO_RF_2G_TSSI_CW_DIFF_C_1 ... EFUSE_INFO_RF_2G_TSSI_CW_DIFF_C_5:
		case EFUSE_INFO_RF_2G_TSSI_CW_DIFF_D_1 ... EFUSE_INFO_RF_2G_TSSI_CW_DIFF_D_5:
			len=5;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				rtw_flash_set_tssi_slp_info(dvobj->nvm, query_id, data, TSSI_2G_SIZE);
			break;

		/* RF 5G TSSI SLOPE K */
		case EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_A_1 ... EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_A_14:
		case EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_B_1 ... EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_B_14:
		case EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_C_1 ... EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_C_14:
		case EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_D_1 ... EFUSE_INFO_RF_5G_TSSI_GAIN_DIFF_D_14:
		case EFUSE_INFO_RF_5G_TSSI_CW_DIFF_A_1 ... EFUSE_INFO_RF_5G_TSSI_CW_DIFF_A_14:
		case EFUSE_INFO_RF_5G_TSSI_CW_DIFF_B_1 ... EFUSE_INFO_RF_5G_TSSI_CW_DIFF_B_14:
		case EFUSE_INFO_RF_5G_TSSI_CW_DIFF_C_1 ... EFUSE_INFO_RF_5G_TSSI_CW_DIFF_C_14:
		case EFUSE_INFO_RF_5G_TSSI_CW_DIFF_D_1 ... EFUSE_INFO_RF_5G_TSSI_CW_DIFF_D_14:
			len=14;

			if (strlen(data) != len*2)
				RTW_ERR("data length error !!\n");
			else
				rtw_flash_set_tssi_slp_info(dvobj->nvm, query_id, data, TSSI_5G_SIZE);
			break;

		default:
			if (0 == strncmp(data, "0x", 2)){
				value = _atoi(data+2, 16);
			} else {
				value = _atoi(data, 10);
			}
			rtw_flash_set_info(dvobj->nvm, query_id, &value, query_size, query_offset);

			break;
		}
	} else {
		RTW_ERR("Not found query id !! \n");
		status = _FAIL;
	}

	return status;
}

void rtw_hal_flash_process(struct dvobj_priv *dvobj)
{
	rtw_flash_process(dvobj->nvm);
}

u32 rtw_nvm_init(void *d)
{
	u32 status;
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;

	status = rtw_flash_init(dvobj, (void **)&(dvobj->nvm));

	return status;
}

#ifdef CONFIG_RTW_DEV_IS_SINGLE_BAND

static void _set_rfe_type(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u8 set_rfe_type = 0xFF;
	u8 band_cap;
	char rfe_type_str[10];

	if (phl_com == NULL) {
		return;
	}

	band_cap = phl_com->hal_spec.band_cap;

	switch (band_cap){
		case BAND_CAP_5G:
			set_rfe_type = rtw_rfe_type_5g;
			RTW_PRINT("BAND_CAP_5G, set set_rfe_type=%d\n", set_rfe_type);
			break;
		case BAND_CAP_2G:
			set_rfe_type = rtw_rfe_type_2g;
			RTW_PRINT("BAND_CAP_2G, set set_rfe_type=%d\n", set_rfe_type);
			break;
		default:
			RTW_ERR("RFE type can not be set for non single band device.\n");
			break;
	}

	snprintf(rfe_type_str, sizeof(rfe_type_str) - 1, "%u", set_rfe_type);
	rtw_nvm_set_info(dvobj, "rfe", rfe_type_str);

	phl_com->dev_cap.rfe_type = set_rfe_type;
	phl_com->dev_sw_cap.rfe_type = set_rfe_type;

#else
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	u8 set_rfe_type = 50;
	u8 band_cap;
	char rfe_type_str[10];

	if (phl_com == NULL) {
		return;
	}

	band_cap = phl_com->hal_spec.band_cap;

	switch (band_cap){
#ifndef CONFIG_2G_ON_WLAN0
		case BAND_CAP_5G:
#else
		case BAND_CAP_2G:
#endif

			#ifdef CONFIG_WLAN0_RFE_TYPE_50
				RTW_PRINT("CONFIG_WLAN0_RFE_TYPE_50\n");
				set_rfe_type = 50;
			#endif

			#ifdef CONFIG_WLAN0_RFE_TYPE_51
				RTW_PRINT("CONFIG_WLAN0_RFE_TYPE_51\n");
				set_rfe_type = 51;
			#endif

			#ifdef CONFIG_WLAN0_RFE_TYPE_52
				RTW_PRINT("CONFIG_WLAN0_RFE_TYPE_52\n");
				set_rfe_type = 52;
			#endif

			#ifdef CONFIG_WLAN0_RFE_TYPE_53
				RTW_PRINT("CONFIG_WLAN0_RFE_TYPE_53\n");
				set_rfe_type = 53;
			#endif

			#ifdef CONFIG_WLAN0_RFE_TYPE_54
				RTW_PRINT("CONFIG_WLAN0_RFE_TYPE_54\n");
				set_rfe_type = 54;
			#endif

				break;

#ifdef CONFIG_2G_ON_WLAN0
		case BAND_CAP_5G:
#else
		case BAND_CAP_2G:
#endif

			#ifdef CONFIG_WLAN1_RFE_TYPE_50
				RTW_PRINT("CONFIG_WLAN1_RFE_TYPE_50\n");
				set_rfe_type = 50;
			#endif

			#ifdef CONFIG_WLAN1_RFE_TYPE_51
				RTW_PRINT("CONFIG_WLAN1_RFE_TYPE_51\n");
				set_rfe_type = 51;
			#endif

			#ifdef CONFIG_WLAN1_RFE_TYPE_52
				RTW_PRINT("CONFIG_WLAN1_RFE_TYPE_52\n");
				set_rfe_type = 52;
			#endif

			#ifdef CONFIG_WLAN1_RFE_TYPE_53
				RTW_PRINT("CONFIG_WLAN1_RFE_TYPE_53\n");
				set_rfe_type = 53;
			#endif

			#ifdef CONFIG_WLAN1_RFE_TYPE_54
				RTW_PRINT("CONFIG_WLAN1_RFE_TYPE_54\n");
				set_rfe_type = 54;
			#endif
				break;
			default:
				RTW_ERR("RFE type can not be set for non single band device.\n");
				break;
	}

	snprintf(rfe_type_str, sizeof(rfe_type_str) - 1, "%u", set_rfe_type);
	rtw_nvm_set_info(dvobj, "rfe", rfe_type_str);

	phl_com->dev_cap.rfe_type = set_rfe_type;
	phl_com->dev_sw_cap.rfe_type = set_rfe_type;
#endif /* CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA */
}
#endif /* CONFIG_RTW_DEV_IS_SINGLE_BAND */

u32 rtw_nvm_efuse_init(void *d)
{
	u32 status;
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;

	status = rtw_flash_efuse_init(dvobj);

	#ifdef CONFIG_RTW_DEV_IS_SINGLE_BAND
	_set_rfe_type(dvobj);
	#endif /* CONFIG_RTW_DEV_IS_SINGLE_BAND */

	return status;
}

void rtw_nvm_deinit(void *d)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	//deinit_flash_id_table(hal_com, flash_id_table, FLASH_TABLE_SIZE); // deinit mapping table

	rtw_flash_deinit(dvobj, dvobj->nvm);
}

enum rtw_flash_info
rtw_hal_flash_lookup_id(struct dvobj_priv *dvobj, u8 *cmd,
                        u32 *offset, u8 *size)
{
	struct flash_t *flash = (struct flash_t *)dvobj->nvm;

	return lookup_flash_id(flash->dict, cmd, offset, size);
}

u32 rtw_hal_flash_read_map(struct flash_t *flash, u8 *map)
{

	memcpy(flash->shadow_map, map, flash->log_efuse_size);

	return _SUCCESS;
}

u32 rtw_hal_flash_set_hw_cap(struct flash_t *flash)
{

	return flash_set_hw_cap(flash);
}

void rtw_nvm_set_by_offset(void *dev, u32 offset, u32 value)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	rtw_flash_set_offset(flash_info, offset, value);
}

u8 rtw_nvm_get_by_offset(void *dev, u32 offset)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	return rtw_flash_get_offset(flash_info, offset);
}

void rtw_nvm_dump(void *dev)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	rtw_flash_dump(flash_info);
}

void rtw_hal_flash_set_para_path(void *dev, const char *path)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	rtw_flash_set_para_path(flash_info, path);
}

const char *rtw_nvm_get_para_path(void *dev)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	struct flash_t *flash_info = (struct flash_t*)dvobj->nvm;

	return rtw_flash_get_para_path(flash_info);
}

void rtw_get_fem_name(void *d, u8* fem_name, u8 name_len)
{
#ifdef CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct rtw_phl_com_t *phl_com = dvobj->phl_com;
	u8 band_cap;

	band_cap = phl_com->hal_spec.band_cap;

	_rtw_memset(fem_name, 0, name_len + 1);

	switch (band_cap) {
		case BAND_CAP_5G:
			strncpy(fem_name, rtw_fem_name_5g, name_len);
			break;
		case BAND_CAP_2G:
			strncpy(fem_name, rtw_fem_name_2g, name_len);
			break;
		default:
			RTW_ERR("%s: No such band cap(%d)\n", __FUNCTION__, band_cap);
			break;
	}
#else
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct rtw_phl_com_t *phl_com = dvobj->phl_com;
	u8 band_cap;

	band_cap = phl_com->hal_spec.band_cap;

	_rtw_memset(fem_name, 0, name_len + 1);

	switch (band_cap) {
#ifndef CONFIG_2G_ON_WLAN0
		case BAND_CAP_5G:
			#ifdef CONFIG_WLAN0_AP_5G
			strncpy(fem_name, "AP_5G", name_len);
			#endif

			#ifdef CONFIG_WLAN0_PON_5G
			strncpy(fem_name, "PON_5G", name_len);
			#endif
#else /* CONFIG_2G_ON_WLAN0 */
		case BAND_CAP_2G:
			#ifdef CONFIG_WLAN0_AP_2G
			strncpy(fem_name, "AP_2G", name_len);
			#endif

			#ifdef CONFIG_WLAN0_PON_2G
			strncpy(fem_name, "PON_2G", name_len);
			#endif
#endif /* CONFIG_2G_ON_WLAN0 */
			#ifdef CONFIG_WLAN0_FEM_VC5333
			strncpy(fem_name, "VC5333", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_VC5337
			strncpy(fem_name, "VC5337", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KCT8239SD
			strncpy(fem_name, "KCT8239SD", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85333
			strncpy(fem_name, "SKY85333", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85337_MSTC
			strncpy(fem_name, "SKY85337_MSTC", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85340
			strncpy(fem_name, "SKY85340", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85747
			strncpy(fem_name, "SKY85747", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85791
			strncpy(fem_name, "SKY85791", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC66204
			strncpy(fem_name, "RTC66204", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC66504
			strncpy(fem_name, "RTC66504", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC66506
			strncpy(fem_name, "RTC66506", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7676E
			strncpy(fem_name, "RTC7676E", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7676D
			strncpy(fem_name, "RTC7676D", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7676D_08U
			strncpy(fem_name, "RTC7676D_08U", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7676D1
			strncpy(fem_name, "RTC7676D1", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7646
			strncpy(fem_name, "RTC7646", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KCT8575HE
			strncpy(fem_name, "KCT8575HE", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KTC8570N
			strncpy(fem_name, "KTC8570N", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KCT8575HP
			strncpy(fem_name, "KCT8575HP", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KCT8539HE
			strncpy(fem_name, "KCT8539HE", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_KCT8531HE
			strncpy(fem_name, "KCT8531HE", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_SKY85755_MSTC
			strncpy(fem_name, "SKY85755_MSTC", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTK66287
			strncpy(fem_name, "RTK66287", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTK66587_DPX
			strncpy(fem_name, "RTK66587_DPX", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTK66587_BPF
			strncpy(fem_name, "RTK66587_BPF", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_FX7555D_PON
			strncpy(fem_name, "FX7555D_PON", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_FX7237D_PON
			strncpy(fem_name, "FX7237D_PON", name_len);
			#endif

			#ifdef CONFIG_WLAN0_FEM_RTC7663
			strncpy(fem_name, "RTC7663", name_len);
			#endif

			break;

#ifdef CONFIG_2G_ON_WLAN0
		case BAND_CAP_5G:
			#ifdef CONFIG_WLAN1_AP_5G
			strncpy(fem_name, "AP_5G", name_len);
			#endif

			#ifdef CONFIG_WLAN1_PON_5G
			strncpy(fem_name, "PON_5G", name_len);
			#endif
#else /* CONFIG_2G_ON_WLAN0 */
		case BAND_CAP_2G:
			#ifdef CONFIG_WLAN1_AP_2G
			strncpy(fem_name, "AP_2G", name_len);
			#endif

			#ifdef CONFIG_WLAN1_PON_2G
			strncpy(fem_name, "PON_2G", name_len);
			#endif
#endif /* CONFIG_2G_ON_WLAN0 */
			#ifdef CONFIG_WLAN1_FEM_VC5333
			strncpy(fem_name, "VC5333", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_VC5337
			strncpy(fem_name, "VC5337", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KCT8239SD
			strncpy(fem_name, "KCT8239SD", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85337_MSTC
			strncpy(fem_name, "SKY85337_MSTC", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85333
			strncpy(fem_name, "SKY85333", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85340
			strncpy(fem_name, "SKY85340", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85747
			strncpy(fem_name, "SKY85747", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85791
			strncpy(fem_name, "SKY85791", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC66204
			strncpy(fem_name, "RTC66204", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC66504
			strncpy(fem_name, "RTC66504", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC66506
			strncpy(fem_name, "RTC66506", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7676E
			strncpy(fem_name, "RTC7676E", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7676D
			strncpy(fem_name, "RTC7676D", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7676D_08U
			strncpy(fem_name, "RTC7676D_08U", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7676D1
			strncpy(fem_name, "RTC7676D1", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7646
			strncpy(fem_name, "RTC7646", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KCT8575HE
			strncpy(fem_name, "KCT8575HE", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KTC8570N
			strncpy(fem_name, "KTC8570N", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KCT8575HP
			strncpy(fem_name, "KCT8575HP", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KCT8539HE
			strncpy(fem_name, "KCT8539HE", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_KCT8531HE
			strncpy(fem_name, "KCT8531HE", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_SKY85755_MSTC
			strncpy(fem_name, "SKY85755_MSTC", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTK66287
			strncpy(fem_name, "RTK66287", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTK66587_DPX
			strncpy(fem_name, "RTK66587_DPX", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTK66587_BPF
			strncpy(fem_name, "RTK66587_BPF", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_FX7555D_PON
			strncpy(fem_name, "FX7555D_PON", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_FX7237D_PON
			strncpy(fem_name, "FX7237D_PON", name_len);
			#endif

			#ifdef CONFIG_WLAN1_FEM_RTC7663
			strncpy(fem_name, "RTC7663", name_len);
			#endif

			break;
		default:
			RTW_ERR("%s: No such band cap(%d)\n", __FUNCTION__, band_cap);
			break;
	}
#endif /* CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA */
}

void rtw_nvm_get_sub_dir_name(void *d, u8 rfe_type, char *sub_dir, u32 rfe_size)
{
#if 1
	snprintf(sub_dir, rfe_size, "RFE%u", rfe_type);
#else
	switch(rfe_type){
		 case 50:
			strcpy(rfe_name, "RFE50");
			break;
		 case 51:
			strcpy(rfe_name, "RFE51");
			break;
		 case 52:
			strcpy(rfe_name, "RFE52");
			break;
		 case 53:
			strcpy(rfe_name, "RFE53");
			break;
		 case 54:
			strcpy(rfe_name, "RFE54");
			break;
		 default:
			RTW_ERR("No RFE type %d\n", rfe_type);
			break;
	}
#endif
}

void rtw_nvm_get_ic_name(void *d, char *ic_name, u32 ic_name_size)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct rtw_phl_com_t *phl_com = dvobj->phl_com;
	char bus_char = '\0';

	if (phl_com->dev_cap.hw_sup_flags | HW_SUP_PCIE_PLFH) {
		bus_char = 'e';
	} else if (phl_com->dev_cap.hw_sup_flags | HW_SUP_USB_MULTI_FUN) {
		bus_char = 'u';
	} else if (phl_com->dev_cap.hw_sup_flags | HW_SUP_SDIO_MULTI_FUN) {
		bus_char = 's';
	}

	rtw_phl_get_ic_name(phl_com, ic_name, ic_name_size - 1);
	strncat(ic_name, &bus_char, 1);
}

#if defined(CONFIG_LOAD_PHY_PARA_FROM_FILE) && defined(RTW_CUSTOM_PARA_DIR) && defined(CONFIG_RTW_DRV_HAS_NVM)
void rtw_nvm_fill_full_para_path(void *dev, char *para_path, u32 path_size)
{
	/* support change path from flash */
	struct dvobj_priv *dvobj = (struct dvobj_priv *)dev;
	const char	*phy_folder = rtw_nvm_get_para_path(dvobj);
	u8		rfe_type;
	char		rfe_name[7]; //example RFE50a, 'a' is subtype
	char		ic_name[12];

	if (dvobj->phl_com)
		rfe_type = dvobj->phl_com->dev_cap.rfe_type;
	else
		rfe_type = rtw_rfe_type;

	rtw_nvm_get_sub_dir_name(dvobj, rfe_type, rfe_name, sizeof(rfe_name) - 1);
	rtw_nvm_get_ic_name(dvobj, ic_name, sizeof(ic_name) - 1);

	snprintf(para_path, path_size, "%s%s%c%s%c",
	         phy_folder, ic_name, '/', rfe_name, '/');
}

#else /* CONFIG_LOAD_PHY_PARA_FROM_FILE */

void rtw_nvm_fill_full_para_path(void *dev, char *para_path)
{
	para_path[0] = 0;
}
}
#endif /* CONFIG_LOAD_PHY_PARA_FROM_FILE */
