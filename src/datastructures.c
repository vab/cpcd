/* datastructures.c - Datastructure Routines Source File
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

#include "datastructures.h"




/* 
 * Navigational Code For Data Structures 
 *
 */

struct name_value_pair_dllst* new_dll_node(void *name, unsigned long name_size,
						void *value, unsigned long value_size)
{
	struct name_value_pair_dllst *new_node;

	new_node = (struct name_value_pair_dllst *)malloc(sizeof(struct name_value_pair_dllst));
	if(new_node != NULL)
	{
		new_node->next = NULL;
		new_node->prev = NULL;
		new_node->name = NULL;
		new_node->value = NULL;
		new_node->name = malloc(name_size+2);
		if(new_node->name != NULL)
		{
			strncpy(new_node->name,name,name_size+1);
		}
		else
		{
			free(new_node);
			new_node = NULL;
		}
		if(value != NULL)
		{
			new_node->value = malloc(value_size+2);
			if(new_node->value != NULL)
			{
				strncpy(new_node->value,value,value_size+1);
			}
			else
			{
				free(new_node->name);
				free(new_node);
				new_node = NULL;
			}
		}
	}

	return new_node;
}


int add_dllst_item(struct name_value_pair_dllst **the_list, struct name_value_pair_dllst *new_node)
{
	int rslt = 0;
	struct name_value_pair_dllst *local_list;

	if(the_list != NULL)
	{
		local_list = get_last_pair(*the_list);
		if(local_list == NULL)
		{
			local_list = new_node;
		}
		else
		{
			local_list->next = new_node;
			new_node->prev = local_list;
		}
		*the_list = local_list;
	}
	else
	{
		rslt = -1;
	}

	return rslt;
}


struct name_value_pair_dllst *get_first_pair(struct name_value_pair_dllst *the_list)
{
	if(the_list != NULL)
	{
		while(the_list->prev != NULL)
		{
			the_list = the_list->prev;
		}
	}

	
	return the_list;
}


struct name_value_pair_dllst *get_last_pair(struct name_value_pair_dllst *the_list)
{
	if(the_list != NULL)
	{
		while(the_list->next != NULL)
		{
			the_list = the_list->next;
		}
	}

	
	return the_list;
}

char *get_value(struct name_value_pair_dllst *the_list,char *name)
{
	if(the_list == NULL)
	{
		return NULL;
	}

	/* Rewind to the beginning */
	the_list = get_first_pair(the_list);

	while(the_list != NULL)
	{
		if(strcmp(the_list->name,name) == 0)
		{
			return the_list->value;
		}
		else
		{
			the_list = the_list->next;
		}
	}

	
	return NULL;
}


void free_name_value_pair_dllst(struct name_value_pair_dllst *the_list)
{
	struct name_value_pair_dllst *walk_pair = NULL;
	struct name_value_pair_dllst *next_pair = NULL;

	if(the_list != NULL)
	{
		walk_pair = (struct name_value_pair_dllst *)get_first_pair(the_list);
		while(walk_pair != NULL)
		{
	//		printf("Freeing: %0.8x\n",*walk_pair);
			next_pair = walk_pair->next;
			if(walk_pair->name != NULL)
			{	
	//			printf("Freeing Walkpair Name: %0.8x\n",*(walk_pair->name));
				free(walk_pair->name);
				walk_pair->name = NULL;
			}
			if(walk_pair->value != NULL)
			{
			//	printf("Freeing Walkpair Value: %0.8x\n",*(walk_pair->value));
				free(walk_pair->value);
				walk_pair->value = NULL;
			}
			free(walk_pair);
			walk_pair = next_pair;
		}
	}

	return;
}
