/* cpcd_config.c - Configuration Source File for CPCD
 * Author:    V. Alex Brennen (codepoet@dublin.ie)
 *            [ http://cryptnet.net/people/vab/ ]
 * Created:   2003.07.01
 * Modified:  2005.04.18
 *
 * This file is part of the CryptNET Peer Cache Daemon.
 * Full sourcecode and documentation is available from
 * the project's web page: http://cryptnet.net/fsp/cpcd/
 *
 * cpcd is free software; it has been placed in the public
 * domain.  If you wish to retain the copyright to your
 * patch, do not submit it to CryptNET.
 */

#include "cpcd_config.h"


int init_config(struct cpcd_ctx **ctx, struct cpcd_config **config)
{
	int rslt = 0;

	memset((*config)->allowed_networks,0x00,255);
	memset((*config)->default_allow,0x00,255);
	memset((*config)->default_network,0x00,255);
	memset((*config)->cpcd_version,0x00,49);
	memset((*config)->server_url,0x00,255);
	memset((*config)->data_dir,0x00,255);

	strncpy((*config)->allowed_networks,"gnutella gnutella2 MetaNET",35);
	strncpy((*config)->default_network,"gnutella",35);
	strncpy((*config)->default_allow,"LIME GTKG GNUC BEAR MRPH MESH RAZA ACQX MNAP SWAP MUTE TEST META XOLO QTLA PHEX KIWI TFLS GALA ACQL GNZL GDNA GIFT",200);
	strncpy((*config)->cpcd_version,"CryptNET cpcd 1.0.0",35);
	strncpy((*config)->server_url,"localhost",35);
	strncpy((*config)->bind_ip,"0.0.0.0",10);
	strncpy((*config)->web_bind_port,"8080",5);
	strncpy((*config)->gnutella_bind_port,"6346",5);
	strncpy((*config)->data_dir,"/var/tmp/data",255);
	strncpy((*config)->log_file,"/var/log/cpcd.log",255);
	strncpy((*config)->pid_file,"/var/run/cpcd.pid",255);
	

	(*config)->cpcd_keep_log = 0;
	(*config)->cpcd_throttle_support = 0;
	
	(*config)->max_bad_urls = 500;
	
	(*config)->max_hosts = 150;
	(*config)->max_urls = 20;
	(*config)->max_nets = 5;
	
	(*config)->listen_threads = 100;
	(*config)->host_threads = 20;
	
	(*config)->port_restriction = 0;

	rslt = read_config(config);
	if(rslt != 0)
	{
        	fprintf(stderr,"An error occured while attempting to read the configuration file.\n");
		fprintf(stderr,"Some default config values may be used.\n");
	}
	
	load_network_caches(ctx, (*config)->allowed_networks);
	
	return 0;
}


int read_config(struct cpcd_config **config)
{
	FILE *conf_file = NULL;
        char line[513];
	char file_loc[1024];
	char *p = NULL;

	
	memset(line,0x00,513);
	memset(file_loc,0x00,1024);
	
	if(file_exists(CONF))
	{
		strncpy(file_loc,CONF,1023);
	}
	else if(file_exists("/etc/cpcd.conf"))
	{
		strncpy(file_loc,"/etc/cpcd.conf",63);
	}
	else if(file_exists("/usr/local/etc/cpcd.conf"))
	{
		strncpy(file_loc,"/usr/local/etc/cpcd.conf",63);
	}
	else if(file_exists("/home/cpcd/etc/cpcd.conf"))
	{
		strncpy(file_loc,"/home/cpcd/etc/cpcd.conf",63);
	}
	else if(file_exists("../cpcd.conf"))
	{
		strncpy(file_loc,"../cpcd.conf",63);
	}
	else
	{
		fprintf(stderr,"CPCD: Unable to find configuration file.\n");
		
		return -1;
	}
	
	if((conf_file = fopen(file_loc,"r")) == NULL)
        {
		fprintf(stderr,"Failed to open configuration file: %s\n",file_loc);
	
		return -1;
        }

	while(fgets(line,512,conf_file) != NULL)
        {
		if( (!(memcmp(line,"#",1) == 0)) && (!(isspace(line[0]))) )
                {
			unsigned char *name = NULL;
                        unsigned char *value = NULL;

                        name = &line[0];
			p = memchr(name,' ',512);
			if(p == NULL)
				continue;
                        value = ++p;
			if(value == NULL)
				continue;
			p='\0';
			value[strlen(value)-1] = '\0';

   			if(memcmp(name,"allowed_networks",16) == 0)
                        {
        			strncpy((*config)->allowed_networks,value,255);
           		}
			else if(memcmp(name,"default_network",15) == 0)
                        {
        			strncpy((*config)->default_network,value,255);
        		}
			else if(memcmp(name,"allow_servlets",14) == 0)
                        {
        			strncpy((*config)->default_allow,value,255);
        		}
                        else if(memcmp(name,"server_url",10) == 0)
                        {
        			strncpy((*config)->server_url,value,255);
        		}
                        else if(memcmp(name,"bind_ip",7) == 0)
                        {
        			strncpy((*config)->bind_ip,value,15);
        		}
                        else if(memcmp(name,"web_bind_port",9) == 0)
                        {
        			strncpy((*config)->web_bind_port,value,9);
        		}
                        else if(memcmp(name,"gnutella_bind_port",18) == 0)
                        {
        			strncpy((*config)->gnutella_bind_port,value,9);
        		}
                        else if(memcmp(name,"data_dir",8) == 0)
                        {
        			strncpy((*config)->data_dir,value,255);
        		}
                        else if(memcmp(name,"cpcd_keep_log",13) == 0)
                        {
        			(*config)->cpcd_keep_log = atoi(value);
        		}
                        else if(memcmp(name,"log_dir",7) == 0)
                        {
        			snprintf((*config)->log_file,255,"%s/cpcd.log",value);
        		}
                        else if(memcmp(name,"pid_dir",7) == 0)
                        {
        			snprintf((*config)->pid_file,255,"%s/cpcd.pid",value);
        		}
                        else if(memcmp(name,"cpcd_throttle_support",21) == 0)
                        {
        			(*config)->cpcd_throttle_support = atoi(value);
        		}
                        else if(memcmp(name,"max_hosts",9) == 0)
                        {
        			(*config)->max_hosts = atoi(value);
        		}
                        else if(memcmp(name,"max_urls",8) == 0)
                        {
        			(*config)->max_urls = atoi(value);
        		}
                        else if(memcmp(name,"max_nets",8) == 0)
                        {
        			(*config)->max_nets = atoi(value);
        		}
			else if(memcmp(name,"listen_threads",14) == 0)
			{
				(*config)->listen_threads = atoi(value);
			}
			else if(memcmp(name,"host_threads",12) == 0)
			{
				(*config)->host_threads = atoi(value);
			}
			else if(memcmp(name,"port_restriction",16) == 0)
			{
				(*config)->port_restriction = atoi(value);
			}
                        else
                        {
                        	fprintf(stderr,"cpcd_config:  Rejected config var: %s\n",name);
                        }
                }
        }
        fclose(conf_file);

	return 0;
}
