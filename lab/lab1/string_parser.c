/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/

	// check for NULL string
	// returning count 1 will account for lone NULL token in command_list
	if (buf == NULL) {
		return 1;
	}

	// nondestructive by duplicating input string
	char* str = strdup(buf);
	char* token, *saveptr;
	int num_token = 0;
	token = strtok_r(str, delim, &saveptr);

	while (token != NULL) {
		// strings ending with delimiter will get an extra token containing '\n'
		// accounted for by stripping '\n' in str_filler
		// if (token[0] == '\n')
		//     break;
		num_token++;
		token = strtok_r(NULL, delim, &saveptr);
	}

	free(str);
	// account NULL for last token
	return ++num_token;
}

command_line str_filler (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	create command_line variable to be filled and returned
	*	#2.	count the number of tokens with count_token function, set num_token. 
    *           one can use strtok_r to remove the \n at the end of the line.
	*	#3. malloc memory for token array inside command_line variable
	*			based on the number of tokens.
	*	#4.	use function strtok_r to find out the tokens 
    *   #5. malloc each index of the array with the length of tokens,
	*			fill command_list array with tokens, and fill last spot with NULL.
	*	#6. return the variable.
	*/

	command_line to_fill;
	char* str = strdup(buf);
	char* end_str, *end_token;
	int i = 0;

	char* str_stripped = strtok_r(str, "\n", &end_str);
	to_fill.num_token = count_token(str_stripped, delim);

	to_fill.command_list = malloc(sizeof(char *) * to_fill.num_token);

	char* token = strtok_r(str_stripped, delim, &end_token);
	while (token != NULL) {
		// length of token plus NULL character
		to_fill.command_list[i] = malloc(strlen(token) + 1);
		strcpy(to_fill.command_list[i], token);
		token = strtok_r(NULL, delim, &end_token);
		i++;
	}
	to_fill.command_list[to_fill.num_token - 1] = NULL;
	
	free(str);
	return to_fill;
}


void free_command_line(command_line* command)
{
	//TODO：
	/*
	*	#1.	free the array base num_token
	*/

	for (int i = 0; i < command->num_token; i++)
		free(command->command_list[i]);
	free(command->command_list);
}
