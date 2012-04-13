/* cgi.h - CGI Routines Header File
 * Author:    V. Alex Brennen (codepoet@dublin.ie)
 *            [ http://cryptnet.net/people/vab/ ]
 * Created:   2005.02.01
 * Modified:  2005.03.31
 *
 * This file is part of the CryptNET Code Repository.
 * The repoistory may be accessed at the project's 
 * web page: http://cryptnet.net/fsp/ccr/
 *
 * The contents of the CryptNET Code Repository are free
 * software; they have been placed in the public domain.
 *
 * Submitting a patch for repository code renders your patch
 * public domain as well.  If you wish to retain the copyright
 * to your patch, do not submit it to CryptNET.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datastructures.h"


struct name_value_pair_dllst *parse_name_value_pairs(char *);
struct name_value_pair_dllst *parse_name_value_pairs_r(char *);
int hex_to_ascii(char *);
