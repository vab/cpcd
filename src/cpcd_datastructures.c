/* cpcd_datastructures.c - Data Structure Functions for CPCD
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

#include "cpcd_common.h"
#include "cpcd_datastructures.h"


/*
 * Initilizational Code For Data Structures
 *
 */

int init_context(struct cpcd_ctx **ctx)
{
	(*ctx)->config = NULL;
	(*ctx)->networks = NULL;
	(*ctx)->bad_caches = NULL;

	(*ctx)->total_hits = 0;
	(*ctx)->updts = 0;
	(*ctx)->reqs = 0;

	(*ctx)->exit_flag = 0;
	(*ctx)->verbose = 0;

	(*ctx)->log = NULL;

	(*ctx)->host_queue = NULL;
	(*ctx)->url_queue = NULL;

	return 0;
}


/*
 * Creational Code For Data Structures
 *
 */

int new_queue_entry(struct queue **the_queue, char *data, char *net, int max_queue_size)
{
	struct queue *new_item;
	int data_len = 0;
	int net_len = 0;

	
	if((data == NULL) || (net == NULL))
	{
		fprintf(stderr,"Invalid values passed to new_queue_item()\n");

		return -1;
	}
	
	data_len = strlen(data);
	net_len = strlen(net);
	if((data_len == 0) || (net_len == 0))
	{
		fprintf(stderr,"Invalid values passed to new_queue_item()\n");

		return -1;
	}
	new_item = (struct queue *)malloc(sizeof(struct queue));
	if(new_item == NULL)
	{
		fprintf(stderr,"Malloc call failed: out of memory\n");
		
		return -1;
	}
	new_item->data = NULL;
	new_item->net_name = NULL;
	new_item->next = NULL;

	new_item->data = (char *)malloc(data_len+2);
	if(new_item->data == NULL)
	{
		fprintf(stderr,"Malloc call failed: out of memory\n");
		if(new_item != NULL)
		{
			free_queue_item(new_item);
		}

		return -1;
	}

	new_item->net_name = (char *)malloc(net_len+2);
	if(new_item->net_name == NULL)
	{
		fprintf(stderr,"Malloc call failed: out of memory\n");
		if(new_item != NULL)
		{
			free_queue_item(new_item);
		}

		return -1;
	}

	memset(new_item->data,0x00,data_len+2);
	memset(new_item->net_name,0x00,net_len+2);
	strncpy(new_item->data,data,data_len+1);
	strncpy(new_item->net_name,net,net_len+1);

	push_entry(the_queue, new_item, max_queue_size);

	return 0;
}


int get_queue_depth(struct queue *the_queue)
{
	int i = 1;
	
	if(the_queue == NULL)
		return 0;
		
	while(the_queue->next != NULL)
	{
		the_queue = the_queue->next;
		i++;
	}

	return i;
}


int push_entry(struct queue **the_queue, struct queue *new_item, int max_queue_size)
{
	struct queue *tail = NULL;
	int i = 0;
	
	
	if(new_item == NULL)
		return -1;
		
	i = get_queue_depth(*the_queue);
	/* A max is provided to prevent memory exhaustion DoS attacks. */
	if(i > max_queue_size)
	{
		struct queue *expired_item = NULL;
		
		expired_item = pop_entry(the_queue);
		free_queue_item(expired_item);
	}
	tail = get_last_item(*the_queue);
	if(tail == NULL)
	{
		*the_queue = new_item;
	}
	else
	{
		tail->next = new_item;
		new_item->next = NULL;
	}

	return 0;
}


struct queue *get_last_item(struct queue *the_queue)
{
	if(the_queue == NULL)
		return NULL;
	
	while(the_queue->next != NULL)
	{
		the_queue = the_queue->next;
	}

	return the_queue;
}


struct queue *pop_entry(struct queue **the_queue)
{
	struct queue *the_item = NULL;


	if(*the_queue == NULL)
		return NULL;
	
	the_item = *the_queue;
	
	if((*the_queue)->next != NULL)
	{
		*the_queue = (*the_queue)->next;
	}
	else
	{
		*the_queue = NULL;
	}
	
	the_item->next = NULL;

	return the_item;
}


 
struct hosts * add_new_cached_host(struct hosts *cached_hosts, char *type_str, char *addr, long bdate)
{
	struct hosts *last_host = NULL;
	struct hosts *new_host = NULL;
	int addr_len = 0;


	addr_len = strlen(addr);
	if(addr_len < 6)
		return cached_hosts;

