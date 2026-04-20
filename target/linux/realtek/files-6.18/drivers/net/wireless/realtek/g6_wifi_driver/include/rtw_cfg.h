/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
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
#ifndef _RTW_CFG_H_
#define _RTW_CFG_H_

u8 rtw_load_dvobj_registry(struct dvobj_priv *dvobj);
uint rtw_load_registry(_adapter *adapter);

void rtw_core_update_default_setting (struct dvobj_priv *dvobj);

#ifdef CONFIG_RTW_EFEM_IDENTIFY_WITH_FLASH
u8 rtw_check_efem_type(struct dvobj_priv *dvobj);  /* Should be called after read band & rfe type*/
#endif

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
extern char *rtw_phy_file_path;
extern uint rtw_rfe_type;
#endif /* CONFIG_LOAD_PHY_PARA_FROM_FILE */

#ifdef CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA
extern uint rtw_rfe_type_5g;
extern uint rtw_rfe_type_2g;
extern char *rtw_fem_name_5g;
extern char *rtw_fem_name_2g;
#endif /* CONFIG_LOAD_PHY_PARA_FROM_MODULE_PARA */

#endif /*_RTW_CFG_H_*/
