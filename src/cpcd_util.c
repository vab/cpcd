/* cpcd_util.c - Utility Functions for CPCD
 * Author:    V. Alex Brennen (codepoet@dublin.ie)
 *            [ http://cryptnet.net/people/vab/ ]
 * Created:   2003.07.01
 * Modified:  2005.05.01
 *
 * This file is part of the CryptNET Peer Cache Daemon.
 * Full sourcecode and documentation is available from
 * the project's web page: http://cryptnet.net/fsp/cpcd/
 *
 * cpcd is free software; it has been placed in the public
 * domain.  If you wish to retain the copyright to your
 * patch, do not submit it to CryptNET.
 */

#include "cpcd_util.h"


static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;


int need_more_caches(struct hosts *caches, int max)
{
	int count = 0;
	struct hosts *walk_host = NULL;
	unsigned long age = 0;
	time_t now = 0;


	now = time(NULL);
	count = count_nodes(caches,D_URLS);
	if(count < max)
		return 1;

	walk_host = get_first_host(caches);
	if(walk_host == NULL)
		return 1;

	while(walk_host != NULL)
	{
		if(walk_host->type == 'U')
		{
			age = now - walk_host->bdate;
			if(age > 86400)
				return 1;
		}
		walk_host = walk_host->next;
	}


	return 0;
}


int clean_url_r(char *url,char **clean_url,int buff_size)
{
	char *h = NULL;
	char *p = NULL;
	char *q = NULL;
	char *r = NULL;
	char *s = NULL;
	char hostname[256];
	char port[256];
	char fileloc[256];
	char *file = NULL;
	unsigned long portnum = 0;
	unsigned int len = 0;
	int rslt = 0;


	if(url == NULL)
	{
		fprintf(stderr,"System Error: clean_url was passed a NULL apgument\n");

		return -1;
	}
	memset(hostname,0x00,256);
	memset(port,0x00,256);
	memset(fileloc,0x00,256);

	rslt = memcmp(url,"http://",7);
	if(rslt != 0)
	{
		return -1;
	}
	p = strstr(url,"://");
	if(p == NULL)
		return -1;
	p = p+3;
	if(p == NULL)
		return -1;
	q = strchr(p,':');
	if(q == NULL)
	{
		/* There's no port info */
		q = strchr(p,'/');
		if(q == NULL)
		{
			strncpy(hostname,p,255);
			/* we're done */
		}
		/* there's a file loc */
		else
		{
			r = q;
			r++;
			*q = '\0';
			strncpy(hostname,p,255);
			strncpy(fileloc,r,255);
		}
	}
	else
	{
		/* There's a port */
		r = q;
		r++;
		*q = '\0';
		strncpy(hostname,p,255);
		/* store ptr to port information in q */
		q = strchr(r,'/');
		if(q == NULL)
		{
			/* There's no file loc, only port */
			strncpy(port,r,255);
			strncpy(fileloc,q,255);
		}
		else
		{
			/* File loc and port */
			s = q;
                        *s='\0';
                        q++;
			strncpy(port,r,255);
			strncpy(fileloc,q,255);
		}
	}
	len = strlen(hostname);
	len--;
	if(hostname[len] == '.')
		hostname[len] = '\0';
	if(isdigit(hostname[len]))
	{
		return -1;
	}
	file = &fileloc[0];
	if(file != NULL)
	{
		while((file[0] == '.') || (file[0] == '/') )
		{
			file++;
		}
	}
	if(isdigit(port[0]))
	{
		portnum = atol(port);
		if( (portnum < 1) || (portnum > 32000) )
		{
			return -1;
		}
	}

	/* Convert the host name to lowercase for when we search for duplicates.
	   We could store the IP the hostname resolves to and search for duplicate
	   ip, bet the limited number of caches and the small amount of system 
	   resources cpcd takes is a valid reason to run multiple caches on a single
	   machine with different CNAMEs.
	*/
	h = &hostname[0];
	str_to_lower(&h);
	
	if(portnum == 0)
	{
		snprintf(*clean_url,buff_size-1,"http://%s/%s",hostname,file);
	}
	else
	{
		snprintf(*clean_url,buff_size-1,"http://%s:%ln/%s",hostname, &portnum, file);
	}

	return 0;
}


