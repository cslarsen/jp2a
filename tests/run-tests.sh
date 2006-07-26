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

if test "`echo $(UNAME) | cut -c6`" == "CYGWIN" ; then
	JP=../src/jp2a.exe
fi

## INITIALIZE VARS
RESULT_OK=0
RESULT_FAILED=0

function test_ok() {
	echo -e "\e[1mOK\e[0m"
	RESULT_OK=$((RESULT_OK + 1))
}

function test_failed() {
	echo -e "\e[1mFAILED\e[0m (check result against ${1})"
	RESULT_FAILED=$((RESULT_FAILED + 1))
}

function test() {
	CMD="${JP} ${1}"
	echo -n "test ($((RESULT_OK+RESULT_FAILED+1))) '${CMD}' ... "
	eval ${CMD} | diff --brief --strip-trailing-cr - ${2} 1>/dev/null && test_ok || test_failed ${2}
}

function test_results() {
	echo ""
	echo "TEST RESULTS FOR JP2A"
	echo ""
	echo "Total tests : $((RESULT_OK + RESULT_FAILED))"
	echo "OK tests    : $((RESULT_OK)) ($((100*RESULT_OK/(RESULT_OK+RESULT_FAILED)))%)"
	echo "Failed tests: $((RESULT_FAILED)) ($((100*RESULT_FAILED/(RESULT_OK+RESULT_FAILED)))%)"
}

echo "-------------------------------------------------------------"
echo " TESTING JP2A BUILD"
echo " "
echo " Note that the output may vary a bit on different platforms,"
echo " so some tests may fail.  This does not mean that jp2a is"
echo " completely broken."
echo "-------------------------------------------------------------"
echo ""

test "--width=78 jp2a.jpg" normal.txt
test "-b --width=78 jp2a.jpg" normal-b.txt
test "--width=160x49 jp2a.jpg" 160x49.txt
test "--height=10 jp2a.jpg" 10h.txt
test "--size=40x40 jp2a.jpg" 40x40.txt
test "--size=1x1 --invert jp2a.jpg" 1x1-inv.txt
test "-i -b --width=110 --height=30 jp2a.jpg" 110x30-i-b.txt
test "--width=78 --flipx --flipy --invert jp2a.jpg" flip-xy-invert.txt
test "--width=78 -b jp2a.jpg jp2a.jpg" 2xnormal-b.txt
test "--verbose --width=78 jp2a.jpg 2>&1 | tr -d '\r'" normal-verbose.txt

TEMPFILE=`mktemp /tmp/jp2a-test-XXXXXX`
test "--width=78 jp2a.jpg --output=${TEMPFILE} && cat ${TEMPFILE}" normal.txt
rm -f ${TEMPFILE}

test "--width=78 jp2a.jpg --clear" normal-clear.txt
test "logo-40x25-gray.jpg --height=30" logo-30.txt
test "--size=80x50 --html --html-fontsize=7 jp2a.jpg" logo.html
test "grind.jpg -i --size=80x30" grind.txt
test "grind.jpg -i --size=80x30 --red=1.0 --green=0.0 --blue=0.0" grind-red.txt
test "grind.jpg -i --size=80x30 --red=0.0 --green=1.0 --blue=0.0" grind-green.txt
test "grind.jpg -i --size=80x30 --red=0.0 --green=0.0 --blue=1.0" grind-blue.txt
test "--width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-normal.txt
test "--invert --width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-invert.txt
test "--invert --size=80x49 dalsnuten-640x480-gray-low.jpg" dalsnuten-80x49-inv.txt
test "dalsnuten-640x480-gray-low.jpg --width=128 --html --html-fontsize=8" dalsnuten-256.html
test "--size=2000x2000 dalsnuten-640x480-gray-low.jpg jp2a.jpg | tr -d '\r' | wc -c | tr -d ' '" dalsnuten-jp2a-2000x2000-md5.txt
test "dalsnuten-640x480-gray-low.jpg --size=80x25 --invert --border --size=150x45" dalsnuten-640x480-gray-low.txt
test "2>/dev/null ; cat jp2a.jpg | ${JP} --width=78 -" normal.txt
test "2>/dev/null ; cat jp2a.jpg | ${JP} - --width=40 --height=40" 40x40.txt
test "--size=454x207 http://jp2a.sourceforge.net/jp2a.jpg" normal-curl.txt
test "--size=454x207 http://jp2a.sf.net/jp2a.jpg" normal-curl.txt

test_results

