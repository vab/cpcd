/* datastructures.h - Datastructure Routines Header File
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

#ifndef DATASTRUCTURES
#define DATASTRUCTURES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct name_value_pair_dllst
{
	unsigned char *name;
	unsigned char *value;

	struct name_value_pair_dllst *prev;
	struct name_value_pair_dllst *next;
};


struct name_value_pair_dllst* new_dll_node(void *, unsigned long, void *, unsigned long);
int add_dllst_item(struct name_value_pair_dllst **, struct name_value_pair_dllst *);
struct name_value_pair_dllst *get_first_pair(struct name_value_pair_dllst *);
struct name_value_pair_dllst *get_last_pair(struct name_value_pair_dllst *);
char *get_value(struct name_value_pair_dllst *,char *);
void free_name_value_pair_dllst(struct name_value_pair_dllst *);

#endif
