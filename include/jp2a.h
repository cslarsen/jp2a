/*
 * Copyright (C) 2006 Christian Stigen Larsen, http://csl.sublevel3.org
 * Distributed under the GNU General Public License (GPL) v2 or later.
 *
 * Project homepage on http://jp2a.sf.net
 *
 * $Id: jp2a.c 163 2006-07-13 11:27:28Z csl $
 */

#ifdef FEAT_CURL
extern int is_url(const char* s);
extern int curl_download(const char* url, const int debug);
#endif