int verify_url(struct cpcd_ctx *ctx, char *data)
{
	char *p = NULL;
	char *q = NULL;
	char *r = NULL;
	char *s = NULL;
	char urlb[256];
	char *url = NULL;
	char hostname[256];
	char port[256];
	char fileloc[256];
	char newurl[256];
	char tmp_buffer[256];
	char *file = NULL;
	int  rslt = 0;
	unsigned long portnum = 0;
	unsigned int len = 0;

	int v1_rslt = 0;
	int v2_rslt = 0;


	memset(urlb,0x00,256);
	memset(hostname,0x00,256);
	memset(port,0x00,256);
	memset(fileloc,0x00,256);
	memset(tmp_buffer,0x00,256);

	strncpy(urlb,data,255);
	url = &urlb[0];

	rslt = memcmp(url,"http://",7);
	if(rslt != 0)
	{
		return -1;
	}
	p = strstr(url,"://");
	if(p == NULL)
		return -1;
	p = p+3;
	if(p == NULL)
		return -1;
	q = strchr(p,':');
	if(q == NULL)
	{
		/* There's no port info */
		q = strchr(p,'/');
		if(q == NULL)
		{
			strncpy(hostname,p,255);
			/* we're done */
		}
		/* there's a file loc */
		else
		{
			r = q;
			r++;
			*q = '\0';
			strncpy(hostname,p,255);
			strncpy(fileloc,r,255);
		}
	}
	else
	{
		/* There's a port */
		r = q;
		r++;
		*q = '\0';
		strncpy(hostname,p,255);
		/* store ptr to port information in q */
		q = strchr(r,'/');
		if(q == NULL)
		{
			/* There's no file loc, only port */
			strncpy(port,r,255);
			strncpy(fileloc,q,255);
		}
		else
		{
			/* File loc and port */
			s = q;
                        *s='\0';
                        q++;
			strncpy(port,r,255);
			strncpy(fileloc,q,255);
		}
	}
	len = strlen(hostname);
	len--;
	if(hostname[len] == '.')
		hostname[len] = '\0';
	if(isdigit(hostname[len]))
	{
		return -1;
	}
	file = &fileloc[0];
	if(file != NULL)
	{
		while((file[0] == '.') || (file[0] == '/') )
		{
			file++;
		}
	}
	if(isdigit(port[0]))
	{
		portnum = atol(port);
		if( (portnum < 1) || (portnum > 32000) )
		{
			return -1;
		}
	}
	if(portnum == 0)
	{
		snprintf(newurl,255,"http://%s/%s",hostname,file);
		portnum=80;
	}
	else
	{
		snprintf(newurl,255,"http://%s:%ln/%s",hostname, &portnum, file);
	}

	v1_rslt = ping_cache_v1(ctx,file,hostname,portnum);
	v2_rslt = ping_cache_v2(ctx,file,hostname,portnum);

	if((v1_rslt == 0) && (v2_rslt == 0))
	{
		return 0;
	}
	else if((v1_rslt == 1) && (v2_rslt == 0))
	{
		return 1;
	}
	else if((v1_rslt == 0) && (v2_rslt == 1))
	{
		return 2;
	}
	else if((v1_rslt == 1) && (v2_rslt == 1))
	{
		return 3;
	}

	return 0;
}