	new_host = (struct hosts *)malloc(sizeof(struct hosts));
	if(new_host == NULL)
	{
		fprintf(stderr, "Malloc call failed.  Out of memory.\n");

		return cached_hosts;
	}
	new_host->type = type_str[0];
	new_host->addr = (unsigned char *)malloc(addr_len+2);
	if(new_host->addr == NULL)
	{
		fprintf(stderr, "Malloc call failed.  Out of memory.\n");
		if(new_host != NULL)
			free(new_host);

		return cached_hosts;
	}
	memset(new_host->addr,0x00,addr_len+2);
	strncpy(new_host->addr,addr,addr_len+1);
	new_host->bdate = bdate;
	new_host->prev = NULL;
	new_host->next = NULL;

	last_host = get_last_host(cached_hosts);
	if(last_host == NULL)
	{
		cached_hosts = new_host;
		cached_hosts->prev = NULL;
		cached_hosts->next = NULL;
	}
	else
	{
		last_host->next = new_host;
		new_host->prev = last_host;
		new_host->next = NULL;
	}

	return new_host;
}


/*
 * Data Retrevial Code For Data Structures
 *
 */

struct hosts *get_first_host(struct hosts *cached_hosts)
{
	if(cached_hosts != NULL)
	{
		while(cached_hosts->prev != NULL)
		{
			cached_hosts = cached_hosts->prev;
		}
	}


	return cached_hosts;
}


struct hosts *get_last_host(struct hosts *cached_hosts)
{
	if(cached_hosts != NULL)
	{
		while(cached_hosts->next != NULL)
		{
			cached_hosts = cached_hosts->next;
		}
	}


	return cached_hosts;
}


struct hosts *get_oldest_host(struct hosts *cached_hosts, int type)
{
	struct hosts *walk_host = NULL;


	if( ((type != D_URLS) && (type != D_HOSTS)) || (cached_hosts == NULL) )
		return cached_hosts;

	walk_host = get_first_host(cached_hosts);
	if(walk_host == NULL)
	{
		return cached_hosts;
	}

	while(walk_host != NULL)
	{
		if((type == D_URLS) && (walk_host->type == 'U') )
		{
			return walk_host;
		}
		else if((type == D_HOSTS) && (walk_host->type == 'H') )
		{
			return walk_host;
		}

		walk_host = walk_host->next;
	}


	return NULL;
}


struct networks *get_first_network(struct networks *nets)
{
	if(nets != NULL)
	{
		while(nets->prev != NULL)
		{
			nets = nets->prev;
		}
	}

	return nets;
}


struct networks *get_last_network(struct networks *nets)
{
	if(nets != NULL)
	{
		while(nets->next != NULL)
		{
			nets = nets->next;
		}
	}

	return nets;
}


struct networks *get_network_by_name(struct networks *nets,char *name)
{
	struct networks *walk_network = NULL;


	if(nets == NULL)
		return NULL;

	walk_network = get_first_network(nets);

	while(walk_network != NULL)
	{
		if(strcmp(name,walk_network->name) == 0)
		{
			return walk_network;
		}
		else
		{
			walk_network = walk_network->next;
		}
	}

	return NULL;
}


/*
 *  Operative Code on Data Structures
 *
 */

int count_nodes(struct hosts *cached_hosts, int type)
{
	int count = 0;
	struct hosts *walk_host = NULL;


	walk_host = get_first_host(cached_hosts);
	if(walk_host == NULL)
	{
		return -1;
	}

	if(type == D_URLS)
	{
		while(walk_host != NULL)
		{
			if(walk_host->type == 'U')
			{
				count++;
			}

			walk_host = walk_host->next;
		}
	}
	else if(type == D_HOSTS)
	{
		while(walk_host != NULL)
		{
			if(walk_host->type == 'H')
			{
				count++;
			}

			walk_host = walk_host->next;
		}
	}
	else
	{
		while(walk_host != NULL)
		{
			count++;

			walk_host = walk_host->next;
		}
	}

	return count;
}


unsigned long node_already_exists(struct hosts *cached_hosts, char *data)
{
	struct hosts *walk_host = NULL;


	if(cached_hosts == NULL)
		return 0;
	if(data == NULL)
		return 1;

	walk_host = get_first_host(cached_hosts);
	while(walk_host != NULL)
	{
		if(strcmp(data,walk_host->addr) == 0)
		{
			return walk_host->bdate;
		}

		walk_host = walk_host->next;
	}

	return 0;
}


