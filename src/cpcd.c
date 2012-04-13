/* cpcd.c - Main Source File for cpcd
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

#include "cpcd.h"
#include <mcheck.h>

struct cpcd_ctx *ctx = NULL;
int		listenfd = 0;


int main(int argc, char *argv[])
{

	struct sockaddr_in	addr_srvr;

	struct linger		so_linger;

	char	*srvr_addr = NULL;
	char	*srvr_port = NULL;

	int z = 0; /* A result/return code */

	unsigned char buff[128];

	int	arg	= 0;

	int	i	= 0;
	int	on	= 1;

	struct	sockaddr	*cliaddr = NULL;
	pthread_t		url_verify_thread;
	pthread_t		file_writer_thread;
	pthread_t	*tptr = NULL;
	pthread_t	*vtptr = NULL;


	if(geteuid() == 0)
	{
		fprintf(stderr, "CPCD: This program may not be run as root.\n");
		fprintf(stderr, "CPCD: Please, drop your privlidges and re-launch.\n\n");

		return -1;
	}

	memset(buff,0x00,128);

	ctx = (struct cpcd_ctx *)malloc(sizeof(struct cpcd_ctx));
	if(ctx == NULL)
	{
		fprintf(stderr,"CPCD: Failed to malloc. System out of memory.\n");

		return -1;
	}
	init_context(&ctx);
	ctx->config = (struct cpcd_config *)malloc(sizeof(struct cpcd_config));
	if(ctx->config == NULL)
	{
		fprintf(stderr,"CPCD: Failed to malloc. System out of memory.\n");	
		free_context(ctx);

		return -1;
	}
	init_config(&ctx, &(ctx->config));
	if(argc > 0)
	{
		for(arg=1;arg<argc;arg++)
		{
			if(argv[arg][0] == '-')
			{
				if(argv[arg][1] == '-')
				{
					if(strstr(argv[arg],"help") != NULL)
					{
						printf("Usage: cpcd\n");
						printf("	-v Verbose Mode\n");
						printf("	-h This Help Text\n");
						printf("	--help This Help Text\n");
						printf("	--version Display Version Information\n");
						printf("\n");
						free_context(ctx);

						return 0;
					}
					else if(strstr(argv[arg],"version") != NULL)
					{
						printf("%s\n",ctx->config->cpcd_version);
						free_context(ctx);

						return 0;
					}
				}
				else if(argv[arg][1] == 'v')
				{
					ctx->verbose = 1;
				}
				else
				{
					printf("Usage: cpcd\n");
					printf("	-v Verbose Mode\n");
					printf("	-h This Help Text\n");
					printf("	--help This Help Text\n");
					printf("	--version Display Version Information\n");
					printf("\n");
					free_context(ctx);

					return 0;
				}
			}
		}
	}
	write_pid_file(ctx->config->pid_file);

	if(ctx->config->cpcd_keep_log == 1)
	{
		if(NULL == (ctx->log = fopen(ctx->config->log_file,"a")))
		{
			fprintf(stderr,"Failed to open log file.  Not logging.\n\n");
		}
	}

	signal(SIGINT,inthandler);
	signal(SIGTERM,inthandler);
	signal(SIGPIPE,SIG_IGN);

	srvr_addr = (char *)ctx->config->bind_ip;
	srvr_port = (char *)ctx->config->web_bind_port;

	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1)
	{
		fprintf(stderr,"Socket call failed.\n");
		delete_pid_file(ctx->config->pid_file);
		free_context(ctx);

		return -1;
	}

	so_linger.l_onoff = TRUE;
	so_linger.l_linger = 3;
	setsockopt(listenfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
        setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	memset(&addr_srvr,0,sizeof(addr_srvr));
	addr_srvr.sin_family  = AF_INET;
	addr_srvr.sin_port = htons(atoi(srvr_port));
	addr_srvr.sin_addr.s_addr = inet_addr(srvr_addr);

	z = bind(listenfd,(struct sockaddr *)&addr_srvr,sizeof(addr_srvr));
	if(z == -1)
	{
		fprintf(stderr,"bind call failed on z\n");
		delete_pid_file(ctx->config->pid_file);
		free_context(ctx);

		return -1;
	}

	if(listen(listenfd,4) == -1)
	{
		fprintf(stderr,"Failed on listen\n");
		delete_pid_file(ctx->config->pid_file);
		free_context(ctx);

		return -1;
	}

	/* End of Data Structure Population */
	if(ctx->verbose == 1)
		printf("Creating thread pools...\n");
	tptr = calloc(ctx->config->listen_threads,sizeof(pthread_t));
	for(i=0;i<ctx->config->listen_threads;i++)
	{
		pthread_create(&tptr[i], NULL, thread_main, (void *)NULL);
	}
	vtptr = calloc(ctx->config->host_threads,sizeof(pthread_t));
	for(i=0;i<ctx->config->host_threads;i++)
	{
		pthread_create(&vtptr[i], NULL, ip_verify_thread_main, (void *)NULL);
	}
	pthread_create(&url_verify_thread, NULL, url_verify_thread_main, (void *)NULL);
	pthread_create(&file_writer_thread, NULL, file_writer_thread_main, (void *)NULL);

	while(ctx->exit_flag != 1)
	{
		pause();
	}

	if(ctx->verbose == 1)
	{
		printf("Exiting...\n");
		printf("\tShutting Down sockets...\n");
	}
	shutdown(listenfd,SHUT_RDWR);
	close(listenfd);

	if(cliaddr != NULL)
		free(cliaddr);

	if(ctx->verbose == 1)
	{
		printf("\tCanceling threads...\n");
	}

	if(tptr != NULL)
	{
		for(i=0;i<ctx->config->listen_threads;i++)
		{
			if(&tptr[i] != NULL)
			{
				pthread_cancel(tptr[i]);
				pthread_join(tptr[i], NULL);
			}
		}
		free(tptr);
	}
	if(vtptr != NULL)
	{
		for(i=0;i<ctx->config->host_threads;i++)
		{
			if(&vtptr[i] != NULL)
			{
				pthread_cancel(vtptr[i]);
				pthread_join(vtptr[i], NULL);
			}
		}
		free(vtptr);
	}
	pthread_join(url_verify_thread, NULL);
	pthread_join(file_writer_thread, NULL);

	/* Update Disk Cache (Files) */
	if(ctx->verbose == 1)
	{
		printf("\tWriting Files...\n");
	}
	write_files();

	/* Free all memory and exit. */
	if(ctx->verbose == 1)
		printf("\tFreeing Memory...\n");

	free_networks(ctx->networks);
	ctx->networks = NULL;

	if(ctx->bad_caches != NULL)
	{
		free_cached_hosts(ctx->bad_caches);
		ctx->bad_caches = NULL;
	}

	if(ctx->verbose == 1)
		printf("\tClosing Log...\n");
	if(ctx->config->cpcd_keep_log == 1)
	{
		if(ctx->log != NULL)
		{
			fclose(ctx->log);
		}
	}

	if(ctx->verbose == 1)
		printf("\tDestroying Context...\n");
	delete_pid_file(ctx->config->pid_file);
	if(ctx->verbose == 1)
		printf("Done.\n");
	free_context(ctx);

	return 0;
}


