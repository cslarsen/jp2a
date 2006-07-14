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

#ifdef WIN32
#include <windows.h>
#include <process.h>
#endif

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

// local variables to curl.c
int fd[2], debugopt;
const char* URL;

//! Return 1 if s is a supported URL
int is_url(const char* s) {
	return !strncmp(s, "ftp://", 6)
		| !strncmp(s, "ftps://", 7)
		| !strncmp(s, "file://", 7)
		| !strncmp(s, "http://", 7)
		| !strncmp(s, "tftp://", 7)
		| !strncmp(s, "https://", 8);
}

void curl_download_child() {
		close(fd[0]); // close read-end

		FILE *fw = fdopen(fd[1], "wb");

		if ( fw == NULL ) {
			fputs("Could not open pipe for writing.\n", stderr);
			exit(1);
		}

		curl_global_init(CURL_GLOBAL_ALL);
		atexit(curl_global_cleanup);

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, URL);

		if ( debugopt )
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); // fail silently
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // redirects

		curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		fclose(fw);
		close(fd[1]); // close write-end

#ifdef WIN32
		_endthread();
#endif
}

// Return read-only file-descriptor that must be closed.
int curl_download(const char* url, const int debug) {

	URL = url;
	debugopt = debug;

	if ( pipe(fd) != 0 ) {
		fputs("Could not create pipe\n", stderr);
		exit(1);
	}

#ifndef WIN32
	int pid;

	if ( (pid = fork()) == 0 ) {
		// CHILD process
		curl_download_child();
		exit(0);
	} else if ( pid < 0 ) {
		fputs("Could not fork.\n", stderr);
		exit(1);
	}

#else
	uintptr_t dlthread = _beginthread(curl_download_child, 0, NULL);
#endif

	// PARENT process

	close(fd[1]); // close write end of pipe
	return fd[0];
}

#endif
