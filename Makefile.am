# $Id$

SUBDIRS = src
man_MANS = man/jp2a.1
EXTRA_DIST = tests/*.txt tests/*.html tests/Makefile tests/*.jpg include/*.h man/jp2a.1

test: all
	cd tests/; make test

man/jp2a.html: man/jp2a.1
	nroff -man $< | man2html > $@

jp2a-0.9.17-win32-bin.zip: jp2a.exe
	upx win32-dist/$<
	rm -f $@
	zip $@ AUTHORS COPYING ChangeLog INSTALL NEWS README man/jp2a.html win32-dist/*

test-curl: all
	cd tests/; make test-curl

test-normal:
	cd tests/; make test-normal
