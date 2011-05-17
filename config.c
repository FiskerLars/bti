/*
 * Copyright (C) 2008-2011 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2009 Bart Trojanowski <bart@jukie.net>
 * Copyright (C) 2009-2010 Amir Mohammad Saied <amirsaied@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curl/curl.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <pcre.h>
#include <termios.h>
#include <dlfcn.h>
#include <oauth.h>
#include "bti.h"

typedef int (*config_function_callback)(struct session *session, char *value);
typedef int (*defined_value_callback)(struct session *session);
typedef int (*cpy_value_callback)(struct session *a, struct session* b);

struct config_function {
      const char *key;
      config_function_callback callback;
      defined_value_callback defined;
      cpy_value_callback cpy;
};

/*
 * get_key function
 *
 * Read a line from the config file and assign it a key and a value.
 *
 * This logic taken almost identically from taken from udev's rule file parsing
 * logic in the file udev-rules.c, written by Kay Sievers and licensed under
 * the GPLv2+.  I hate writing parsers, so it makes sense to borrow working
 * logic from those smarter than I...
 */
static int get_key(struct session *session, char *line, char **key, char **value)
{
	char *linepos;
	char *temp;
	char terminator;

	linepos = line;
	if (linepos == NULL || linepos[0] == '\0')
		return -1;

	/* skip whitespace */
	while (isspace(linepos[0]) || linepos[0] == ',')
		linepos++;
	if (linepos[0] == '\0')
		return -1;

	*key = linepos;

	for (;;) {
		linepos++;
		if (linepos[0] == '\0')
			return -1;
		if (isspace(linepos[0]))
			break;
		if (linepos[0] == '=')
			break;
	}

	/* remember the end of the key */
	temp = linepos;

	/* skip whitespace after key */
	while (isspace(linepos[0]))
		linepos++;
	if (linepos[0] == '\0')
		return -1;

	/* make sure this is a = operation */
	/*
	 * udev likes to check for += and == and lots of other complex
	 * assignments that we don't care about.
	 */
	if (linepos[0] == '=')
		linepos++;
	else
		return -1;

	/* terminate key */
	temp[0] = '\0';

	/* skip whitespace after opearator */
	while (isspace(linepos[0]))
		linepos++;
	if (linepos[0] == '\0')
		return -1;

	/*
	 * if the value is quoted, then terminate on a ", otherwise space is
	 * the terminator.
	 * */
	if (linepos[0] == '"') {
		terminator = '"';
		linepos++;
	} else
		terminator = ' ';

	/* get the value */
	*value = linepos;

	/* terminate */
	temp = strchr(linepos, terminator);
	if (temp) {
		temp[0] = '\0';
		temp++;
	} else {
		/*
		 * perhaps we just hit the end of the line, so there would not
		 * be a terminator, so just use the whole rest of the string as
		 * the value.
		 */
	}
	/* printf("%s = %s\n", *key, *value); */
	return 0;
}

static int session_string(char **field, char *value)
{
	char *string;

	string = strdup(value);
	if (string) {
		if (*field)
			free(*field);
		*field = string;
		return 0;
	}
	return -1;
}

static int session_bool(int *field, char *value)
{
	if ((strncasecmp(value, "true", 4) == 0) ||
	    strncasecmp(value, "yes", 3) == 0)
		*field = 1;
	return 0;
}

static int account_callback(struct session *session, char *value)
{
	return session_string(&session->account, value);
}

static int account_defined(struct session * session) 
{
      return session->account != 0;
}
static int account_cpy(struct session *a, struct session* b)
{
      return session_string(&b->account, a->account);
}


static int password_callback(struct session *session, char *value)
{
	return session_string(&session->password, value);
}
static int password_defined(struct session * session) 
{
      return session->password != 0;
}
static int password_cpy(struct session *a, struct session* b)
{
      return session_string(&b->password, a->password);
}

static int proxy_callback(struct session *session, char *value)
{
	return session_string(&session->proxy, value);
}
static int proxy_defined(struct session * session) 
{
      return session->proxy != 0;
}
static int proxy_cpy(struct session *a, struct session* b)
{
      return session_string(&b->proxy, a->proxy);
}

static int user_callback(struct session *session, char *value)
{
	return session_string(&session->user, value);
}
static int user_defined(struct session * session) 
{
      return session->user != 0;
}
static int user_cpy(struct session *a, struct session* b)
{
      return session_string(&b->user, a->user);
}

static int consumer_key_callback(struct session *session, char *value)
{
	return session_string(&session->consumer_key, value);
}
static int consumer_key_defined(struct session * session) 
{
      return session->consumer_key != 0;
}
static int consumer_key_cpy(struct session *a, struct session* b)
{
      return session_string(&b->consumer_key, a->consumer_key);
}

static int consumer_secret_callback(struct session *session, char *value)
{
	return session_string(&session->consumer_secret, value);
}
static int consumer_secret_defined(struct session * session) 
{
      return session->consumer_secret != 0;
}
static int consumer_secret_cpy(struct session *a, struct session* b)
{
      return session_string(&b->consumer_secret, a->consumer_secret);
}