void	inthandler(int i)
{
	ctx->exit_flag = 1;

	return;
}


void	*thread_main(void *arg)
{
	int			c = 0;
	socklen_t		inet_length;
	struct	sockaddr	cliaddr;
	
	
	inet_length = sizeof(struct sockaddr_in);
	while(ctx->exit_flag != 1)
	{
		
		c = accept(listenfd,(struct sockaddr *)&cliaddr,&inet_length);
		if(c == -1)
		{
			continue;
		}

		ctx->total_hits++;
		if(ctx->verbose == 1)
		{
			if((ctx->total_hits % 100) == 0)
			{
				printf("Total Hits: %ln\n",&(ctx->total_hits));
			}
		}

		accept_connect(c,ctx->config);

		shutdown(c,SHUT_RDWR);
		close(c);
	}

	return NULL;
}


void *file_writer_thread_main(void *nothing)
{
	unsigned int i = 0;


	while(ctx->exit_flag != 1)
	{
		for(i=0;i<100;i++)
		{
			if(ctx->exit_flag == 1)
			{
				break;
			}
			else
			{
				sleep(3);
			}
		}
		write_files();
	}

	return NULL;
}


int write_files(void)
{
	struct networks *walk_net = NULL;


	pthread_mutex_lock(&data_mutex);
	walk_net = get_first_network(ctx->networks);
	while(walk_net != NULL)
	{
		write_file(ctx->config,walk_net->hosts,walk_net->name);

		walk_net = walk_net->next;
	}
	write_file(ctx->config,ctx->bad_caches,"bad_caches");
	pthread_mutex_unlock(&data_mutex);

	return 0;
}


void *ip_verify_thread_main(void *nothing)
{
	struct queue *work_item = NULL;


	while(ctx->exit_flag != 1)
	{
		pthread_mutex_lock(&ip_queue_mutex);
		work_item = (struct queue *)pop_entry(&(ctx->host_queue));
		pthread_mutex_unlock(&ip_queue_mutex);
		if(work_item == NULL)
		{
			sleep(1);
		}
		else
		{
			process_node(work_item);
			free_queue_item(work_item);
		}
	}

	return NULL;
}


