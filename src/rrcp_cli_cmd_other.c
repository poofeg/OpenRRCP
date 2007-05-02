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
#ifdef RTL83XX
#include "../lib/fake-libcli.h"
#else
#include "../lib/libcli.h"
#endif
#include "rrcp_cli.h"
#include "rrcp_io.h"
#include "rrcp_config.h"
#include "rrcp_switches.h"

int cmd_write_terminal(struct cli_def *cli, char *command, char *argv[], int argc)
{
    char text[32768];
#ifdef RTL83XX
    rrcp_config_read_from_switch();
#endif
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
    rrcp_config_bin2text(text,sizeof(text),1);
    cli_print(cli, "%s", text);
    return CLI_OK;
}

int cmd_write_memory(struct cli_def *cli, char *command, char *argv[], int argc)
{
    cli_print(cli, "%% Not implemented yet");
    return CLI_ERROR;
}

int cmd_copy_running_config(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc>0){
	if (argv[0][strlen(argv[0])-1]=='?'){
	    cli_print(cli, "  file:<filename>  Copy EEPROM to a specified binary file");
	}else{
	    cli_print(cli, "%% Not implemented yet");
	    return CLI_ERROR;
	}
	return CLI_OK;
    }else{
	cli_print(cli, "%% Invalid input detected.");
	return CLI_OK;
    }
}

int cmd_copy(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc==1){
	if (argv[0][strlen(argv[0])-1]=='?'){
	    cli_print(cli, "  file:<filename>  Copy from specified file");
	    return CLI_OK;
	}
    }
    if (argc==2){
	if (argv[1][strlen(argv[1])-1]=='?'){
	    cli_print(cli, "  eeprom  Copy to EEPROM");
	}else{
	    char *s;
	    uint8_t buf[2049];
	    int i,l;
	    FILE *f;

	    if (strncmp(argv[0], "file:",5)==0){
		s=argv[0]+5;
		if (strcmp(argv[1], "eeprom")==0){
		    if ((f=fopen(s,"r"))!=NULL){
		    	l=fread(buf,1,sizeof(buf),f);
			if (l!=eeprom_type_size[swconfig.eeprom_type]){
			    cli_print(cli, "%% ERROR: File size (%d bytes) does not match EEPROM size (%d bytes)",l,eeprom_type_size[swconfig.eeprom_type]);
			    return CLI_ERROR;
			}else{
			    cli_print(cli, "%% INFO: Successfully read %d bytes from file \"%s\".",l,s);
			}
		    	for (i=0;i<l;i++){
			    if ((i % 32)==0){
				fprintf(cli->client, "%% %4d bytes writen to EEPROM.\r", i);
			    }
			    if (eeprom_write(i,buf[i])){
			    	break;
			    }
		    	}
			fprintf(cli->client, "%% %4d bytes writen to EEPROM.\r\n", i);
		    	fclose(f);
			cli_print(cli, "%% WARNING: You need to reload switch to apply new configuration.");
			return CLI_OK;
		    }else{
			cli_print(cli, "%% ERROR: Can't open file \"%s\" for reading.",s);
			return CLI_ERROR;
		    }
		}else{
		    cli_print(cli, "%% ERROR: Unknown destination: \"%s\".",argv[1]);
		    return CLI_ERROR;
		}
	    }else{
		cli_print(cli, "%% Invalid input detected.");
		return CLI_ERROR;
	    }
	}
	return CLI_OK;
    }else{
	cli_print(cli, "%% Invalid input detected.");
	return CLI_ERROR;
    }
}

int cmd_copy_eeprom(struct cli_def *cli, char *command, char *argv[], int argc)
{
    if (argc>0){
	if (argv[0][strlen(argv[0])-1]=='?'){
	    cli_print(cli, "  file:<filename>  Copy EEPROM to a specified binary file");
	}else{
	    char *s;
	    uint8_t buf[2048];
	    int i;
	    FILE *f;

	    if (strncmp(argv[0], "file:",5)==0){
		s=argv[0]+5;
		if ((f=fopen(s,"w"))!=NULL){
		    for (i=0;i<2048;i++){
			if ((i % 32)==0){
			    fprintf(cli->client, "%% %4d bytes read from EEPROM.\r", i);
			}
			if (eeprom_read(i,&buf[i])){
			    break;
			}
		    }
		    fprintf(cli->client, "%% %4d bytes read from EEPROM.\r\n", i);
		    fwrite(buf,i,1,f);
		    fclose(f);
		    cli_print(cli, "%% INFO: Wrote EEPROM contents to file \"%s\" okay.",s);
		    return CLI_OK;
		}else{
		    cli_print(cli, "%% ERROR: Can't open file \"%s\" for writing.",s);
		    return CLI_ERROR;
		}
	    }else{
		cli_print(cli, "%% Invalid input detected.");
		return CLI_ERROR;
	    }
	}
	return CLI_OK;
    }else{
	cli_print(cli, "%% Invalid input detected.");
	return CLI_ERROR;
    }
}

#ifndef RTL83XX
void cmd_other_register_commands(struct cli_def *cli)
{
    struct cli_command *c;
    c = cli_register_command(cli, NULL, "write", NULL,  PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Write running configuration to memory or terminal");
    cli_register_command(cli, c, "memory", cmd_write_memory, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Write to NV memory");
    cli_register_command(cli, c, "terminal", cmd_write_terminal, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Write to terminal");

    {
	c = cli_register_command(cli, NULL, "copy", cmd_copy,  PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Copy from one file to another");
	cli_register_command(cli, c, "running-config", cmd_copy_running_config, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Copy from current system configuration");
	cli_register_command(cli, c, "eeprom", cmd_copy_eeprom, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Copy from EEPROM as binary file");
    }
}
#endif
