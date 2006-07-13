/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id: jp2a.c 163 2006-07-13 11:27:28Z csl $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef FEAT_CURL

#ifdef HAVE_CURL_CURL_H
#include "curl/curl.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

//! return 1 if `s' is an URL, 0 if not
int is_url(const char* s) {
	int r = 0;
	r |= !strncmp(s, "ftp://", 6);
	r |= !strncmp(s, "ftps://", 7);
	r |= !strncmp(s, "file://", 7);
	r |= !strncmp(s, "http://", 7);
	r |= !strncmp(s, "tftp://", 7);
	r |= !strncmp(s, "https://", 8);
	return r;
}

// Fork and return filedescriptor of read-pipe for downloaded data
// Returns -1 in case of errors
// You must close() the filedescriptor after using it.
int curl_download(const char* url, const int debug) {
	int p, fd[2];

	if ( (p = pipe(fd)) != 0 ) {
		fprintf(stderr, "Could not create pipe (returned %d)\n", p);
		return -1;
	}

	pid_t pid;
	if ( (pid = fork()) == 0 ) {
		// CHILD process
		close(fd[0]); // close read end

		FILE *fw = fdopen(fd[1], "wb");

		if ( fw == NULL ) {
			fprintf(stderr, "Could not write to pipe\n");
			exit(1);
		}

		curl_global_init(CURL_GLOBAL_ALL);
		atexit(curl_global_cleanup);

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);

		if ( debug )
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); // fail silently on errors
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fflush(fw);
		fclose(fw);
		close(fd[1]);
		exit(0);
	} else if ( pid < 0 ) {
		fprintf(stderr, "Failed to fork\n");
		return -1;
	}

	// PARENT process

	close(fd[1]); // close write end of pipe
	return fd[0];
}

#endif