int ping_cache_v1(struct cpcd_ctx *ctx, char *file, char *hostname, unsigned long portnum)
{
	int sockfd = 0;
	struct sockaddr_in addr;
	struct hostent ret;
	struct hostent hostinfo;
	struct hostent *hostinfo_p = NULL;
	char hostent_buff[1024];

	char tmp_buffer[256];
	int rslt = 0;
	unsigned int len = 0;
	int h_errnop = 0;


	memset(tmp_buffer,0x00,256);
	memset(hostent_buff,0x00,1024);


	hostinfo_p = &hostinfo;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		return -1;
	}

	addr.sin_family = AF_INET;

	strncpy(tmp_buffer,hostname,1);
	if(isdigit(tmp_buffer[0]))
	{
		addr.sin_addr.s_addr = inet_addr(hostname);
	}
	else
	{
		rslt = gethostbyname_r(hostname, &ret, hostent_buff, 1024,&hostinfo_p,&h_errnop);
		if((rslt != 0) || (hostinfo_p == NULL))
		{
			close(sockfd);

			return -1;
		}
		addr.sin_addr = *(struct in_addr *)*hostinfo_p->h_addr_list;
	}
	addr.sin_port = htons(portnum);
	len = sizeof(addr);

	rslt = connect_nonblock(ctx,&sockfd,(struct sockaddr *)&addr,len);
	if(rslt == -1)
	{
		close(sockfd);

		return -1;
	}

	snprintf(tmp_buffer,255,"GET /%s%s HTTP/1.1\r\n",file,gwc_ping);
	write_line_to_socket(sockfd,tmp_buffer);

	snprintf(tmp_buffer,255,"Host: %s\r\n\r\n",hostname);
	write_line_to_socket(sockfd,tmp_buffer);

	/* read the response */
	rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
	if(rslt < 2)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}

	/*  What did the socket report back?  Was it code 200? */
	if(strstr(tmp_buffer,"200 OK") == NULL)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}

	do
	{
		rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
		if(rslt < 2)
		{
			shutdown(sockfd,SHUT_RDWR);
			close(sockfd);

			return -1;
		}
	} while((tmp_buffer[0] != '\r') && (tmp_buffer[0] != '\n'));

	memset(tmp_buffer,0x00,256);
	rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
	if(rslt < 2)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		return -1;
	}
	if( (strstr(tmp_buffer,"PONG")== NULL) && (strstr(tmp_buffer,"pong")== NULL) )
	{

		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return	0;
	}

	shutdown(sockfd,SHUT_RDWR);
	close(sockfd);

	return 1;
}


int ping_cache_v2(struct cpcd_ctx *ctx, char *file, char *hostname, unsigned long portnum)
{
	int sockfd = 0;
	struct sockaddr_in addr;
	struct hostent ret;
	struct hostent hostinfo;
	struct hostent *hostinfo_p = NULL;
	char hostent_buff[1024];

	char tmp_buffer[256];
	int rslt = 0;
	unsigned int len = 0;
	int h_errnop = 0;


	memset(tmp_buffer,0x00,256);
	memset(hostent_buff,0x00,1024);


	hostinfo_p = &hostinfo;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		return -1;
	}

	addr.sin_family = AF_INET;

	strncpy(tmp_buffer,hostname,1);
	if(isdigit(tmp_buffer[0]))
	{
		addr.sin_addr.s_addr = inet_addr(hostname);
	}
	else
	{
		rslt = gethostbyname_r(hostname, &ret, hostent_buff, 1024,&hostinfo_p,&h_errnop);
		if((rslt != 0) || (hostinfo_p == NULL))
		{
			close(sockfd);

			return -1;
		}
		addr.sin_addr = *(struct in_addr *)*hostinfo_p->h_addr_list;
	}
	addr.sin_port = htons(portnum);
	len = sizeof(addr);

	rslt = connect_nonblock(ctx,&sockfd,(struct sockaddr *)&addr,len);
	if(rslt == -1)
	{
		close(sockfd);

		return -1;
	}

	snprintf(tmp_buffer,255,"GET /%s%s HTTP/1.1\r\n",file,gwc_ping_v2);
	write_line_to_socket(sockfd,tmp_buffer);

	snprintf(tmp_buffer,255,"Host: %s\r\n\r\n",hostname);
	write_line_to_socket(sockfd,tmp_buffer);

	/* read the response */
	rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
	if(rslt < 2)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		return -1;
	}

	/*  What did the socket report back?  Was it code 200? */
	if(strstr(tmp_buffer,"200 OK") == NULL)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}

	do
	{
		rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
		if(rslt < 2)
		{
			shutdown(sockfd,SHUT_RDWR);
			close(sockfd);
			return -1;
		}
	} while((tmp_buffer[0] != '\r') && (tmp_buffer[0] != '\n'));

	memset(tmp_buffer,0x00,256);
	rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
	if(rslt < 2)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		return -1;
	}
	if( (strstr(tmp_buffer,"PONG")== NULL) && (strstr(tmp_buffer,"pong")== NULL) )
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return	0;
	}

	shutdown(sockfd,SHUT_RDWR);
	close(sockfd);

	return 1;
}


