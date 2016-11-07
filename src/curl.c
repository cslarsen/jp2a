/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Distributed under the GNU General Public License (GPL) v2.
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

#ifdef WIN32
size_t passthru_write(void *buffer, size_t size, size_t nmemb, void *userp) {
	FILE *f = (FILE*) userp;
	return f!=NULL? fwrite(buffer, size, nmemb, f) : 0;
}
#endif

#ifndef WIN32
void curl_download_child()
#else
void curl_download_child(void*)
#endif
{
	FILE *fw;
	CURL *curl;
#ifndef WIN32
	close(fd[0]); // close read-end
#endif

	if ( (fw = fdopen(fd[1], "wb")) == NULL ) {
		fputs("Could not open pipe for writing.\n", stderr);
		exit(1);
	}

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, URL);

	if ( debugopt )
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1); // fail silently
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // redirects
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
	#ifdef WIN32
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, passthru_write);
	#endif

	curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	fclose(fw);
	close(fd[1]); // close write-end

	curl_global_cleanup();

#ifdef WIN32
	_endthread();
#endif	
}

// Return read-only file-descriptor that must be closed.
int curl_download(const char* url, const int debug) {
#ifndef WIN32
	int pid;
#endif

	URL = url;
	debugopt = debug;

	if ( pipe(fd) != 0 ) {
		fputs("Could not create pipe\n", stderr);
		exit(1);
	}

#ifndef WIN32

	if ( (pid = fork()) == 0 ) {
		// CHILD process
		curl_download_child();
		exit(0);
	} else if ( pid < 0 ) {
		fputs("Could not fork.\n", stderr);
		exit(1);
	}

#else
	if ( _beginthread(curl_download_child, 0, NULL) <= 0 ) {
		fputs("Could not create thread", stderr);
		exit(1);
	}
#endif

	// PARENT process

#ifndef WIN32
	close(fd[1]); // close write end of pipe
#endif

	return fd[0];
}

#endif
