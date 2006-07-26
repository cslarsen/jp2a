#!/bin/bash

#####
#
# $Id
#
# Copyright (C) 2006 Christian Stigen Larsen
#
# This is a small script to test if jp2a has been correctly built.
# See http://jp2a.sf.net for project information.
#
#####

## PATH TO EXECUTABLE jp2a
JP=../src/jp2a
UNAME=`uname -a`

if test "`echo $(UNAME) | cut -c1-6`" == "CYGWIN" ; then
	JP=../win32-dist/jp2a.exe
fi

## INITIALIZE VARS
RESULT_OK=0
RESULT_FAILED=0
FAILED_STR=""

function test_ok() {
	echo -e -n "\e[1mOK\e[0m"
	RESULT_OK=$((RESULT_OK + 1))
}

function test_failed() {
	echo -e -n "\e[1mFAILED\e[0m"
	RESULT_FAILED=$((RESULT_FAILED + 1))
	FAILED_STR="${FAILED_STR}\n${2} | diff --strip-trailing-cr - ${1}"
}

function test_jp2a() {
	CMD="${JP} ${2}"
	echo -n "test ($((RESULT_OK+RESULT_FAILED+1))) (${1}) ... "
	eval ${CMD} | diff --brief --strip-trailing-cr - ${3} 1>/dev/null && test_ok || test_failed ${3} "${CMD}"
	echo ""
}

function test_results() {
	echo ""
	echo "TEST RESULTS FOR JP2A"
	echo ""
	echo "Total tests : $((RESULT_OK + RESULT_FAILED))"
	echo "OK tests    : $((RESULT_OK)) ($((100*RESULT_OK/(RESULT_OK+RESULT_FAILED)))%)"
	echo "Failed tests: $((RESULT_FAILED)) ($((100*RESULT_FAILED/(RESULT_OK+RESULT_FAILED)))%)"
	echo ""

	if test "x${FAILED_STR}" != "x" ; then
		echo "Summary of failed tests:"
		echo -e "${FAILED_STR}"
	fi
}

echo "-------------------------------------------------------------"
echo " TESTING JP2A BUILD"
echo " "
echo " Note that the output may vary a bit on different platforms,"
echo " so some tests may fail.  This does not mean that jp2a is"
echo " completely broken."
echo "-------------------------------------------------------------"
echo ""

test_jp2a "width" "--width=78 jp2a.jpg" normal.txt
test_jp2a "border, width" "-b --width=78 jp2a.jpg" normal-b.txt
test_jp2a "size" "--size=160x49 jp2a.jpg" 160x49.txt
test_jp2a "height" "--height=10 jp2a.jpg" 10h.txt
test_jp2a "size" "--size=40x40 jp2a.jpg" 40x40.txt
test_jp2a "size" "--size=1x1 --invert jp2a.jpg" 1x1-inv.txt
test_jp2a "invert, border" "-i -b --width=110 --height=30 jp2a.jpg" 110x30-i-b.txt
test_jp2a "width, flip, invert" "--width=78 --flipx --flipy --invert jp2a.jpg" flip-xy-invert.txt
test_jp2a "width, border" "--width=78 -b jp2a.jpg jp2a.jpg" 2xnormal-b.txt
test_jp2a "width, verbose" "--verbose --width=78 jp2a.jpg 2>&1 | tr -d '\r'" normal-verbose.txt

TEMPFILE=`mktemp /tmp/jp2a-test-XXXXXX`
test_jp2a "width, outfile" "--width=78 jp2a.jpg --output=${TEMPFILE} && cat ${TEMPFILE}" normal.txt
rm -f ${TEMPFILE}

test_jp2a "width, clear" "--width=78 jp2a.jpg --clear" normal-clear.txt
test_jp2a "height, grayscale" "logo-40x25-gray.jpg --height=30" logo-30.txt
test_jp2a "size, html" "--size=80x50 --html --html-fontsize=7 jp2a.jpg" logo.html
test_jp2a "size, invert" "grind.jpg -i --size=80x30" grind.txt
test_jp2a "size, invert, red channel" "grind.jpg -i --size=80x30 --red=1.0 --green=0.0 --blue=0.0" grind-red.txt
test_jp2a "size, invert, blue channel" "grind.jpg -i --size=80x30 --red=0.0 --green=1.0 --blue=0.0" grind-green.txt
test_jp2a "size, invert, green channel" "grind.jpg -i --size=80x30 --red=0.0 --green=0.0 --blue=1.0" grind-blue.txt
test_jp2a "width, grayscale" "--width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-normal.txt
test_jp2a "invert, width, grayscale" "--invert --width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-invert.txt
test_jp2a "invert, size, grayscale" "--invert --size=80x49 dalsnuten-640x480-gray-low.jpg" dalsnuten-80x49-inv.txt
test_jp2a "width, html, grayscale" "dalsnuten-640x480-gray-low.jpg --width=128 --html --html-fontsize=8" dalsnuten-256.html
test_jp2a "big size" "--size=2000x2000 dalsnuten-640x480-gray-low.jpg jp2a.jpg | tr -d '\r' | wc -c | tr -d ' '" dalsnuten-jp2a-2000x2000-md5.txt
test_jp2a "size, invert, border" "dalsnuten-640x480-gray-low.jpg --size=80x25 --invert --border --size=150x45" dalsnuten-640x480-gray-low.txt
test_jp2a "standard input, width" " 2>/dev/null ; cat jp2a.jpg | ${JP} --width=78 -" normal.txt
test_jp2a "standard input, width, height" " 2>/dev/null ; cat jp2a.jpg | ${JP} - --width=40 --height=40" 40x40.txt
test_jp2a "size, curl download" "--size=454x207 http://jp2a.sourceforge.net/jp2a.jpg" normal-curl.txt
test_jp2a "size, curl download" "--size=454x207 http://jp2a.sf.net/jp2a.jpg" normal-curl.txt

test_results