/* System Related Functions */
int write_pid_file(char *file_loc)
{
	FILE *pid_file = NULL;
	pid_t	pid = -1;


	pid = getpid();
	if(NULL == (pid_file = fopen(file_loc,"w")))
	{
		fprintf(stderr,"Error: Failed to open pid file: %s\n", file_loc);

		return -1;
	}
	fprintf(pid_file,"%d",pid);
	fclose(pid_file);

	return 0;
}


int delete_pid_file(char *file_loc)
{
	int rslt = 0;


	rslt = remove(file_loc);
	if(rslt == -1)
	{
		fprintf(stderr,"Failed to remove pid file: %s\n", file_loc);
		fprintf(stderr,"Please, remove the file manually.\n");

		return -1;
	}

	return 0;
}


int file_exists(char *path)
{
	int rslt = 0;
	struct stat tmp_stats;


	rslt = lstat(path, &tmp_stats);
	if(rslt != -1)
	{
		return 1;
	}

	return 0;
}


int write_file(struct cpcd_config *config, struct hosts *cached_hosts, char *network)
{
	FILE *data = NULL;
	char file_path[1024];
	char tmp_file_path[1024];
	struct hosts *walk_cached_hosts = NULL;
	int rslt = 0;
	struct stat tmp_stats;


	if(cached_hosts == NULL)
	{
		return -1;
	}

	walk_cached_hosts = get_first_host(cached_hosts);
	if(walk_cached_hosts == NULL)
	{
		return -1;
	}

	memset(file_path,0x00,1024);
	memset(tmp_file_path,0x00,1024);

	snprintf(file_path,1018,"%s/%s.dat",config->data_dir,network);
	snprintf(tmp_file_path,1023,"%s/%s.dat.tmp",config->data_dir,network);

	rslt = lstat(tmp_file_path, &tmp_stats);
	if(rslt != -1)
	{
		if(S_ISREG(tmp_stats.st_mode) != 0)
		{
			rslt = remove(tmp_file_path);
			if(rslt != -1)
			{
				fprintf(stderr,"Failed to remove stale tmp file.  Please, remove it manually.  File: %s\n",tmp_file_path);

				return -1;
			}
		}
		else
		{
			fprintf(stderr,"Tmp file appears to exist.  Please, remove it.\n");

			return -1;
		}
	}

	if(NULL == (data = fopen(tmp_file_path,"w")))
	{
		fprintf(stderr,"Failed to open tmp data file for writing.\n");

		return -1;
	}

	while(walk_cached_hosts != NULL)
	{
		if(walk_cached_hosts->addr != NULL)
		{
			rslt = fprintf(data,"%c|%s|%ln|\n",walk_cached_hosts->type,walk_cached_hosts->addr,&(walk_cached_hosts->bdate));
			if(rslt < 1)
			{
				fprintf(stderr,"Error writing file. Disk may be full.\n");
				remove(tmp_file_path);

				return -1;
			}
		}

		walk_cached_hosts = walk_cached_hosts->next;
	}
	fclose(data);

	rslt = rename(tmp_file_path,file_path);
	if(rslt == -1)
	{
		fprintf(stderr,"Failed to overwrite data file (%s) with tmp file (%s)\n",
			file_path,tmp_file_path);
		fprintf(stderr,"Disk may be full.\n");

		return -1;
	}


	return 0;
}