struct hosts * trim_struct_to_max(struct hosts *hosts, int max, char type)
{
	int host_count = 0;
	struct hosts *free_host = NULL;


	host_count = count_nodes(hosts,type);
	while(host_count > max)
	{
		free_host = get_oldest_host(hosts,type);
		hosts = delete_cached_host(free_host);
	}

	return hosts;
}


struct hosts * delete_cached_host(struct hosts *host_to_delete)
{
	struct hosts *rtrn = NULL;


	if(host_to_delete != NULL)
        {
		rtrn = get_first_host(host_to_delete);
		if(rtrn == host_to_delete)
		{
			if(rtrn->next == NULL)
			{
				rtrn = NULL;
			}
			else
			{
				rtrn = rtrn->next;
			}
		}

		if(host_to_delete->prev != NULL)
		{
			host_to_delete->prev->next = host_to_delete->next;
		}
		if(host_to_delete->next != NULL)
		{
			host_to_delete->next->prev = host_to_delete->prev;
		}
		if(host_to_delete->addr != NULL)
		{
			free(host_to_delete->addr);
			host_to_delete->addr = NULL;
		}
		free(host_to_delete);
		host_to_delete = NULL;
	}


	return rtrn;
}


struct hosts * delete_cached_host_by_name(struct hosts *cached_hosts, char *name)
{
	struct hosts *walk_host = NULL;
	struct hosts *rtrn = NULL;


	if( (name == NULL) || (cached_hosts == NULL) )
		return cached_hosts;

	rtrn = walk_host = get_first_host(cached_hosts);

	while(walk_host != NULL)
	{
		if(strcmp(name,walk_host->addr) == 0)
		{
			rtrn = delete_cached_host(walk_host);
			return rtrn;
		}
		else
		{
			walk_host = walk_host->next;
		}
	}

	return rtrn;
}


struct networks *add_new_network(struct networks *nets, struct networks *new_net)
{
	struct networks *last_net = NULL;


	printf("Adding: %s\n",new_net->name);
	last_net = get_last_network(nets);
	if(last_net == NULL)
	{
		nets = new_net;
		nets->prev = NULL;
		nets->next = NULL;
	}
	else
	{
		last_net->next = new_net;
		new_net->prev = last_net;
		new_net->next = NULL;
	}

	return new_net;
}


/*
 *  Functions to Free Alloc's Structures
 *
 */

void free_queue_item(struct queue *the_item)
{
	if(the_item != NULL)
	{
		if(the_item->data != NULL)
		{
			free(the_item->data);
			the_item->data = NULL;
		}
		if(the_item->net_name != NULL)
		{
			free(the_item->net_name);
			the_item->net_name = NULL;
		}
		free(the_item);
		the_item = NULL;
	}

	return;
}

void free_queue(struct queue *the_queue)
{
	struct queue *next_item = NULL;
	
	while(the_queue != NULL)
	{
		next_item = the_queue->next;
		free_queue_item(the_queue);
		the_queue = next_item;
	}
}


void free_cached_hosts(struct hosts *cached_hosts)
{
	struct hosts *walk_host = NULL;
	struct hosts *next_host = NULL;


	if(cached_hosts != NULL)
	{
		walk_host = (struct hosts *)get_first_host(cached_hosts);
		while(walk_host != NULL)
		{
			next_host = walk_host->next;
			if(walk_host->addr != NULL)
			{
				free(walk_host->addr);
				walk_host->addr = NULL;
			}
			free(walk_host);
			walk_host = next_host;
		}
	}

	return;
}


void free_networks(struct networks *networks)
{
	struct networks *next_networks = NULL;

	networks = get_first_network(networks);
	if(networks != NULL)
	{
		while(networks != NULL)
		{
			printf("Freeing: %s\n",networks->name);
			next_networks = networks->next;
			if(networks->hosts != NULL)
			{
				free_cached_hosts(networks->hosts);
			}
			if(networks->bad_hosts != NULL)
			{
				free_cached_hosts(networks->bad_hosts);
			}
			free(networks);
			networks = next_networks;
		}
	}

	return;
}


void free_context(struct cpcd_ctx *ctx)
{
	if(ctx != NULL)
	{
		if(ctx->host_queue != NULL)
			free_queue(ctx->host_queue);
		if(ctx->url_queue != NULL)
			free_queue(ctx->url_queue);
		if(ctx->config != NULL)
		{
			free(ctx->config);
		}
		free_networks(ctx->networks);
		free(ctx);
	}

	return;
}
