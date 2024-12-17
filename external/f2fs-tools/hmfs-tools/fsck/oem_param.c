/*
 * oem_param.c
 *
 * Copyright (c) 2018~ Huawei Technologies Co., Ltd.
 *
 * process data preload files with file or special path
 * used params -P and -F
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "oem_param.h"

#ifdef HW_F2FS_IMG_OEM
#define MAX_PATH        128
#define MAX_FILE        256
#define PARSED_INIT     0
#define PARSED_FOUND    1
#define PARSED_NONE     2
#define LINE_MAX_LENGTH 1024
#define DELIM_FLAG      "/\r\n"

// used for cmd -P param
static char *g_path_relative[MAX_PATH];
static int g_total_path_count;
// used for cmd -F param
static char *g_full_path_file[MAX_FILE][MAX_PATH];
static int g_total_file_path_count[MAX_FILE];
static char g_file_path_pass[LINE_MAX_LENGTH];
static int g_max_step_length;
static int g_total_file_count;
// used for both cmd -P && -F
static int g_path_index;

/*
 * FunctionName:  set_token_string
 * Description :  set a string parsed by strtok to target
 * Parameters: index: position in target
 *		 value: string input to target
 *		 target: dest to save string
 * return value: 0 success
 *	   1 error
 */
static int set_token_string(int index, const char *value, char **target)
{
	char *newstring = NULL;
	int buflen;

	if (value == NULL || (value && strlen(value) == 0)) {
		printf("input param error\n");
		return 1;
	}
	buflen = (int)strlen(value) + 1;
	newstring = malloc(buflen);
	if (newstring == NULL) {
		printf("malloc mem failed\n");
		return 1;
	}

	if (memset_s(newstring, buflen, 0, buflen) != EOK)
		printf("memset_s maybe error\n");

	if (memcpy_s(newstring, buflen, value, buflen - 1) != EOK)
		printf("memcpy_s maybe error\n");

	target[index] = newstring;

	return 0;
}

/*
 * FunctionName:  release_special_config_mem
 * Description :  release mem used for sparse special config
 */
void release_special_config_mem(void)
{
	int i;

	if (g_total_path_count > 0) {
		for (i = 0; i < g_total_path_count; i++) {
			if (g_path_relative[i]) {
				free(g_path_relative[i]);
				g_path_relative[i] = NULL;
			}
		}
	}

	if (g_total_file_count > 0) {
		int nstr;
		int j;

		for (i = 0; i < g_total_file_count; i++) {
			nstr = g_total_file_path_count[i];
			for (j = 0; j < nstr; j++) {
				if (g_full_path_file[i][j]) {
					free(g_full_path_file[i][j]);
					g_full_path_file[i][j] = NULL;
				}
			}
		}
	}
	// free the mem for config file
	if (c.target_special_file != NULL) {
		free(c.target_special_file);
		c.target_special_file = NULL;
	}
}

/*
 * FunctionName:  parse_path_string
 * Description :  parsed input string with delim and store result to output
 * Parameters: inputstr: input string
 *		 output_target: target to store the parse result
 *		 delim: flag of input to parse
 * return value: parsed output count of result
 */
static int parse_path_string(const char *inputstr, char **output_target, const char *deli)
{
	int outputcount;
	int countno = 0;
	char *token = NULL;
	char *nextoken = NULL;
	char *pathbuf = NULL;
	int  pathlen;

	if (inputstr == NULL)
		return 0;

	pathlen = (int)strlen(inputstr);
	pathbuf = malloc(pathlen + 1);
	if (pathbuf) {
		int idx = 0;

		if (memset_s(pathbuf, pathlen + 1, 0, pathlen + 1) != EOK)
			printf("memset_s maybe error\n");

		if (memcpy_s(pathbuf, pathlen + 1, inputstr, pathlen + 1) != EOK)
			printf("memcpy_s maybe error\n");

		token = strtok_s(pathbuf, deli, &nextoken);
		while (token) {
			if (set_token_string(idx++, (const char *)token, output_target)) {
				countno = idx;
				idx = 0;
				printf("ERROR: failed when set token to path relative!!!!!!!\n");
				break;
			}

			token = strtok_s(NULL, deli, &nextoken);
			if (idx == MAX_PATH && token) {
				countno = idx;
				idx = 0;
				printf("###########ERROR: path step is more than %d deep!!!!!!###############\n", MAX_PATH);
				break;
			}
		}
		free(pathbuf);
		outputcount = idx;
	} else {
		printf("ERROR: malloc mem failed when parse the relative path\n");
		outputcount = 0;
	}

	if (countno != 0 && outputcount == 0) {
		int j;

		for (j = 0; j < countno; j++) {
			free(output_target[j]);
			output_target[j] = NULL;
		}
	}

	return outputcount;
}

/*
 * FunctionName:  parse_input_relative_path
 * Description :  parse the relative input path for create image
 */
static void parse_input_relative_path(void)
{
	g_total_path_count = parse_path_string((const char *)c.target_special_dir, g_path_relative, DELIM_FLAG);
}

/*
 * FunctionName:  parse_input_full_path_file
 * Description :  parse the input file config for create image
 */