int load_network_caches(struct cpcd_ctx **ctx, char *networks)
{
	char *net = NULL;
	char *p = NULL;
	int rslt = 0;
	char nets[512];


	memset(nets,0x00,512);
	strncpy(nets,networks,511);

	/* Data Structure Population from cache files on disk */
	net = (char *)strtok_r(nets," ",&p);
	do
	{
		rslt = load_network_cache(ctx,net);
	} while((net = ((char *)strtok_r('\0'," ",&p))));

	rslt = read_file((*ctx)->config,&((*ctx)->bad_caches),"bad_caches");
	if(rslt == -1)
	{
		fprintf(stderr,"System error reading bad cache file.\n");
	}

	return 0;
}


int load_network_cache(struct cpcd_ctx **ctx, char *net)
{
	int rslt = 0;
	struct networks *new_net = NULL;


	if(net == NULL)
	{
		return -1;
	}

	new_net = (struct networks *)malloc(sizeof(struct networks));
	if(new_net == NULL)
	{
		fprintf(stderr,"Failed to allocate memory for new network.\n");

		return -1;
	}
	strncpy(new_net->name,net,255);
	new_net->hosts = NULL;
	new_net->bad_hosts = NULL;

	rslt = read_file((*ctx)->config,&(new_net->hosts),net);
	if(rslt == -1)
	{
		fprintf(stderr,"System error reading file for network %s.\n",net);
	}

	(*ctx)->networks = add_new_network((*ctx)->networks,new_net);

	return 0;
}


int read_file(struct cpcd_config *config, struct hosts **cached_hosts, char *network)
{
	FILE *fp;
	char line[256];
	char file_path[256];
	char *lineptr = NULL;

	char *type_str = NULL;
	char *addr = NULL;
	char *bdate_str = NULL;

	char *p = NULL;


	memset(line,0x00,256);
	memset(file_path,0x00,256);

	snprintf(file_path,255,"%s/%s.dat",config->data_dir,network);

	/* Create an empty file if it doesn't exist */
	if(file_exists(file_path) == 0)
	{
		fp = fopen(file_path,"w");
		if(fp == NULL)
		{
			fprintf(stderr,"Failed to open data file: '%s'\n",file_path);
			perror("File error");

			return -1;
		}
		fclose(fp);
	}

	fp = fopen(file_path,"r");
	if(fp == NULL)
	{
		fprintf(stderr,"Failed to open data file: '%s'\n",file_path);
		perror("File error");

		return -1;
	}

	while(fgets(line,sizeof(line),fp) != NULL)
	{
		lineptr = &line[0];
		type_str = (char *)strtok_r(lineptr,"|",&p);
		if(type_str == NULL)
			continue;
		addr = (char *)strtok_r(NULL,"|",&p);
		if(addr == NULL)
			continue;
		bdate_str = (char *)strtok_r(NULL,"|",&p);
		if(bdate_str == NULL)
			continue;

		if((type_str != NULL) && (addr != NULL) && (bdate_str != NULL))
		{
			if((*cached_hosts = add_new_cached_host(*cached_hosts,type_str,addr,atol(bdate_str))) == NULL)
			{
				fprintf(stderr,"failed to add cached host.\n");

				return -1;
			}

		}
	}
	fclose(fp);


	return 0;
}


int connect_nonblock(struct cpcd_ctx *ctx, int *skt, struct sockaddr *addr,unsigned int len)
{
	int i = 0;
	int flags = 0;
	int rslt = 0;


	/* Set non-blocking mode on the socket */
	flags = fcntl(*skt, F_GETFL,0);
	fcntl(*skt,F_SETFL,flags|O_NONBLOCK);

	for(i=0;i<10;i++)
	{
		if(ctx->exit_flag == 1)
		{
			return -1;
		}
		else
		{
			rslt = connect(*skt,addr,len);
			if(rslt == 0)
			{
				/* have a good connection return */
				fcntl(*skt,F_SETFL,flags);
				return 0;
			}
			else if(rslt == -1)
			{
				if(errno != EINPROGRESS)
				{
					/* no connection yet, check errno */
					return -1;
				}
			}
			/* Wait for tcp/ip handshake to cmplete */
			sleep(1);
		}
	}

	return -1;
}


