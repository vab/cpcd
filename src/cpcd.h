/* cpcd.h - Main Header File for cpcd
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

#ifndef CPCD
#define CPCD

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "cpcd_datastructures.h"
#include "cpcd_common.h"
#include "cpcd_config.h"
#include "cpcd_util.h"
#include "cpcd_html.h"
#include "datastructures.h"
#include "cgi.h"

/* Config Vars */
#define TRUE	1
#define D_MAX_QUEUE_SIZE 250


/* Update Functions */
int process_update(char *, char *, struct networks *);

/* Result/Output Functions */
long print_cached_hosts(int,struct hosts *, int, int, char *);

/* Socket Functions */
int	accept_connect(int,struct cpcd_config *);

/* Signal Functions */
void	inthandler(int);

/* Thread Functions */
void	*thread_main(void *);
void	*ip_verify_thread_main(void *);
void	*url_verify_thread_main(void *);
void	*file_writer_thread_main(void *);

int process_node(struct queue *);
int process_url(struct queue *);
int write_files(void);

/* Gnutella Node Verification Related Functions */
int verify_node(char *, char *, int);
int process_ultrapeers(char *, struct networks *);
int cache_known_bad(struct cpcd_ctx **, char *);

static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ip_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t url_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif
