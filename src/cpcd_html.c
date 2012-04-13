/* cpcd_html.c - HTML Output Source File for CPCD
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

#include "cpcd_html.h"


/* Print the default HTML Page */
long html_page(int c, struct cpcd_ctx *ctx)
{
	struct hosts *walk_cached_hosts = NULL;
	time_t current_time;

	char buff[256];
	unsigned int buff_size = 255;
	char tmpdate[256];
	char *tmpdate_ptr = NULL;
	unsigned long total_bytes = 0;
	int rslt = 0;
	struct networks *current_network = NULL;

	
	memset(buff,0x00,256);
	memset(tmpdate,0x00,256);

	current_time = time(NULL);
	
	tmpdate_ptr = &tmpdate[0];

	/* Output Everything */
	rslt = write_line_to_socket(c,"HTTP/1.1 200 OK\r\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"Content-type: text/html\r\n\r\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<HTML><HEAD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TITLE>CryptNET Peer Cache</TITLE>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<STYLE TYPE=\"text/css\">\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"BODY,P,TD  {\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"	font-family: Tahoma, Arial, Helvetica, sans-serif;\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"	font-size: 10pt;\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"}\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"H1 { font-size: 16pt; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"H3 { font-size: 12pt; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"TD { font-size: 8pt; white-space: nowrap; background: lightblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"TH { font-size: 8pt; font-weight: bold; white-space: nowrap; background: lightsteelblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A { color: blue; text-decoration: none; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A:Hover { text-decoration: underline; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A:Visited { color:darkblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</STYLE>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</HEAD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<BODY>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<H1><A HREF=\"http://cryptnet.net/\">CryptNET</A> Peer Cache</H1>\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<P>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"Running: <A HREF=\"http://cryptnet.net/fsp/cpcd/\">%s</A> on %s<BR>\n", ctx->config->cpcd_version, ctx->config->server_url);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"by <A HREF=\"http://cryptnet.net/people/vab/\">V. Alex Brennen</A> (<A HREF=\"mailto:codepoet@dublin.ie\">codepoet@dublin.ie</A>)\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</P>\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<H3>Data</H3>\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;

	current_network = get_network_by_name(ctx->networks,ctx->config->default_network);
	if(current_network != NULL)
	{
		rslt = write_line_to_socket(c,"<P><TABLE BORDER=0 CELLPADDING=3 CELLSPACING=2>\n");
		if(rslt == -1)
			return rslt;
		else
			total_bytes += rslt;
		snprintf(buff,255,"<TR><TH COLSPAN=2>Network: %s</TH></TR>\n",current_network->name);
		rslt = write_line_to_socket(c,buff);
		if(rslt == -1)
			return rslt;
		else
			total_bytes += rslt;
		rslt = write_line_to_socket(c,"<TR><TH>Host Info</TH><TH>Age</TH></TR>\n");
		if(rslt == -1)
			return rslt;
		else
			total_bytes += rslt;
		walk_cached_hosts = get_first_host(current_network->hosts);
	
		while(walk_cached_hosts != NULL)
		{
			if(walk_cached_hosts->type == 'H')
			{
				rslt = get_age_r(&(walk_cached_hosts->bdate),&tmpdate_ptr,buff_size);
				if(rslt != -1)
				{
					snprintf(buff,255,"<TR><TD><A HREF=\"http://%s\">%s</A></TD><TD>%s</TD></TR>\n",walk_cached_hosts->addr,
								walk_cached_hosts->addr,tmpdate_ptr);
					rslt = write_line_to_socket(c,buff);
				}
			}
	
			walk_cached_hosts = walk_cached_hosts->next;
		}

		write_line_to_socket(c,"<TR><TH>Cache URL</TH><TH>Age</TH></TR>\n");
		if(rslt == -1)
			return rslt;
		else
			total_bytes += rslt;
		walk_cached_hosts = get_first_host(current_network->hosts);
	
		while(walk_cached_hosts != NULL)
		{
			if(walk_cached_hosts->type == 'U')
			{
				rslt = get_age_r(&(walk_cached_hosts->bdate),&tmpdate_ptr,buff_size);
				if(rslt != -1)
				{
					snprintf(buff,255,"<TR><TD><A HREF=\"%s\">%s</A></TD><TD>%s</TD></TR>\n",walk_cached_hosts->addr,
								walk_cached_hosts->addr,tmpdate_ptr);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
						return rslt;
					else
						total_bytes += rslt;
				}
			}
			walk_cached_hosts = walk_cached_hosts->next;
		}
		
		rslt = write_line_to_socket(c,"</TABLE></P>\n");
		if(rslt == -1)
			return rslt;
		else
			total_bytes += rslt;
	}

	rslt = write_line_to_socket(c,"<P><TABLE BORDER=0 CELLPADDING=3 CELLSPACING=2>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TR><TH COLSPAN=3>Alternate Networks</TH></TR>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;

	current_network = get_first_network(ctx->networks);
	while(current_network != NULL)
	{
		if(strcmp(current_network->name,ctx->config->default_network) != 0)
		{
			walk_cached_hosts = get_first_host(current_network->hosts);
		
			rslt = write_line_to_socket(c,"<TR><TH>Network</TH><TH>Cache URL</TH><TH>Age</TH></TR>\n");
			if(rslt == -1)
				return rslt;
			else
				total_bytes += rslt;
			
			while(walk_cached_hosts != NULL)
			{
				if(walk_cached_hosts->type == 'U')
				{
					rslt = get_age_r(&(walk_cached_hosts->bdate),&tmpdate_ptr,buff_size);
					if(rslt != -1)
					{
						snprintf(buff,255,"<TR><TD>%s</TD><TD><A HREF=\"%s\">%s</A></TD><TD>%s</TD></TR>\n",
							current_network->name, walk_cached_hosts->addr,walk_cached_hosts->addr,tmpdate_ptr);
						rslt = write_line_to_socket(c,buff);
					}
				}
				walk_cached_hosts = walk_cached_hosts->next;
			}
		}
		current_network = current_network->next;
	}

	rslt = write_line_to_socket(c,"</TABLE></P>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	
	rslt = write_line_to_socket(c,"<!-- Config Data\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"Allowed Nets: %s\n", ctx->config->allowed_networks);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"Default Net: %s\n", ctx->config->default_network);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"Max Hosts: %d\n", ctx->config->max_hosts);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"Max URLS: %d\n", ctx->config->max_urls);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"-->\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</BODY></HTML>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;

	
	return total_bytes;
}


long stats_page(int c, struct cpcd_ctx *ctx)
{
	char buff[256];
	int host_count = 0;
	int url_count = 0;
	unsigned long total_bytes = 0;
	int rslt = 0;
	struct networks *default_network = NULL;
	

	memset(buff,0x00,256);

	rslt = write_line_to_socket(c,"HTTP/1.1 200 OK\r\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"Content-type: text/html\r\n\r\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<HTML><HEAD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"        <TITLE>CryptNET Peer Cache Statistics</TITLE>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<STYLE TYPE=\"text/css\">\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"BODY,P,TD  {\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"        font-family: Tahoma, Arial, Helvetica, sans-serif;\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"        font-size: 10pt;\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"}\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"H1 { font-size: 16pt; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"H3 { font-size: 12pt; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"TD { font-size: 8pt; white-space: nowrap; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"TH { font-size: 8pt; font-weight: bold; white-space: nowrap; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"TD.cent { text-align: center; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,".lite { background: lightblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,".dark { background: lightsteelblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A { color: blue; text-decoration: none; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A:Hover { text-decoration: underline; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"A:Visited { color:darkblue; }\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</STYLE></HEAD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<BODY>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<H1><A HREF=\"http://cryptnet.net/\">CryptNET</A> <A HREF=\"http://cryptnet.net/fsp/cpcd/\">Peer Cache</A> Statistics</H1>\n\n\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<P>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"[ <A HREF=\"pwc.cgi\">Home</A>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"| <A HREF=\"pwc.cgi?stats=1&client=TEST&version=1.0\">Statistics</A>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"| <A HREF=\"pwc.cgi?get=1&client=TEST&version=1.0\">Data</A> ]\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</P>\n\n\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=2>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TR><TD CLASS=dark WIDTH=50%%>&nbsp;Total Requests:</TD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"        <TD CLASS=lite WIDTH=50%%>&nbsp;%ln</TD> </TR>\n",&(ctx->total_hits));
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TR><TD CLASS=dark VALIGN=Top>&nbsp;Requests:</TD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;

	snprintf(buff,255,"<TD CLASS=lite>&nbsp;%ln Update, %ln Host/URL-File<BR>\n\n",&(ctx->updts),&(ctx->reqs));
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TR><TD CLASS=dark>&nbsp;Hosts in Cache:</TD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
		
	default_network = get_network_by_name(ctx->networks, ctx->config->default_network);
	if(default_network->hosts != NULL)
	{
		host_count = count_nodes(default_network->hosts,D_HOSTS);
		url_count = count_nodes(default_network->hosts,D_URLS);
	}
	
	snprintf(buff,255,"        <TD CLASS=lite>&nbsp; (%d of %d)</TD></TR>\n",host_count,ctx->config->max_hosts);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"<TR><TD CLASS=dark>&nbsp;Alternate Caches:</TD>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	snprintf(buff,255,"        <TD CLASS=lite>&nbsp; (%d of %d)</TD></TR>\n",url_count,ctx->config->max_urls);
	rslt = write_line_to_socket(c,buff);
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	
	rslt = write_line_to_socket(c,"</TABLE>\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;
	rslt = write_line_to_socket(c,"</BODY></HTML>\n\n");
	if(rslt == -1)
		return rslt;
	else
		total_bytes += rslt;


	return total_bytes;
}