static int access_token_key_callback(struct session *session, char *value)
{
	return session_string(&session->access_token_key, value);
}
static int access_token_key_defined(struct session * session) 
{
      return session->access_token_key != 0;
}
static int access_token_key_cpy(struct session *a, struct session* b)
{
      return session_string(&b->access_token_key, a->access_token_key);
}

static int access_token_secret_callback(struct session *session, char *value)
{
	return session_string(&session->access_token_secret, value);
}
static int access_token_secret_defined(struct session * session) 
{
      return session->access_token_secret != 0;
}
static int access_token_secret_cpy(struct session *a, struct session* b)
{
      return session_string(&b->access_token_secret, a->access_token_secret);
}

static int logfile_callback(struct session *session, char *value)
{
	return session_string(&session->logfile, value);
}
static int logfile_defined(struct session * session) 
{
      return session->logfile != 0;
}
static int logfile_cpy(struct session *a, struct session* b)
{
      return session_string(&b->logfile, a->logfile);
}

static int replyto_callback(struct session *session, char *value)
{
	return session_string(&session->replyto, value);
}
static int replyto_defined(struct session * session) 
{
      return session->replyto != 0;
}
static int replyto_cpy(struct session *a, struct session* b)
{
      return session_string(&b->replyto, a->replyto);
}

static int retweet_callback(struct session *session, char *value)
{
	return session_string(&session->retweet, value);
}
static int retweet_defined(struct session * session) 
{
      return session->retweet != 0;
}
static int retweet_cpy(struct session *a, struct session* b)
{
      return session_string(&b->retweet, a->retweet);
}

static int host_callback(struct session *session, char *value)
{
	if (strcasecmp(value, "twitter") == 0) {
		session->host = HOST_TWITTER;
		session->hosturl = strdup(twitter_host);
		session->hostname = strdup(twitter_name);
	} else if (strcasecmp(value, "identica") == 0) {
		session->host = HOST_IDENTICA;
		session->hosturl = strdup(identica_host);
		session->hostname = strdup(identica_name);
	} else {
		session->host = HOST_CUSTOM;
		session->hosturl = strdup(value);
		session->hostname = strdup(value);
	}
	return 0;
}
static int host_defined(struct session * session) 
{
      return session->hostname != 0;
}
static int host_cpy(struct session *a, struct session* b)
{
      b->host = a->host;
      session_string(&b->hosturl, a->hosturl);
      session_string(&b->hostname, a->hostname);
}

static int action_callback(struct session *session, char *value)
{
	if (strcasecmp(value, "update") == 0)
		session->action = ACTION_UPDATE;
	else if (strcasecmp(value, "friends") == 0)
		session->action = ACTION_FRIENDS;
	else if (strcasecmp(value, "user") == 0)
		session->action = ACTION_USER;
	else if (strcasecmp(value, "replies") == 0)
		session->action = ACTION_REPLIES;
	else if (strcasecmp(value, "public") == 0)
		session->action = ACTION_PUBLIC;
	else if (strcasecmp(value, "group") == 0)
		session->action = ACTION_GROUP;
	else
		session->action= ACTION_UNKNOWN;
	return 0;
}
static int action_defined(struct session * session) 
{
      return session->action != ACTION_UPDATE ||
	    session->action != ACTION_UPDATE ||
	    session->action != ACTION_FRIENDS ||
	    session->action != ACTION_USER ||
	    session->action != ACTION_PUBLIC ||
	    session->action != ACTION_GROUP;
}
static int action_cpy(struct session* a, struct session* b)
{
      b->action = a->action;
      return 0;
}

static int verbose_callback(struct session *session, char *value)
{
	return session_bool(&session->verbose, value);
}
static int verbose_defined(struct session * session) 
{
      return session->verbose != 0;
}
static int verbose_cpy(struct session* a, struct session* b)
{
      b->verbose = a->verbose;
      return 0;
}

static int shrink_urls_callback(struct session *session, char *value)
{
	return session_bool(&session->shrink_urls, value);
}
static int shrink_urls_defined(struct session * session) 
{
      return session->shrink_urls != 0;
}
static int shrink_urls_cpy(struct session* a, struct session* b)
{
      b->shrink_urls = a->shrink_urls;
      return 0;
}

/*
 * List of all of the config file options.
 *
 * To add a new option, just add a string for the key name, and the callback
 * function that will be called with the value read from the config file.
 *
 * Make sure the table is NULL terminated, otherwise bad things will happen.
 */
