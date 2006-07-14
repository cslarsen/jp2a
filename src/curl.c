/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef FEAT_CURL

#include <stdio.h>

#ifdef HAVE_CURL_CURL_H
#include "curl/curl.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#define close _close
#define pipe(x) _pipe(x, 256, O_BINARY)
#endif

//! Return 1 if s is a supported URL
int is_url(const char* s) {
	return !strncmp(s, "ftp://", 6)
		| !strncmp(s, "ftps://", 7)
		| !strncmp(s, "file://", 7)
		| !strncmp(s, "http://", 7)
		| !strncmp(s, "tftp://", 7)
		| !strncmp(s, "https://", 8);
}

void curl_download_child(int fd[2], const char* url, const int debug) {
		close(fd[0]); // close read-end

		FILE *fw = fdopen(fd[1], "wb");

		if ( fw == NULL ) {
			fputs("Could not open pipe for writing.\n", stderr);
			exit(1);
		}

		curl_global_init(CURL_GLOBAL_ALL);
		atexit(curl_global_cleanup);

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url);

		if ( debug )
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); // fail silently
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // redirects

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		fclose(fw);
		close(fd[1]); // close write-end
}

// Return read-only file-descriptor that must be closed.
int curl_download(const char* url, const int debug) {
	int pid, fd[2];

	if ( pipe(fd) != 0 ) {
		fputs("Could not create pipe\n", stderr);
		exit(1);
	}

	if ( (pid = fork()) == 0 ) {
		curl_download_child(fd, url, debug);
		exit(0);
	} else if ( pid < 0 ) {
		fputs("Could not fork.\n", stderr);
		exit(1);
	}

	// PARENT process

	close(fd[1]); // close write end of pipe
	return fd[0];
}

#endif