static void parse_input_full_path_file(void)
{
	char tmpbuf[LINE_MAX_LENGTH] = {0};
	int filecount = 0;
	FILE *spf = fopen(c.target_special_file, "r");

	if (spf == NULL) {
		printf("ERROR: open file failed[%s]\n", strerror(errno));
		return;
	}

	while (!feof(spf)) {
		if (memset_s(tmpbuf, LINE_MAX_LENGTH, 0, LINE_MAX_LENGTH) != EOK)
			printf("memset_s maybe error");

		if (fgets(tmpbuf, LINE_MAX_LENGTH, spf)) {
			int nret;

			printf("get config line: %s\n", tmpbuf);
			nret = parse_path_string((const char *)tmpbuf, g_full_path_file[filecount], DELIM_FLAG);
			if (nret > 0) {
				g_total_file_path_count[filecount] = nret;
				if (g_max_step_length < nret)
					g_max_step_length = nret;
				filecount++;
			}
		} else {
			//null terminate line also return null
			if (errno) {
				printf("ERROR: read file line error[%s]", strerror(errno));
				filecount = 0;
				break;
			}
		}
	}
	g_total_file_count = filecount;
	fclose(spf);
}

/*
 * FunctionName:  parse_input_full_path_file
 * Description :  parse the input file config for create image
 */
int parse_file_or_path_input(void)
{
	if (c.target_special_dir && c.target_special_dir[0] != '/') {
		parse_input_relative_path();
		return PARSED_FOUND;
	} else if (c.target_special_file) {
		parse_input_full_path_file();
		return PARSED_FOUND;
	}

	return PARSED_NONE;
}

/*
 * FunctionName:  checkfile_or_path_valid
 * Description :  check file scaned whether is specified
 * Parameters: cmpfile : input filename to be compare
 *		 pathindex: step count the in file path
 * return value: 1 can be copy to image
 *	   0 can not be used
 */
int checkfile_or_path_valid(const char *cmpfile, int pathindex)
{
	int dirok = 0;
	int i;

	if (cmpfile == NULL)
		return 1;

	if (g_total_file_count <= 0)
		return 0;

	// file config or path config only can set one at the same time
	if (c.target_special_file == NULL) {
		if (pathindex < g_total_path_count) {
			printf("enter compare strings, [%s]\n", g_path_relative[pathindex]);
			dirok = (!strcmp(cmpfile, g_path_relative[pathindex]));
		} else {
			dirok = 1;
		}
		return dirok;
	}

	if (pathindex >= g_max_step_length)
		return 1;

	for (i = 0; i < g_total_file_count; i++) {
		int ncount = g_total_file_path_count[i];

		// little, then compare the path index value
		if (pathindex < ncount) {
			dirok = (!strcmp(cmpfile, g_full_path_file[i][pathindex]));
			if (dirok)
				break;
		} else { // else, compare the path before to confirm whether path is same or not, this used for config path not file
			char *tmpparse[MAX_PATH] = {0};
			int tmpparsecount;

			if (memset_s(tmpparse, sizeof(char *) * MAX_PATH, 0, sizeof(char *) * MAX_PATH) != EOK)
				printf("memset_s maybe error\n");

			tmpparsecount = parse_path_string((const char *)g_file_path_pass, tmpparse, DELIM_FLAG);
			if (tmpparsecount > ncount) {
				int nj;

				for (nj = 0; nj < ncount; nj++) {
					dirok = !(strcmp(tmpparse[nj], g_full_path_file[i][nj]));
					if (dirok)
						break;
				}
				//free memory which malloc in function parse_path_string
				for (nj = 0; nj < tmpparsecount; nj++) {
					if (tmpparse[nj]) {
						free(tmpparse[nj]);
						tmpparse[nj] = NULL;
					}
				}
			}
		}
	}

	return dirok;
}

/*
 * FunctionName:  filter_dot
 * Description :  . and .. need not to copy and also check our special config,
 *			check each file&dir whether can be copy to image or not
 * Parameters: d : each item of file&dir
 * return value: 1 can be copy to image
 *		   0 can not be used
 */
int filter_dot(const struct dirent *d)
{
	static int parsed;
	int dirok = 0;

	dirok = (strcmp(d->d_name, "..") && strcmp(d->d_name, "."));
	/* if . or .. , just return
	 * if no special config, return result also
	 */
	if (!dirok || (c.target_special_file == NULL && c.target_special_dir == NULL))
		return dirok;

	if (!parsed)
		parsed = parse_file_or_path_input();

	if (parsed == PARSED_FOUND)
		dirok = (int)((dirok != 0) && (checkfile_or_path_valid(d->d_name, g_path_index) != 0));

	return dirok;
}

/*
 * FunctionName:  modify_path_index_value
 * Description :  modify the path index value,
 * Parameters: value : use the value to modify path index
 *		 binit : if true use the value set to path index
 *		         else use path index add the value
 */
void modify_path_index_value(int value, int binit)
{
	if (binit)
		g_path_index = value;
	else
		g_path_index += value;
}

/*
 * set path for file config var from input path
 */
void set_path_passed_for_file(const char *path)
{
	if (memset_s(g_file_path_pass, LINE_MAX_LENGTH, 0, LINE_MAX_LENGTH) != EOK)
		printf("WARING: memset_s function maybe error\n");

	if (strcpy_s(g_file_path_pass, LINE_MAX_LENGTH, path) != EOK)
		printf("WARING: strcpy_s function maybe error\n");
}
/*
 * when set pointer to null, check first to release memory
 */
void set_target_file_path_null(void)
{
	if (c.target_special_file != NULL) {
		free(c.target_special_file);
		c.target_special_file = NULL;
	}
}
#endif

