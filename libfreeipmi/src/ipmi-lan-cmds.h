/*
   ipmi-lan-cmds.h - IPMI LAN Commands

   Copyright (C) 2003 FreeIPMI Core Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _IPMI_LAN_CMDS_H
#define _IPMI_LAN_CMDS_H

#define IPMI_ENABLE_BMC_GENERATED_GRATUITOUS_ARPS     0x1
#define IPMI_DISABLE_BMC_GENERATED_GRATUITOUS_ARPS    0x0

#define IPMI_ENABLE_BMC_GENERATED_ARP_RESPONSES       0x1
#define IPMI_DISABLE_BMC_GENERATED_ARP_RESPONSES      0x0

#define IPMI_IP_ADDR_SOURCE_UNSPECIFIED                0x0
#define IPMI_IP_ADDR_SOURCE_STATIC                     0x1
#define IPMI_IP_ADDR_SOURCE_DHCP                       0x2
#define IPMI_IP_ADDR_SOURCE_BIOS                       0x3
#define IPMI_IP_ADDR_SOURCE_OTHER                      0x4

/* #define IPMI_ENABLE_AUTH_TYPE_NONE                  0x1 */
/* #define IPMI_DISABLE_AUTH_TYPE_NONE                 0x0 */
/* #define IPMI_ENABLE_AUTH_TYPE_MD2                   0x1 */
/* #define IPMI_DISABLE_AUTH_TYPE_MD2                  0x0 */
/* #define IPMI_ENABLE_AUTH_TYPE_MD5                   0x1 */
/* #define IPMI_DISABLE_AUTH_TYPE_MD5                  0x0 */
/* #define IPMI_ENABLE_AUTH_TYPE_STRAIGHT_PASSWORD     0x1 */
/* #define IPMI_DISABLE_AUTH_TYPE_STRAIGHT_PASSWORD    0x0 */
/* #define IPMI_ENABLE_AUTH_TYPE_OEM_PROPRIETARY       0x1 */
/* #define IPMI_DISABLE_AUTH_TYPE_OEM_PROPRIETARY      0x0 */

#define IPMI_AUTH_TYPE_NONE                  0x01
#define IPMI_AUTH_TYPE_MD2                   0x02
#define IPMI_AUTH_TYPE_MD5                   0x04
#define IPMI_AUTH_TYPE_STRAIGHT_PASSWORD     0x10
#define IPMI_AUTH_TYPE_OEM_PROPRIETARY       0x20

#define IPMI_BMC_GENERATED_GRATUITOUS_ARP_NO_SUSPEND    0x0
#define IPMI_BMC_GENERATED_GRATUITOUS_ARP_SUSPEND       0x1
#define IPMI_BMC_GENERATED_ARP_RESPONSE_NO_SUSPEND      0x0
#define IPMI_BMC_GENERATED_ARP_RESPONSE_SUSPEND         0x1
#define IPMI_BMC_GENERATED_GRATUITOUS_ARP_SUSPENDED     0x0
#define IPMI_BMC_GENERATED_GRATUITOUS_ARP_OCCURRING     0x1
#define IPMI_BMC_GENERATED_ARP_RESPONSE_SUSPENDED       0x0
#define IPMI_BMC_GENERATED_ARP_RESPONSE_OCCURRING       0x1

#define IPMI_LAN_CONF_GET_PARAMETER             0x0
#define IPMI_LAN_CONF_GET_PARAMETER_REVISION    0x1