static struct config_function config_table[] = {
      { "account", account_callback, account_defined, account_cpy },
      { "password", password_callback, password_defined, password_cpy },
      { "proxy", proxy_callback, proxy_defined, proxy_cpy },
      { "user", user_callback, user_defined, user_cpy },
      { "consumer_key", consumer_key_callback, consumer_key_defined, consumer_key_cpy },
      { "consumer_secret", consumer_secret_callback, consumer_secret_defined, consumer_secret_cpy },
      { "access_token_key", access_token_key_callback, access_token_key_defined, access_token_key_cpy },
      { "access_token_secret", access_token_secret_callback, access_token_secret_defined, access_token_secret_cpy },
      { "logfile", logfile_callback, logfile_defined, logfile_cpy },
      { "replyto", replyto_callback, replyto_defined, replyto_cpy },
      { "retweet", retweet_callback, retweet_defined, retweet_cpy },
      { "host", host_callback, host_defined, host_cpy },
      { "action", action_callback, action_defined, action_cpy },
      { "verbose", verbose_callback, verbose_defined, verbose_cpy },
      { "shrink-urls", shrink_urls_callback, shrink_urls_defined, shrink_urls_cpy },
      { NULL, NULL, NULL, NULL }
};

static void process_line(struct session *session, char *key, char *value)
{
	struct config_function *item;
	int result;

	if (key == NULL || value == NULL)
		return;

	item = &config_table[0];
	for (;;) {
		if (item->key == NULL || item->callback == NULL)
			break;

		if (strncasecmp(item->key, key, strlen(item->key)) == 0) {
			/*
			 * printf("calling %p, for key = '%s' and value = * '%s'\n",
			 * 	  item->callback, key, value);
			 */
			result = item->callback(session, value);
			if (!result)
				return;
		}
		item++;
	}
}

struct session **bti_parse_configfile(struct session* session)
{
	FILE *config_file;
	char *line = NULL;
	char *key = NULL;
	char *value = NULL;
	char *hashmarker;
	size_t len = 0;
	ssize_t n;
	char *c;
	struct session **accounts;
	int ses_cnt = 1;
	config_file = fopen(session->configfile, "r");

	/* No error if file does not exist or is unreadable.  */
	if (config_file == NULL)
		return;

	accounts = malloc(2*sizeof(struct session*));
	accounts[0] = session;

	do { // parse all lines
		n = getline(&line, &len, config_file);
		if (n < 0)
			break;
		if (line[n - 1] == '\n')
			line[n - 1] = '\0';

		/*
		 * '#' is comment markers, like bash style but it is a valid
		 * character in some fields, so only treat it as a comment
		 * marker if it occurs at the beginning of the line, or after
		 * whitespace
		 */
		hashmarker = strchrnul(line, '#');
		if (line == hashmarker)
			line[0] = '\0';
		else {
			while (hashmarker[0] != '\0') {
				--hashmarker;
				if (isblank(hashmarker[0]))
					hashmarker[0] = '\0';
				else {
					/*
					 * false positive; '#' occured
					 * within a string
					 */
					hashmarker = strchrnul(hashmarker+2, '#');
				}
			}
		}
		c = line;
		while (isspace(*c))
			c++;
		/* Ignore blank lines.  */
		if (c[0] == '\0')
			continue;

		/* parse the line into a key and value pair */
		get_key(session, c, &key, &value);
		dbg("key,val %s,%s\n", key, value);

		if (0 == strcmp(config_table[0].key, key)) {
		      accounts = (struct session**)realloc(accounts, (ses_cnt + 2) * sizeof(struct session*));
		      accounts[ses_cnt] = (struct session*) session_alloc();
		      session = accounts[ses_cnt];
		      ses_cnt++;
		}
		process_line(session, key, value);
	} while (!feof(config_file));

	accounts[ses_cnt] = 0;


	/* Free buffer and close file.  */
	free(line);
	fclose(config_file);
	return accounts;
}

/** update sessions from default (first) session un accaunts, overruled by cli_session
 */
void consolidate_config(struct session **accounts) 
{
      int i;
      struct config_function* cf;

      for (i = 1; accounts[i] != 0; i++) {
	    for (cf = &(config_table[1]); cf->key != NULL;cf++)
		  if(!cf->defined(accounts[i]) && cf->defined(accounts[0]))
			cf->cpy(accounts[0], accounts[i]);
	    // additional fields (TODO: might be missing some)
	    if(!accounts[1]->time && accounts[0]->time)
		  accounts[1]->time = strdup(accounts[0]->time);
	    if(!accounts[1]->homedir && accounts[0]->homedir)
		  accounts[1]->homedir = strdup(accounts[0]->homedir);
	    if(!accounts[1]->configfile && accounts[0]->configfile)
		  accounts[1]->configfile = strdup(accounts[0]->configfile);
	    if(!accounts[1]->proxy && accounts[0]->proxy)
		  accounts[1]->proxy = strdup(accounts[0]->proxy);
      // ??int interactive;
      }


}


void cpy_ovwrt_session(struct session* a, struct session* b)
{
      struct config_function* cf;
      for (cf = &config_table[0]; cf->key != NULL; cf++ )
	    if(cf->defined(a))
		  cf->cpy(a, b);
}
