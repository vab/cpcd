/* cgi.c - CGI Routines Source File
 * Author:    V. Alex Brennen (codepoet@dublin.ie)
 *            [ http://cryptnet.net/people/vab/ ]
 * Created:   2005.02.01
 * Modified:  2005.04.18
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

#include "cgi.h"


struct name_value_pair_dllst *parse_name_value_pairs(char *data)
{
	struct name_value_pair_dllst *cgidata = NULL;
	struct name_value_pair_dllst *current = NULL;

	char *dataptr = NULL;
	char *name = NULL;
	char *true_name = NULL;
	char *true_value = NULL;
	char *p = NULL;


	cgidata = (struct name_value_pair_dllst *)malloc(sizeof(struct name_value_pair_dllst));
	if(cgidata == NULL)
	{
		fprintf(stderr,"Malloc call failed: Out of memory.\n");

		return NULL;
	}

	cgidata->prev = NULL;
	cgidata->next = NULL;
	cgidata->name = NULL;
	cgidata->value = NULL;

	current = cgidata;

	dataptr = &data[0];

	name = (char *)strtok_r(dataptr,"&",&p);
	if(name == NULL)
	{
		fprintf(stderr,"incorrectly formated query.\n");
		free_name_value_pair_dllst(cgidata);

		return NULL;
	}
	do
	{
		true_name = &name[0];
		true_value = (char *)memchr(name,'=',strlen(name));
		if(true_value == NULL)
		{
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}

		true_value[0] = '\0';
		true_value++;
		if(true_value == NULL)
		{
			fprintf(stderr,"Unexpected Error: The value is null.");
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}
		if(hex_to_ascii(true_value) == -1)
		{
			fprintf(stderr,"hex_to_ascii failure\n");
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}

		cgidata->name = (char *)malloc(strlen(true_name)+2);
		if(cgidata->name == NULL)
		{
			fprintf(stderr,"Failed to malloc: out of memory\n");
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}
		strncpy(cgidata->name,true_name,strlen(true_name)+1);
		cgidata->value = (char *)malloc(strlen(true_value)+2);
		if(cgidata->value == NULL)
		{
			fprintf(stderr,"Failed to malloc: out of memory\n");
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}
		strncpy(cgidata->value,true_value,strlen(true_value)+1);
		cgidata->next = (struct name_value_pair_dllst *)malloc(sizeof(struct name_value_pair_dllst));
		if(cgidata->next == NULL)
		{
			fprintf(stderr,"Failed to malloc:  out of memory\n");
			free_name_value_pair_dllst(cgidata);

			return NULL;
		}
		cgidata->next->prev = cgidata;
		cgidata = cgidata->next;
		cgidata->next = NULL;
		cgidata->name = NULL;
		cgidata->value = NULL;
	} while((name = (char *)strtok_r('\0',"&",&p)));

	cgidata = cgidata->prev;

	if(cgidata->next != NULL)
	{
		free(cgidata->next);
		cgidata->next=NULL;
	}

	return cgidata;
}


struct name_value_pair_dllst * parse_name_value_pairs_r(char *data)
{
	struct name_value_pair_dllst *new_node = NULL;
	struct name_value_pair_dllst *tmpdata = NULL;

	char *name = NULL;
	char *value = NULL;
	char *p = NULL;


	name = (char *)strtok_r(&data[0],"&",&p);
	if(name == NULL)
	{
		return NULL;
	}

	do
	{
		value = (char *)memchr(name,'=',strlen(name));
		if(value == NULL)
		{
			continue;
		}
		value[0] = '\0';
		value++;
		
		new_node = new_dll_node((void *)name, strlen(name),(void *)value, strlen(value));
		if(new_node == NULL)
			printf("New Node is NULL\n");
		add_dllst_item(&tmpdata, new_node);
        } while((name = (char *)strtok_r('\0',"&",&p)));

	return tmpdata;
}


int hex_to_ascii(char *s)
{
	static const char *hex = "0123456789ABCDEF";
	unsigned int ascii = 0;
	char *p = NULL;
	char *match = NULL;
	int error = 0;


	if(s == NULL)
	{
		return -1;
	}

	for(p = s; !error && *s !='\0'; s++)
	{
		if(*s == '%')
		{
			s++;
			if((match = (char *)strchr(hex, *s)) != NULL)
			{
				ascii = (unsigned int)(match - hex);

				s++;
				if((match = (char *)strchr(hex, *s)) != NULL)
				{
					ascii <<= 4;
					ascii |= (unsigned int)(match - hex);

					*p++ = (char)ascii;
				}
			}
		}
		else if(*s == '+')
		{
			*p++ = ' ';
		}
		else
		{
			*p++ = *s;
		}
	}
	*p  = '\0';

	return 0;
}