#ifdef __cplusplus
extern "C" {
#endif

extern fiid_template_t tmpl_set_lan_conf_param_rs;

extern fiid_template_t tmpl_set_lan_conf_param_bmc_generated_arp_control_rq;
extern fiid_template_t tmpl_set_lan_conf_param_gratuitous_arp_interval_rq;
extern fiid_template_t tmpl_set_lan_conf_param_auth_type_enables_rq;

extern fiid_template_t tmpl_suspend_bmc_arps_rq;
extern fiid_template_t tmpl_suspend_bmc_arps_rs;

extern fiid_template_t tmpl_get_lan_conf_param_rq;
extern fiid_template_t tmpl_get_lan_conf_param_bmc_generated_arp_control_rs;
extern fiid_template_t tmpl_get_lan_conf_param_gratuitous_arp_interval_rs;
extern fiid_template_t tmpl_get_lan_conf_param_auth_type_enables_rs;
extern fiid_template_t tmpl_get_lan_conf_param_ip_addr_source_rs;
extern fiid_template_t tmpl_get_lan_conf_param_ip_addr_rs;
extern fiid_template_t tmpl_get_lan_conf_param_mac_addr_rs;
extern fiid_template_t tmpl_get_lan_conf_param_subnet_mask_rs;
extern fiid_template_t tmpl_get_lan_conf_param_gw_ip_addr_rs;
extern fiid_template_t tmpl_get_lan_conf_param_gw_mac_addr_rs;

int8_t ipmi_lan_set_arp (u_int8_t channel_number, 
			 u_int8_t bmc_generated_gratuitous_arps_flag, 
			 u_int8_t bmc_generated_arp_responses_flag, 
			 fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_gratuitous_arp_interval (u_int8_t channel_number, 
					     u_int8_t gratuitous_arp_interval, 
					     fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_auth_type_enables (u_int8_t channel_number, 
				       u_int8_t max_privilege_auth_type_callback_level, 
				       u_int8_t max_privilege_auth_type_user_level, 
				       u_int8_t max_privilege_auth_type_operator_level, 
				       u_int8_t max_privilege_auth_type_admin_level, 
				       u_int8_t max_privilege_auth_type_oem_level, 
				       fiid_obj_t obj_data_rs);

int8_t 
ipmi_lan_get_auth_type_enables (u_int8_t channel_number, 
				u_int8_t parameter_type, 
				u_int8_t set_selector, 
				u_int8_t block_selector, 
				fiid_obj_t obj_data_rs);

int8_t ipmi_suspend_bmc_arps (u_int8_t channel_number, 
			      u_int8_t gratuitous_arp_suspend, 
			      u_int8_t arp_response_suspend, 
			      fiid_obj_t obj_data_rs);

int8_t ipmi_lan_get_arp (u_int8_t channel_number, 
			 u_int8_t parameter_type, 
			 u_int8_t set_selector, 
			 u_int8_t block_selector, 
			 fiid_obj_t obj_data_rs);

int8_t ipmi_lan_get_gratuitous_arp_interval (u_int8_t channel_number, 
					     u_int8_t parameter_type, 
					     u_int8_t set_selector, 
					     u_int8_t block_selector, 
					     fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_ip_addr_source (u_int8_t channel_number,
				    u_int8_t ip_addr_source,
				    fiid_obj_t data_rs);

int8_t ipmi_lan_get_ip_addr_source (u_int8_t channel_number,
				    u_int8_t parameter_type,
				    u_int8_t set_selector,
				    u_int8_t block_selector,
				    fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_ip_addr (u_int8_t channel_number,
			     u_int32_t ip_addr,
			     fiid_obj_t data_rs);

int8_t ipmi_lan_get_ip_addr (u_int8_t channel_number,
			     u_int8_t parameter_type,
			     u_int8_t set_selector,
			     u_int8_t block_selector,
			     fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_gw1_ip_addr (u_int8_t channel_number,
				 u_int32_t ip_addr,
				 fiid_obj_t data_rs);

int8_t ipmi_lan_get_gw1_ip_addr (u_int8_t channel_number,
				 u_int8_t parameter_type,
				 u_int8_t set_selector,
				 u_int8_t block_selector,
				 fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_gw2_ip_addr (u_int8_t channel_number,
				 u_int32_t ip_addr,
				 fiid_obj_t data_rs);

int8_t ipmi_lan_get_gw2_ip_addr (u_int8_t channel_number,
				 u_int8_t parameter_type,
				 u_int8_t set_selector,
				 u_int8_t block_selector,
				 fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_subnet_mask (u_int8_t channel_number,
				 u_int32_t ip_addr,
				 fiid_obj_t data_rs);

int8_t ipmi_lan_get_subnet_mask (u_int8_t channel_number,
				 u_int8_t parameter_type,
				 u_int8_t set_selector,
				 u_int8_t block_selector,
				 fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_mac_addr (u_int8_t channel_number,
			      u_int64_t mac_addr,
			      fiid_obj_t data_rs);

int8_t ipmi_lan_get_mac_addr (u_int8_t channel_number,
			      u_int8_t parameter_type,
			      u_int8_t set_selector,
			      u_int8_t block_selector,
			      fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_gw1_mac_addr (u_int8_t channel_number,
				  u_int64_t mac_addr,
				  fiid_obj_t data_rs);

int8_t ipmi_lan_get_gw1_mac_addr (u_int8_t channel_number,
				  u_int8_t parameter_type,
				  u_int8_t set_selector,
				  u_int8_t block_selector,
				  fiid_obj_t obj_data_rs);

int8_t ipmi_lan_set_gw2_mac_addr (u_int8_t channel_number,
				  u_int64_t mac_addr,
				  fiid_obj_t data_rs);

int8_t ipmi_lan_get_gw2_mac_addr (u_int8_t channel_number,
				  u_int8_t parameter_type,
				  u_int8_t set_selector,
				  u_int8_t block_selector,
				  fiid_obj_t obj_data_rs);


#ifdef __cplusplus
}
#endif


#endif