int read_line(int skt, unsigned char *buffer, unsigned int read_len)
{
        unsigned char c = 0x00;
	unsigned char *ptr = NULL;
        unsigned int n = 0;
	unsigned int rc = 0;

	struct pollfd client[2];
	int rslt = 0;


	if(skt == -1)
		return -1;

	if(buffer == NULL)
	{
		fprintf(stderr,"Error: NULL buffer passed to read_line.\n");

		return -1;
	}

	ptr = buffer;

	client[0].fd = skt;
	client[1].fd = -1;
	client[0].events = POLLIN;
	rslt = poll(client,1,3000);
	if(rslt == -1)
	{
		return -1;
	}
	else if(rslt == 0)
	{
		return -1;
	}
	else
	{
		for(n=1; n < read_len; n++)
		{
			if((rc = read(skt, &c,1)) == 1)
			{
				*ptr++ = c;
				if(c == '\n')
				{
					break;
				}
			}
			else if(rc == 0)
			{
				if(n == 1)
				{
					return 0;
				}
				else
				{
					break;
				}
			}
			else
			{
				return -1;
			}
		}
		*ptr = 0;
	}


        return n;
}


int write_line_to_socket(int skt, char *the_line)
{
	int status = 0;
        int result = 0;
	unsigned int count = 0;

	int rslt = 0;
	struct pollfd client[2];


	if(skt == -1)
		return -1;
	if(the_line == NULL)
	{
		fprintf(stderr,"Error: NULL buffer passed to write_line_to_socket.\n");

		return 0;
	}
        count = strlen(the_line);

	client[0].fd = skt;
	client[1].fd = -1;
	client[0].events = POLLOUT;
	rslt = poll(client,1,3001);

	if(rslt == -1)
	{
		return -1;
	}
	else if(rslt == 0)
	{
		return -1;
	}
	else
	{
		while(status != count)
		{
			result = write(skt, the_line + status, count - status);
			if (result < 0) return result;
			status += result;
		}
	}

        return(status);
}


int log_access(FILE *log, char *ip, unsigned int code, char *request,
			unsigned long bytes, char *client)
{
        char ncsa_date[256];
	char buffer[2048];
        time_t lt;
        struct tm *ptr = NULL;


	memset(ncsa_date,0x00,256);
	memset(buffer,0x00,2048);

        lt = time(NULL);
        ptr = localtime(&lt);

        strftime(&ncsa_date[0],255,"[%d/%b/%Y:%H:%M:%S %z]",ptr);
	snprintf(buffer,2047,"%s - - %s \"%s\" %d %ln \"\" \"%s\" \"\"\n",ip,ncsa_date,request,code,&bytes,client);
	pthread_mutex_lock(&log_mutex);
	fputs(buffer,log);
	pthread_mutex_unlock(&log_mutex);

	fflush(log);

	return 0;
}


int str_to_lower(char **string)
{
	unsigned int i = 0;


	if((*string) == NULL)
	{
		fprintf(stderr,"strtoupper was passed a null arg.\n");

		return -1;
	}
	while((*string)[i])
	{
		(*string)[i] = tolower((*string)[i]);
		i++;
	}

	return 0;
}


/* intput age in seconds, returns day/hr/min string */
int get_age_r(long *age, char **str_age, unsigned int buff_size)
{
	struct tm *bdate = NULL;


	if((*str_age == NULL) || (buff_size < 30) )
		return -1;
	bdate = (struct tm *)malloc(sizeof(struct tm));
	if(bdate == NULL)
	{
		fprintf(stderr, "Out of memory. Malloc call failed.\n");

		return -1;
	}
	if(NULL == localtime_r(age, bdate))
	{
		fprintf(stderr, "Call to localtime_r failed.\n");
		if(bdate != NULL)
			free(bdate);

		return -1;
	}

	snprintf(*str_age,buff_size-1,"%d.%02d.%02d.%02d.%02d.%02d",(bdate->tm_year +
			1900),++(bdate->tm_mon),bdate->tm_mday,bdate->tm_hour,bdate->tm_min,bdate->tm_sec);

	if(bdate != NULL)
		free(bdate);

	return 0;
}
