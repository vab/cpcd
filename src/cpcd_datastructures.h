/* cpcd_datastructures.h - Data Structure Header File for CPCD
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

#ifndef CPCD_DATASTRUCTURES
#define CPCD_DATASTRUCTURES

#include "cpcd_common.h"


struct hosts
{
	char		type;
	unsigned char	*addr;
	unsigned long	bdate;

	struct hosts *next;
	struct hosts *prev;
};

struct networks
{
	char name[256];

	struct hosts *hosts;
	struct hosts *bad_hosts;

	unsigned int dirty;

	struct networks *next;
	struct networks *prev;
};

struct cpcd_config
{
	char allowed_networks[256];
	char default_allow[256];
	char default_network[256];
	char cpcd_version[50];
	char server_url[256];
	char data_dir[256];
	char log_file[256];
	char pid_file[256];

	int cpcd_keep_log;
	int cpcd_throttle_support;

	int max_bad_urls;

	int max_hosts;
	int max_urls;
	int max_nets;

	int listen_threads;
	int host_threads;

	int port_restriction;

	unsigned char bind_ip[16];
	unsigned char web_bind_port[10];
	unsigned char gnutella_bind_port[10];

};

struct cpcd_ctx
{
	struct cpcd_config *config;
	struct networks *networks;
	struct hosts *bad_caches;

	unsigned long total_hits;
	unsigned long updts;
	unsigned long reqs;

	int exit_flag;
	int verbose;
	
	struct queue *host_queue;
	struct queue *url_queue;

	FILE *log;
};

struct queue
{
	char *data;
	char *net_name;
	
	struct queue *next;
};

int new_queue_entry(struct queue **, char *, char *, int);
int push_entry(struct queue **, struct queue *, int);
struct queue *pop_entry(struct queue **);
int get_queue_depth(struct queue *);
struct queue *get_last_item(struct queue *);
void free_queue_item(struct queue *);

int init_context(struct cpcd_ctx **);
void free_context(struct cpcd_ctx *);


struct networks *get_first_network(struct networks *);
struct networks *get_last_network(struct networks *);
struct networks *add_new_network(struct networks *,struct networks *);
struct networks *get_network_by_name(struct networks *, char *);
void free_networks(struct networks *);

unsigned long node_already_exists(struct hosts *, char *);
int count_nodes(struct hosts *, int);
struct hosts *get_first_host(struct hosts *);
struct hosts *get_last_host(struct hosts *);
struct hosts *get_oldest_host(struct hosts *, int);
struct hosts *add_new_cached_host(struct hosts *, char *, char *, long);
struct hosts *trim_struct_to_max(struct hosts *, int, char);
struct hosts *delete_cached_host(struct hosts *);
struct hosts *delete_cached_host_by_name(struct hosts *, char *);
void free_cached_hosts(struct hosts *);


/* Flags */
#define D_ALL_DATA		   1
#define D_HOSTS			   2
#define D_URLS			   3

#endif
