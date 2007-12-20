/*
    This file is part of OpenRRCP

    OpenRRCP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    OpenRRCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenRRCP; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    ---

    You can send your updates, patches and suggestions on this software
    to it's original author, Andrew Chernyak (nording@yandex.ru)
    This would be appreciated, however not required.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/libcli.h"
#include "rrcp_cli.h"
#include "rrcp_io.h"
#include "rrcp_config.h"
#include "rrcp_switches.h"

int cmd_show_version(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char s1[32];
    cli_print(cli, "OpenRRCP CLI, Version %s",RRCP_CLI_VERSION);
    cli_print(cli, "http://openrrcp.org.ru/");
    cli_print(cli, "Licensed under terms of GPL");
    cli_print(cli, "http://www.gnu.org/licenses/gpl.html#SEC1\n");

    cli_print(cli, "%d FastEthernet/IEEE 802.3 interface(s)",switchtypes[switchtype].num_ports >24 ? 24 : switchtypes[switchtype].num_ports);
    if (switchtypes[switchtype].num_ports >24){
	cli_print(cli, "%d Gigabit Ethernet/IEEE 802.3 interface(s)",switchtypes[switchtype].num_ports >24 ? switchtypes[switchtype].num_ports-24 : 0);
    }

    cli_print(cli, "\nBase ethernet MAC Address:  %02x:%02x:%02x:%02x:%02x:%02x",
		    dest_mac[0],
		    dest_mac[1],
		    dest_mac[2],
		    dest_mac[3],
		    dest_mac[4],
		    dest_mac[5]);
    cli_print(cli, "Vendor: %s",switchtypes[switchtype].vendor);
    cli_print(cli, "Model: %s",switchtypes[switchtype].model);
    cli_print(cli, "Chip: %s",chipnames[switchtypes[switchtype].chip_id]);
    cli_print(cli, "EEPROM: %s",eeprom_type_text[swconfig.eeprom_type]);
    cli_print(cli, "802.1Q support: %s",(switchtypes[switchtype].chip_id==rtl8326) ? "No/Buggy" : "Yes");
    cli_print(cli, "IGMP support: %s",(switchtypes[switchtype].chip_id==rtl8326) ? "v1" : "v1, v2");
    cli_print(cli, "Facing host interface: %s",ifname);
    cli_print(cli, "Facing switch interface: %s",rrcp_config_get_portname(s1, sizeof(s1), 
			map_port_number_from_physical_to_logical(swconfig.facing_switch_port_phys),
			swconfig.facing_switch_port_phys));
    return CLI_OK;
}

int cmd_show_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
    cli_print(cli, "! Sorry, permanent config saving not available yet.");
    cli_print(cli, "! Use \"show running-config\" to get running config.");
    return CLI_OK;
}

int cmd_show_running_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char text[32768];

    if (argc==1){
	if (strcmp(argv[0],"?")==0){
	    return CLI_OK;
	}else if (strcmp(argv[0],"full")!=0){
	    cli_print(cli, "%% Invalid input detected.");
	    return CLI_ERROR;
	}
    }

    cli_print(cli, "!");
    cli_print(cli, "! Config for %s %s switch at %02x:%02x:%02x:%02x:%02x:%02x@%s",
		    switchtypes[switchtype].vendor,
		    switchtypes[switchtype].model,
		    dest_mac[0],
		    dest_mac[1],
		    dest_mac[2],
		    dest_mac[3],
		    dest_mac[4],
		    dest_mac[5],
		    ifname);
    cli_print(cli, "! generated by OpenRRCP CLI ver %s",RRCP_CLI_VERSION);
    rrcp_config_bin2text(text,sizeof(text),(strcasecmp(command,"show running-config full")==0));
    cli_print(cli, "%s", text);
    return CLI_OK;
}

int cmd_show_interfaces(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc < 1){
	cli_print(cli, "%% Specify an interface");
	return CLI_ERROR;
    }
    if (strcmp(argv[0], "?") == 0){
	int i;
	char s1[30],s2[30];
	for(i=0;i<switchtypes[switchtype].num_ports;i+=2){
	    cli_print(cli,"%-19s %-19s",rrcp_config_get_portname(s1, sizeof(s1), i+1, i),rrcp_config_get_portname(s2, sizeof(s2), i+2, i+1));
	}
	cli_print(cli,"<%d-%d> - reference interface by its number",1,i);
	return CLI_OK;
    }else{
	char *a=argv[0];
	int port,port_phys;
	port=0;
	char s[20];

	if ((strlen(a)>0)&&('0'<=(a[strlen(a)-1]))&&((a[strlen(a)-1])<='9'))
	    port+=a[strlen(a)-1]-'0';
	if ((strlen(a)>1)&&('0'<=(a[strlen(a)-2]))&&((a[strlen(a)-2])<='9'))
	    port+=10*(a[strlen(a)-2]-'0');
	if (port>0 && port<=switchtypes[switchtype].num_ports){
	    union {
        	uint8_t raw;
        	struct t_rtl83xx_port_link_status struc;
    	    } link_status;
	    char *sp[4] = {"10","100","1000","???"};

	    port_phys=map_port_number_from_logical_to_physical(port);
	    link_status.raw=(uint8_t)(rtl83xx_readreg16(0x0619+port_phys/2)>>(8*(port_phys%2)));
	    cli_print(cli, "%s is %s, line protocol is %s",
		rrcp_config_get_portname(s, sizeof(s), port, port_phys),
		(swconfig.port_disable.bitmap&(1<<port_phys)) ? "administratively down" : 
		    (link_status.struc.link ? "up" : "down"),
		link_status.struc.link ? "up (connected)" : "down (notconnect)");
	    if (link_status.struc.link){
		cli_print(cli, "  %s, %sMb/s",
		    link_status.struc.duplex ? "Full-duplex" : "Half-duplex",
		    sp[link_status.struc.speed]);
	    }
	    cli_print(cli, "  PHY auto-negotiation %s",
		swconfig.port_config.config[port_phys].autoneg ? "on" : "off");
	    cli_print(cli, "  PHY capability:%s%s%s%s%s%s%s",
		swconfig.port_config.config[port_phys].media_10half ? " 10HD" : "",
		swconfig.port_config.config[port_phys].media_10full ? " 10FD" : "",
		swconfig.port_config.config[port_phys].media_100half ? " 100HD" : "",
		swconfig.port_config.config[port_phys].media_100full ? " 100FD" : "",
		swconfig.port_config.config[port_phys].media_1000full ? " 1000FD" : "",
		swconfig.port_config.config[port_phys].pause ? " PAUSE" : "",
		swconfig.port_config.config[port_phys].pause_asy ? " PAUSE_ASY" : "");
	    rtl83xx_setreg16(0x0700+(port_phys/2),0x0000);//read rx byte, tx byte, drop byte
	    cli_print(cli, "     %lu input bytes",(unsigned long)rtl83xx_readreg32(0x070d+port_phys));
	    cli_print(cli, "     %lu output bytes",(unsigned long)rtl83xx_readreg32(0x0727+port_phys));
	    cli_print(cli, "     %lu dropped bytes",(unsigned long)rtl83xx_readreg32(0x0741+port_phys));
	}else{
	    cli_print(cli, "Unknown interface %s", argv[0]);
	}
    }
    return CLI_OK;
}

int cmd_show_ip_igmp_snooping(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc>0){
	if (strcmp(argv[0],"?")==0){
	    cli_print(cli, "  |  Output modifiers");
	    cli_print(cli, "  <cr>");
	}else{
	    cli_print(cli, "%% Invalid input detected.");
	}
    }else{
	if (strcasecmp(command,"show ip igmp snooping")==0){
	    cli_print(cli, "Global IGMP Snooping configuration:");
	    cli_print(cli, "-----------------------------------");
	    cli_print(cli, "IGMP snooping              : %s",swconfig.alt_igmp_snooping.config.en_igmp_snooping ? "Enabled":"Disabled");
	}
	if (strcasecmp(command,"show ip igmp snooping mrouter")==0){
	    int port,port_phys;
	    char pn[64];

	    cli_print(cli, "Multicast routers found on port(s):");
	    for(port=1;port<=switchtypes[switchtype].num_ports;port++){
	        port_phys=map_port_number_from_logical_to_physical(port);
	        if (swconfig.alt_mrouter_mask.mask & (1<<port_phys)){
		    cli_print(cli, "%s",rrcp_config_get_portname(pn,sizeof(pn),port,port_phys));
		}
	    }
	}
    }
    return CLI_OK;
}

int cmd_show_switch_register(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc==1){
	if (strcmp(argv[0],"?")==0){
	    cli_print(cli, "  |  Output modifiers");
	    cli_print(cli, "  <0-fff> Specify register number (hex)");
	}else{
	    int regno;
	    if (sscanf(argv[0],"%x",&regno)==1){
		cli_print(cli, "  reg(0x%04x)=0x%04x",regno,rtl83xx_readreg16(regno));
	    }else{
		cli_print(cli, "%% ERROR: Invalig register number: '%s'.",argv[0]);
	    }
	}
	return CLI_OK;
    }
    cli_print(cli, "%% Invalid input detected.");
    return CLI_ERROR;
}

int cmd_show_eeprom_register(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc==1){
	if (strcmp(argv[0],"?")==0){
	    cli_print(cli, "  |  Output modifiers");
	    cli_print(cli, "  <0-fff> Specify register number (hex)");
	}else{
	    int regno;
	    unsigned char regval;
	    if (sscanf(argv[0],"%x",&regno)==1){
		if (eeprom_read(regno,&regval)==0){
		    cli_print(cli, "  eeprom(0x%04x)=0x%02x",regno,regval);
		}else{
		    cli_print(cli, "%% ERROR: Can't access EEPROM register 0x%04x.",regno);
		}
	    }else{
		cli_print(cli, "%% ERROR: Invalig register number: '%s'.",argv[0]);
	    }
	}
	return CLI_OK;
    }
    cli_print(cli, "%% Invalid input detected.");
    return CLI_ERROR;
}

int cmd_show_phy_register(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc==1){
	if (strcmp(argv[0],"?")==0){
	    cli_print(cli, "  |  Output modifiers");
	    cli_print(cli, "  <2-3,8-31> Specify register number (dec)");
	}else{
	    uint16_t regno,regval[4];
	    int i,j;	    

	    if (sscanf(argv[0],"%d",&i)==1){
		regno=(uint16_t)i;
		cli_print(cli, "  PHY number %d registers hex dump:",(int)regno);
		cli_print(cli, "   +0x00  +0x08  +0x10  +0x18");
		for(i=0;i<8;i++){
		    for(j=0;j<4;j++){
			if (phy_read(regno,i+j*8,&regval[j])!=0){
			    cli_print(cli, "%% ERROR: Can't access PHY number %d, register %d(0x%02d)",regno,i+j*8,i+j*8);
			    break;
			}
		    }
		    cli_print(cli, "  0x%04x 0x%04x 0x%04x 0x%04x",(int)regval[0],(int)regval[1],(int)regval[2],(int)regval[3]);
		}
	    }else{
		cli_print(cli, "%% ERROR: Invalig register number: '%s'.",argv[0]);
	    }
	}
	return CLI_OK;
    }
    cli_print(cli, "%% Invalid input detected.");
    return CLI_ERROR;
}

int cmd_test(struct cli_def *cli, char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "called %s with \"%s\"", __FUNCTION__, command);
    cli_print(cli, "%d arguments:", argc);
    for (i = 0; i < argc; i++)
    {
	cli_print(cli, "	%s", argv[i]);
    }
    return CLI_OK;
}

void cmd_show_register_commands(struct cli_def *cli)
{
    struct cli_command *c;
    c = cli_register_command(cli, NULL, "show", NULL,  PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Show running system information");
    cli_register_command(cli, c, "version", cmd_show_version, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "System hardware and software status");
    cli_register_command(cli, c, "configuration", cmd_show_config, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Contents of Non-Volatile memory");
    cli_register_command(cli, c, "switch-register", cmd_show_switch_register, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Contents of internal switch controller register");
    cli_register_command(cli, c, "eeprom-register", cmd_show_eeprom_register, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Contents of internal switch EEPROM register");
    cli_register_command(cli, c, "phy-register", cmd_show_phy_register, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Contents of internal switch PHY register");
    {//show running-config
	struct cli_command *show_runningconfig;
	show_runningconfig = cli_register_command(cli, c, "running-config", cmd_show_running_config, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Current operating configuration");
	cli_register_command(cli, show_runningconfig, "full", cmd_show_running_config, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Current operating configuration including default statements");
    }
    cli_register_command(cli, c, "startup-config", cmd_show_config, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Contents of startup configuration");
    cli_register_command(cli, c, "interfaces", cmd_show_interfaces, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Interface status and configuration");
    {//show ip igmp snooping
	struct cli_command *show_ip,*show_ip_igmp,*show_ip_igmp_snooping;
	show_ip = cli_register_command(cli, c, "ip", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, "IP information");
	show_ip_igmp = cli_register_command(cli, show_ip, "igmp", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, "IGMP information");
	show_ip_igmp_snooping = cli_register_command(cli, show_ip_igmp, "snooping", cmd_show_ip_igmp_snooping, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Snooping info on this switch");
	cli_register_command(cli, show_ip_igmp_snooping, "mrouter", cmd_show_ip_igmp_snooping, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Show multicast routers on this switch");
    }
}
