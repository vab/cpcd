/* cpcd_util.h - Utility Functions Header File for CPCD
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

#ifndef CPCD_UTIL
#define CPCD_UTIL

#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "cpcd_common.h"
#include "cpcd_datastructures.h"
#include "datastructures.h"


/* Gnutella Web Cache Verification Related Functions */
static const char gwc_ping[] = "?ping=1&net=gnutella&client=TEST&Version=CPWC";
static const char gwc_ping_v2[] = "?ping=1&net=gnutella2&client=TEST&Version=CPWC";

int clean_url_r(char *,char **,int);
int verify_url(struct cpcd_ctx *,char *);
int need_more_caches(struct hosts *, int);
int ping_cache_v1(struct cpcd_ctx *,char *, char *,unsigned long);
int ping_cache_v2(struct cpcd_ctx *,char *, char *,unsigned long);

/* System Related Functions */
int write_pid_file(char *);
int delete_pid_file(char *);

/* File Functions */
int file_exists(char *);
int read_file(struct cpcd_config *, struct hosts **, char *);
int write_file(struct cpcd_config *, struct hosts *, char *);

/* Disk Cache Functions */
int load_network_cache(struct cpcd_ctx **, char *);
int load_network_caches(struct cpcd_ctx **,char *);

/* Socket Related Functions */
int connect_nonblock(struct cpcd_ctx *, int *, struct sockaddr *,unsigned int);
int write_line_to_socket(int, char *);
int read_line(int, unsigned char *, unsigned int);

/* Log File Related Functions */
int log_access(FILE *, char *, unsigned int, char *, unsigned long, char *);

int str_to_lower(char **);
int get_age_r(long *, char **, unsigned int);

#endif