int process_node(struct queue *the_item)
{
	struct hosts *oldest_host = NULL;
	struct networks *current_net = NULL;
	int host_count = 0;
	unsigned long bdate = 0;

	time_t time_of_discovery = 0;


	time_of_discovery = time(NULL);
	
	current_net = get_network_by_name(ctx->networks,the_item->net_name);
	if(current_net == NULL)
	{
		fprintf(stderr,"get_network_by_name() returned NULL\n");

		return -1;
	}

	if(verify_node(the_item->data,the_item->net_name,ctx->config->port_restriction) != -1)
	{
		time_of_discovery = time(NULL);

		pthread_mutex_lock(&data_mutex);

		/* make sure another thread hasn't added it */
		bdate = node_already_exists(current_net->hosts,the_item->data);
		if(bdate == 0)
		{
			host_count = count_nodes(current_net->hosts,D_HOSTS);
			if(ctx->config->max_hosts < host_count)
			{
				current_net->hosts = trim_struct_to_max(current_net->hosts,ctx->config->max_hosts,D_HOSTS);
			}
			if(ctx->config->max_hosts == host_count)
			{
				oldest_host = get_oldest_host(current_net->hosts,D_HOSTS);
				current_net->hosts = delete_cached_host(oldest_host);
			}
			current_net->hosts = add_new_cached_host(current_net->hosts,
						"H",the_item->data,time_of_discovery);
		}
		pthread_mutex_unlock(&data_mutex);
	}

	return 0;
}


void *url_verify_thread_main(void *nothing)
{
	struct queue *work_item = NULL;


	while(ctx->exit_flag != 1)
	{
		pthread_mutex_lock(&url_queue_mutex);
		work_item = (struct queue *)pop_entry(&(ctx->url_queue));
		pthread_mutex_unlock(&url_queue_mutex);
		if(work_item == NULL)
		{
			sleep(1);
		}
		else
		{
			process_url(work_item);
			free_queue_item(work_item);
		}
	}

	return NULL;
}


int process_url(struct queue *the_item)
{
	struct hosts *oldest_url = NULL;
	struct networks *current_network = NULL;

	time_t time_of_discovery;
	int url_count = 0;
	int rslt = 0;
	size_t url_len = 0;
	unsigned long bdate = 0;


	url_len = strlen(the_item->data);
	if(url_len < 12)
		return 0;

	rslt = verify_url(ctx,the_item->data);
	if(rslt == -1)
	{
		return 0;
	}

	time_of_discovery = time(NULL);
	pthread_mutex_lock(&data_mutex);
	if((rslt == -1) || (rslt == 0) )
	{
		bdate = node_already_exists(ctx->bad_caches,the_item->data);
		if(bdate == 0)
		{
			url_count = count_nodes(ctx->bad_caches,D_URLS);
			if(ctx->config->max_bad_urls < url_count)
			{
				ctx->bad_caches = trim_struct_to_max(ctx->bad_caches, ctx->config->max_bad_urls,D_URLS);
			}
			if(ctx->config->max_bad_urls == url_count)
			{
				oldest_url = get_oldest_host(ctx->bad_caches,D_URLS);
				ctx->bad_caches = delete_cached_host(oldest_url);
			}
			ctx->bad_caches = add_new_cached_host(ctx->bad_caches,"U",the_item->data,time_of_discovery);
		}
	}
	if(rslt == 1)
	{
		current_network = get_network_by_name(ctx->networks,"gnutella");
		if(current_network == NULL)
		{
			fprintf(stderr,"get_network_by_name() returned NULL\n");
		}
		bdate = node_already_exists(current_network->hosts,the_item->data);
		if(bdate == 0)
		{
			url_count = count_nodes(current_network->hosts,D_URLS);
			if(ctx->config->max_urls < url_count)
			{
				current_network->hosts = trim_struct_to_max(current_network->hosts,ctx->config->max_urls,D_URLS);
			}
			if(ctx->config->max_urls == url_count)
			{
				oldest_url = get_oldest_host(current_network->hosts,D_URLS);
				current_network->hosts = delete_cached_host(oldest_url);
			}
			current_network->hosts = add_new_cached_host(current_network->hosts,"U",the_item->data,time_of_discovery);
		}
	}
	else if(rslt == 2)
	{
		current_network = get_network_by_name(ctx->networks,"gnutella2");
		if(current_network == NULL)
		{
			fprintf(stderr,"get_network_by_name() returned NULL\n");
		}
		bdate = node_already_exists(current_network->hosts,the_item->data);
		if(bdate == 0)
		{
			url_count = count_nodes(current_network->hosts,D_URLS);
			if(ctx->config->max_urls < url_count)
			{
				current_network->hosts = trim_struct_to_max(current_network->hosts,ctx->config->max_urls,D_URLS);
			}
			if(ctx->config->max_urls == url_count)
			{
				oldest_url = get_oldest_host(current_network->hosts,D_URLS);
				current_network->hosts = delete_cached_host(oldest_url);
			}
			current_network->hosts = add_new_cached_host(current_network->hosts,"U",the_item->data,time_of_discovery);
		}
	}
	else if(rslt == 3)
	{
		/* gnutella */
		current_network = get_network_by_name(ctx->networks,"gnutella");
		if(current_network == NULL)
		{
			fprintf(stderr,"get_network_by_name() returned NULL\n");
		}
		bdate = node_already_exists(current_network->hosts,the_item->data);
		if(bdate == 0)
		{
			url_count = count_nodes(current_network->hosts,D_URLS);
			if(ctx->config->max_urls < url_count)
			{
				current_network->hosts = trim_struct_to_max(current_network->hosts,ctx->config->max_urls,D_URLS);
			}
			if(ctx->config->max_urls == url_count)
			{
				oldest_url = get_oldest_host(current_network->hosts,D_URLS);
				current_network->hosts = delete_cached_host(oldest_url);
			}
			current_network->hosts = add_new_cached_host(current_network->hosts,"U",the_item->data,time_of_discovery);
		}
		/* gnutella2 */
		current_network = get_network_by_name(ctx->networks,"gnutella2");
		if(current_network == NULL)
		{
			fprintf(stderr,"get_network_by_name() returned NULL\n");
		}
		bdate = node_already_exists(current_network->hosts,the_item->data);
		if(bdate == 0)
		{
			url_count = count_nodes(current_network->hosts,D_URLS);
			if(ctx->config->max_urls < url_count)
			{
				current_network->hosts = trim_struct_to_max(current_network->hosts,ctx->config->max_urls,D_URLS);
			}
			if(ctx->config->max_urls == url_count)
			{
				oldest_url = get_oldest_host(current_network->hosts,D_URLS);
				current_network->hosts = delete_cached_host(oldest_url);
			}
			current_network->hosts = add_new_cached_host(current_network->hosts,"U",the_item->data,time_of_discovery);
		}
	}
	pthread_mutex_unlock(&data_mutex);

	return 0;
}


int accept_connect(int c, struct cpcd_config *config)
{
	struct name_value_pair_dllst *form = NULL;

	/* CGI Variables */
	char remote_address[256];

	/* Form Variables */
	char *client = NULL;
	char *version = NULL;
	char *hostfile = NULL;
	char *urlfile = NULL;
	char *net = NULL;
	char *get = NULL;
	char *ip = NULL;
	char *url = NULL;
	char *update = NULL;
	char *ping = NULL;
	char *cluster = NULL;
	char *stats = NULL;

	/* flags */
	int get_alt_nets = 0;
	int guess_version = 0;

	unsigned long z = 0;

	char bufftrash[256];
	char buff[256];
	char buff2[256];
	char *buff_ptr = NULL;

	/* logging vars */
	char log_client[256];
	char log_request[256];
	char *log_client_p = NULL;
	char *log_request_p = NULL;
	unsigned long total_bytes = 0;
	unsigned int code = 200;

	int rslt = 0;
	long lrslt = 0;

	struct sockaddr_in peer;
	int namelen = 0;

	struct networks *current_network = NULL;


	memset(buff,0x00,256);

	/* Read the data from the socket */
	z = read_line(c,buff,255);
	if(z < 2)
	{
		return -1;
	}
	else if(z == -1)
	{
		return -1;
	}

	memset(buff2,0x00,256);
	memset(bufftrash,0x00,256);
	memset(log_request,0x00,256);
	memset(log_client,0x00,256);
	memset(remote_address,0x00,256);

	/* This truncation is delebrate to trim the '/n' */
	buff[strlen(buff)-2] = '\0';
	strncpy(log_request,buff,255);
	log_request_p = &log_request[0];
	if( (memcmp(log_request,"GET /pwc.cgi ",13) != 0)
	  && (memcmp(log_request,"GET /pwc.cgi?",13) != 0) )
	{
		code = 404;
	}
	else
	{
		code = 200;
	}

	if(strncasecmp("GET",buff,3) == 0)
	{
		/* Read the remaining headers until */
		do
		{
			rslt = read_line(c,bufftrash,255);
		}
		while( (! ( ((bufftrash[0] == '\n') || (bufftrash[0] == '\r')) &&
				strlen(bufftrash) < 3)) && (rslt > 2));
	}

	if(hex_to_ascii(buff) == -1)
	{
		fprintf(stderr,"hex_to_ascii failure\n");

		return -1;
	}
	if(memchr(buff,'|',255) != NULL)
	{
		rslt = write_line_to_socket(c,"Meta characters are not allowed in requests.\n");
		if(rslt != -1)
			total_bytes += rslt;

		return 0;
	}
	buff_ptr = memchr(buff,'?',255);
	if(buff_ptr == NULL)
	{
		rslt = html_page(c,ctx);
		if(rslt != -1)
			total_bytes += rslt;

		return 0;
	}
	buff_ptr++;
	if(buff_ptr == NULL)
	{
		rslt = html_page(c,ctx);
		if(rslt != -1)
			total_bytes += rslt;

		return 0;
	}
	strncpy(buff2,buff_ptr,strlen(buff_ptr)+1);
	buff_ptr = memchr(buff2,' ',255);
        if(buff_ptr == NULL)
        {
                rslt = html_page(c,ctx);
		if(rslt != -1)
			total_bytes += rslt;

                return 0;
        }
        buff_ptr[0] = '\0';
	form = parse_name_value_pairs_r(buff2);
	if(form == NULL)
	{
		/* No Request, so Echo Default Page */
		rslt = html_page(c,ctx);
		if(rslt != -1)
			total_bytes += rslt;

		return 0;
	}

	stats = get_value(form,"stats");
	if(stats != NULL)
	{
		rslt = stats_page(c,ctx);
		if(rslt != -1)
			total_bytes += rslt;

		if(form != NULL)
		{
			free_name_value_pair_dllst(form);
		}

		return 0;
	}
	else
	{
		client = get_value(form,"client");
		version =  get_value(form,"version");
		hostfile = get_value(form,"hostfile");
		urlfile = get_value(form,"urlfile");
		net = get_value(form,"net");
		get = get_value(form,"get");
		ip = get_value(form,"ip");
		url = get_value(form,"url");
		update = get_value(form,"update");
		ping = get_value(form,"ping");
		cluster = get_value(form,"cluster");

		/* Begin Response */

		/* print header info */
		snprintf(buff,255,"HTTP/1.1 %d OK\r\n",code);
		rslt = write_line_to_socket(c,buff);
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}
		rslt = write_line_to_socket(c,"Content-type: text/plain\r\n");
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}
		rslt = write_line_to_socket(c,"Pragma: no-cache\r\n");
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}
		rslt = write_line_to_socket(c,"Cache-Control: no-store, no-cache, must-revalidate\r\n");
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}
		rslt = write_line_to_socket(c,"Cache-Control: post-check=0, pre-check=0\r\n");
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}
		namelen = sizeof(peer);
		rslt = getpeername(c,(struct sockaddr *)&peer,&namelen);
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		snprintf(remote_address,255,"%s",inet_ntoa(peer.sin_addr));
		snprintf(buff,255,"X-Remote-IP: %s\r\n\r\n", remote_address);
		rslt = write_line_to_socket(c,buff);
		if(rslt == -1)
		{
			if(form != NULL)
			{
				free_name_value_pair_dllst(form);
			}

			return -1;
		}
		else
		{
			total_bytes += rslt;
		}

		if(code == 404)
		{
			rslt = write_line_to_socket(c,"404: File not found.\n");
			if(rslt != -1)
			{
				total_bytes += rslt;
			}
		}
		else
		{
			if(get == NULL)
			{
				/* We're working with Version 1 */
				guess_version = 1;
			}
			else
			{
				/* We're working with Version 2 */
				guess_version = 2;
			}
			/* Handle a ping request  */
			/* Assume new ping format */
			if(ping != NULL)
			{
				if(guess_version == 1)
				{
					snprintf(buff,255,"PONG %s\n", ctx->config->cpcd_version);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{
						if(form != NULL)
						{
							free_name_value_pair_dllst(form);
						}

						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
				}
				else
				{
					snprintf(buff,255,"I|pong|%s\n", ctx->config->cpcd_version);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{
						if(form != NULL)
						{
							free_name_value_pair_dllst(form);
						}

						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
				}

				if(form != NULL)
				{
					free_name_value_pair_dllst(form);
				}

				return 0;
			}

			if(client == NULL)
			{
				rslt = write_line_to_socket(c,"Must send correctly formatted client version information.\n");
				if(rslt != -1)
				{
					total_bytes += rslt;
				}
				if(form != NULL)
				{
					free_name_value_pair_dllst(form);
				}

				return 0;
			}
			else
			{
				if(strstr(ctx->config->default_allow,client) == NULL)
				{
					rslt = write_line_to_socket(c,"I|client-not-supported\n");
					if(rslt != -1)
					{
						total_bytes += rslt;
					}
					if(form != NULL)
					{
						free_name_value_pair_dllst(form);
					}

					return 0;
				}
			}

			/* Multi Network Support */
			if(net != NULL)
			{
				if(strstr(ctx->config->allowed_networks, net) == NULL)
				{
					get_alt_nets=1;
					rslt = write_line_to_socket(c,"I|net-not-supported\n");
					if(rslt != -1)
					{
						total_bytes += rslt;
					}

					if(form != NULL)
					{
						free_name_value_pair_dllst(form);
					}

					return 0;
				}
			}
			else
			{
				net = ctx->config->default_network;
			}
			current_network = get_network_by_name(ctx->networks,net);

			/* Get Request */
			if(get != NULL)
			{
				ctx->reqs++;
				/* Default Net */
				lrslt = print_cached_hosts(c,current_network->hosts,D_ALL_DATA,guess_version,net);
				if(lrslt != -1)
					total_bytes += lrslt;
			}
			else
			{
				ctx->reqs++;
				if(urlfile != NULL)
				{
					lrslt = print_cached_hosts(c,current_network->hosts,D_URLS,guess_version,net);
					if(lrslt != -1)
						total_bytes += lrslt;
				}
				if(hostfile != NULL)
				{
					lrslt = print_cached_hosts(c,current_network->hosts,D_HOSTS,guess_version,net);
					if(lrslt != -1)
						total_bytes += lrslt;
				}
			}

			/* Handle and update request */
			if( (ip != NULL) || (url != NULL) )
			{
				ctx->updts++;
				process_update(ip, url, current_network);

				if(guess_version == 1)
				{
					/* Give old-skool requess to old-skool updates */
					rslt = write_line_to_socket(c,"OK\n");
					if(rslt == -1)
					{
						if(form != NULL)
						{
							free_name_value_pair_dllst(form);
						}

						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
					snprintf(buff,255,"%s %s\n",ip,url);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
						total_bytes += rslt;
				}
				else
				{
					rslt = write_line_to_socket(c,"I|update|OK\n");
					if(rslt == -1)
						total_bytes += rslt;
				}
			}
		}
	}

	/* Log request if user requested */
	if(ctx->config->cpcd_keep_log == 1)
	{
		if(ctx->log != NULL)
		{
			snprintf(log_client,255,"%s %s",client,version);
			log_client_p = &log_client[0];
			log_access(ctx->log, remote_address, total_bytes, log_request_p, code, log_client_p);
		}
	}

	/* Clean up Memory and Exit */
	if(form != NULL)
	{
		free_name_value_pair_dllst(form);
	}

	return 0;
}


long print_cached_hosts(int c,struct hosts *cached_hosts,int mode,int version,char *current_net)
{
	struct hosts *walk_cached_hosts = NULL;
	time_t current_time;
	unsigned long secs_old = 0;
	char buff[256];
	int rslt = 0;
	unsigned long total_bytes = 0;


	if(cached_hosts == NULL)
		return 0;

	memset(buff,0x00,256);

	current_time = time(NULL);

	walk_cached_hosts = get_first_host(cached_hosts);
	if(walk_cached_hosts == NULL)
	{
		fprintf(stderr,"Error Cached Hosts was NULL\n");
	}

	while(walk_cached_hosts != NULL)
	{
		secs_old = (unsigned long)current_time - walk_cached_hosts->bdate;
		if( (mode == D_ALL_DATA) && (version == 2) )
		{
			snprintf(buff,255,"%c|%s|%ln|\n",walk_cached_hosts->type,walk_cached_hosts->addr,&secs_old);
			rslt = write_line_to_socket(c,buff);
			if(rslt == (-1))
			{
				return -1;
			}
			else
			{
				total_bytes += rslt;
			}
		}
		else
		{
			if( (mode == D_HOSTS) && (version == 2) )
			{
				if(walk_cached_hosts->type == 'H')
				{
					snprintf(buff,255,"%c|%s|%ln|\n",walk_cached_hosts->type,walk_cached_hosts->addr,&secs_old);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{
						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
				}
			}
			else if( (mode == D_HOSTS) && (version == 1) )
			{
				if(walk_cached_hosts->type == 'H')
				{
					snprintf(buff,255,"%s\n",walk_cached_hosts->addr);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{
						return -1;
					}
					else
					{
						total_bytes += rslt;
					}

				}
			}
			else if( (mode == D_URLS) && (version == 2) )
			{
				if(walk_cached_hosts->type == 'U')
				{
					snprintf(buff,255,"%c|%s|%ln|\n",walk_cached_hosts->type,walk_cached_hosts->addr,&secs_old);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{
						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
				}
			}
			else if( (mode == D_URLS) && (version == 1) )
			{
				if(walk_cached_hosts->type == 'U')
				{
					snprintf(buff,255,"%s\n",walk_cached_hosts->addr);
					rslt = write_line_to_socket(c,buff);
					if(rslt == -1)
					{

						return -1;
					}
					else
					{
						total_bytes += rslt;
					}
				}
			}
		}
		walk_cached_hosts = walk_cached_hosts->next;
	}

	return total_bytes;
}


int process_update(char *ip, char *url, struct networks *current_network)
{
	size_t	ip_len = 0;
	size_t	url_len = 0;

	char *url_ptr = NULL;
	int rslt = 0;


	/* Check Submissions */
	if(ip != NULL)
	{
		ip_len = strlen(ip);
		if((ip_len<6) || (ip_len>32))
		{

			return -1;
		}
		if((isdigit(ip[0])))
		{
			unsigned long age = 0;
			unsigned long bdate = 0;
			time_t now = 0;


			now = time(NULL);
			bdate = node_already_exists(current_network->hosts,ip);
			age = now - bdate;
			if(bdate == 0)
			{
				pthread_mutex_lock(&ip_queue_mutex);
				new_queue_entry(&(ctx->host_queue),ip,current_network->name,D_MAX_QUEUE_SIZE);
				pthread_mutex_unlock(&ip_queue_mutex);
			}
			else if(age > 86400)
			{
				pthread_mutex_lock(&data_mutex);
				current_network->hosts = delete_cached_host_by_name(current_network->hosts,ip);
				pthread_mutex_unlock(&data_mutex);
				pthread_mutex_lock(&ip_queue_mutex);
				new_queue_entry(&(ctx->host_queue),ip,current_network->name,D_MAX_QUEUE_SIZE);
				pthread_mutex_unlock(&ip_queue_mutex);
			}
		}
	}
	if(url != NULL)
	{
		rslt = need_more_caches(current_network->hosts,ctx->config->max_urls);
		if(rslt != 1)
		{
			return -1;
		}
		if( (strstr(url,".asp") != NULL) || (strstr(url,".php") != NULL)
		    || (strstr(url,".ASP") != NULL) || (strstr(url,".PHP") != NULL) )
		{
			/* I decided to filter these because these
			   platforms just cannot handle the current load
			   and because they are often set up by hackers
			   to support botnets. There are enough C powered
			   caches out there with Crab & CryptNET. */

			return -1;
		}
		url_ptr = malloc(1024);
		if(url_ptr == NULL)
			return -1;
		memset(url_ptr,0x00,1024);
		url_len = strlen(url);
		if((url_len<12) || (url_len>1000))
		{
			if(url_ptr != NULL)
			{
				free(url_ptr);
			}

			return -1;
		}

		rslt = clean_url_r(url,&url_ptr,1024);
		if(rslt == -1)
		{
			if(url_ptr != NULL)
			{
				free(url_ptr);
			}

			return -1;
		}
		rslt = cache_known_bad(&ctx,url_ptr);
		if(rslt == 1)
		{
			if(url_ptr != NULL)
			{
				free(url_ptr);
			}

			return -1;
		}
		if(url_ptr != NULL)
		{
			if(strcmp(url_ptr, ctx->config->server_url) != 0)
			{
				unsigned long age = 0;
				unsigned long bdate = 0;
				time_t now = 0;


				now = time(NULL);
				bdate = node_already_exists(current_network->hosts,url_ptr);
				age = now - bdate;
				if(bdate == 0)
				{
					pthread_mutex_lock(&ip_queue_mutex);
					new_queue_entry(&(ctx->url_queue),url_ptr,current_network->name,D_MAX_QUEUE_SIZE);
					pthread_mutex_unlock(&ip_queue_mutex);
				}
				else if(age > 86400)
				{
					pthread_mutex_lock(&data_mutex);
					current_network->hosts = delete_cached_host_by_name(current_network->hosts,url_ptr);
					pthread_mutex_unlock(&data_mutex);
					pthread_mutex_lock(&url_queue_mutex);
					new_queue_entry(&(ctx->url_queue),url_ptr,current_network->name,D_MAX_QUEUE_SIZE);
					pthread_mutex_unlock(&url_queue_mutex);
				}
			}
		}
		if(url_ptr != NULL)
		{
			free(url_ptr);
		}
	}

	return 0;
}


int verify_node(char *ip, char *net_name, int port_restrict)
{
	char *oct_1_c = NULL;
	char *oct_2_c = NULL;
	char *oct_3_c = NULL;
	char *oct_4_c = NULL;
	char *port_c = NULL;
	unsigned int	oct_1 = 0;
	unsigned int	oct_2 = 0;
	unsigned int	oct_3 = 0;
	unsigned int	oct_4 = 0;
	unsigned long	port = 0;
	unsigned int	len = 0;
	char node[23];
	char *ip_c = NULL;
	char *ptr = NULL;
	int code = 0;

	int sockfd = 0;
	struct sockaddr_in addr;
	int rslt = 0;
	int rslt2 = 0;
	char tmp_buffer[256];
	struct networks *current_network = NULL;


	if(ip == NULL)
	{
		fprintf(stderr,"System Error: NULL argument passed to verify_node.\n");

		return -1;
	}
	/* A port number is required, appended to the end
	   of the IP following a colon (http standard) */
	if(strchr(ip,':') == NULL)
	{
		return -1;
	}
	len = strlen(ip);
	if( (len < 9) || (len > 21) )
	{
		return -1;
	}

	/* Process the IP into octet integers */
	memset(node,0x00,23);
	strncpy(node,ip,22);
	ip_c = &node[0];
	ptr = strchr(ip_c,':');
	if(ptr == NULL)
	{
		return -1;
	}
	port_c = ptr;
	port_c++;
	ptr[0] = '\0';

	if(!isdigit(ip_c[0]))
	{
		return -1;
	}
	oct_1_c = &ip_c[0];
	ptr = strchr(ip_c,'.');
	if(ptr == NULL)
		return -1;
	oct_2_c = &ptr[0];
	oct_2_c++;
	ptr[0] = '\0';
	ptr = strchr(oct_2_c,'.');
	if(ptr == NULL)
		return -1;
	oct_3_c = &ptr[0];
	oct_3_c++;
	ptr[0] = '\0';
	ptr = strchr(oct_3_c,'.');
	if(ptr == NULL)
		return -1;
	oct_4_c = &ptr[0];
	oct_4_c++;
	ptr[0] = '\0';

	oct_1 = atoi(oct_1_c);
	oct_2 = atoi(oct_2_c);
	oct_3 = atoi(oct_3_c);
	oct_3 = atoi(oct_4_c);
	if( (oct_1 > 254) || (oct_2 > 254) || (oct_3 > 254) || (oct_4 > 254) )
	{
		return -1;
	}
	/* The Loopback Interface: 127.255.255.255 */
	if((oct_1 == 127) || (oct_1 == 10))
	{
		return -1;
	}
	/* Private Space: 192.168.255.255 */
	else if((oct_1 == 192) && (oct_2 == 168))
	{
		return -1;
	}
	/* Reserved Space for Private AutoIP: 169.254.255.255 */
	else if((oct_1 == 169) && (oct_2 == 254))
	{
		return -1;
	}
	/* Private Space: 172.16.255.255 -> 172.31.255.255 */
	else if( (oct_1 == 172) && ((oct_2 > 15) && (oct_2) < 32) )
	{
		return -1;
	}

	/* Process the port into an integer */
	if(!isdigit(port_c[0]))
	{
		return -1;
	}
	port = atol(port_c);
	if( (port > 33000) || (port < 1024) )
	{
		return -1;
	}
	if( (port_restrict == 1) && (port != 6346) )
	{
		return -1;
	}

	strncpy(node,ip,22);
	ptr = strchr(node,':');
	if(ptr == NULL)
		return -1;
	ptr[0] = '\0';

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
		return -1;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(&node[0]);
	addr.sin_port = htons(port);
	len = sizeof(addr);

	rslt = connect_nonblock(ctx,&sockfd,(struct sockaddr *)&addr,len);
	if(rslt == -1)
	{
		close(sockfd);

		return -1;
	}

	/* connect to the gnutella node (check for not busy) */

	rslt = write_line_to_socket(sockfd,"GNUTELLA CONNECT/0.6\r\n");
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}
	rslt = write_line_to_socket(sockfd,"Node: 128.227.212.74:6346\r\n");
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}
	snprintf(tmp_buffer,256,"Remote-IP: %s\r\n",ip);
	rslt = write_line_to_socket(sockfd,tmp_buffer);
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}
	rslt = write_line_to_socket(sockfd,"User-Agent: LimeWire/4.6.0\r\n\r\n");
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}

	// read the response
	memset(tmp_buffer,0x00,256);
	rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
	if(rslt < 2)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}
	if(strstr(tmp_buffer,"503") != NULL)
	{
		code = 503;
	}
	if(strstr(tmp_buffer,"200") != NULL)
	{
		code = 200;
	}

	//  Read The Headers
	current_network = get_network_by_name(ctx->networks,net_name);
	if(current_network == NULL)
	{
		fprintf(stderr,"get_network_by_name() returned NULL\n");
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);

		return -1;
	}

	do
	{
		rslt = read_line(sockfd,tmp_buffer,sizeof(tmp_buffer));
		rslt2 = memcmp("X-Try-Ultrapeers: ",tmp_buffer,18);
		if(rslt2 == 0)
		{
			process_ultrapeers(tmp_buffer,current_network);
		}
	} while((tmp_buffer[0] != '\r') && (tmp_buffer[0] != '\n') && (rslt > 2));

	rslt = write_line_to_socket(sockfd,"GNUTELLA/0.6 503 Servent Shutdown\r\n");
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		if(code == 200)
			return 0;
		else
			return -1;
	}
	rslt = write_line_to_socket(sockfd,"User-Agent: LimeWire/4.6.0\r\n\r\n");
	if(rslt == -1)
	{
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		if(code == 200)
			return 0;
		else
			return -1;
	}

	shutdown(sockfd,SHUT_RDWR);
	close(sockfd);

	if(code == 200)
		return 0;
	else
		return -1;

	return 0;
}


int process_ultrapeers(char *peers,struct networks *current_network)
{
	char *n = NULL;
	char *p = NULL;
	char *node = NULL;


	if(peers == NULL)
		return -1;

	n = strchr(peers,' ');
	if(n == NULL)
		return -1;

	*n = '\0';
	n++;

	if(n != NULL)
	{
		node = strtok_r(n,",",&p);
		do
		{
			process_update(node,NULL,current_network);
		} while(((node) = (strtok_r('\0',",",&p))));
	}

	return 0;
}


int cache_known_bad(struct cpcd_ctx **ctx, char *url)
{
	unsigned long age = 0;
	unsigned long bdate = 0;
	time_t now = 0;


	now = time(NULL);
	bdate = node_already_exists((*ctx)->bad_caches,url);
	if(bdate == 0)
	{
		return 0;
	}

	age = now - bdate;
	if(age > 604800) /* 604800 Seconds = 1 Week */
	{
		pthread_mutex_lock(&data_mutex);
		(*ctx)->bad_caches = delete_cached_host_by_name((*ctx)->bad_caches,url);
		pthread_mutex_unlock(&data_mutex);
	}
	else
	{
		return 1;
	}

	return 0;
}
