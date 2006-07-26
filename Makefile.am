# $Id$

VERSION = @VERSION@
SUBDIRS = src
man_MANS = man/jp2a.1
EXTRA_DIST = tests/*.txt tests/*.html tests/*.jpg tests/run-tests.sh tests/Makefile include/*.h man/jp2a.1 LICENSES

test: all
	cd tests/; make test

man/jp2a.html: man/jp2a.1
	groff -T html -man $< > $@

dist-win32: jp2a-$(VERSION)-win32-bin.zip

jp2a-$(VERSION)-win32-bin.zip: win32-dist/jp2a.exe man/jp2a.html
	upx $< || exit 0
	rm -f $@
	zip $@ AUTHORS COPYING LICENSES ChangeLog INSTALL NEWS README man/jp2a.html win32-dist/*

test-curl: all
	cd tests/; make test-curl

test-normal: all
	cd tests/; make test-normal
