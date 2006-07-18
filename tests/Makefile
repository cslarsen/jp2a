# $Id$

JP = ../src/jp2a
DIFF = diff --brief --strip-trailing-cr -

# For renewing, uncomment this
#DIFF = cat - >

test: intro test-normal test-curl 
	@echo -e "\nALL TESTS OK"

test-normal: normal \
normal-b \
160x49 \
10h \
40x40 \
1x1-inv \
110x30-i-b \
flip-xy-invert \
2xnormal-b \
normal-verbose \
normal-output \
normal-clear \
logo \
dalsnuten-normal \
dalsnuten-invert \
dalsnuten-80x49-inv \
dalsnuten-256-html \
dalsnuten-jp2a-2000x2000 \
dalsnuten-640x480-low \
stdin-normal \
stdin-40x40

test-curl: curl-simple curl-redir

intro:
	@echo
	@echo "NOTE: The output may vary on different platforms, so this test"
	@echo "      may break.  Check the results against the text-files to"
	@echo "      see for yourself if jp2a is completely broken.  If not,"
	@echo "      you can safely install it."
	@echo
	@echo TESTING VARIOUS JP2A RUNS

normal:
	$(JP) jp2a.jpg | $(DIFF) normal.txt

normal-output:
	$(JP) jp2a.jpg --output=/tmp/jp2a-test.txt && diff --brief /tmp/jp2a-test.txt normal.txt
	rm -f /tmp/jp2a-test.txt

normal-clear:
	$(JP) jp2a.jpg --clear | $(DIFF) normal-clear.txt

normal-b:
	$(JP) -b jp2a.jpg | $(DIFF) normal-b.txt

dalsnuten-jp2a-2000x2000:
	$(JP) --size=2000x2000 dalsnuten-640x480-gray-low.jpg jp2a.jpg | tr -d '\r' | wc -c | tr -d ' ' | $(DIFF) dalsnuten-jp2a-2000x2000-md5.txt

dalsnuten-normal:
	$(JP) dalsnuten-640x480-gray-low.jpg | $(DIFF) dalsnuten-normal.txt

dalsnuten-invert:
	$(JP) --invert dalsnuten-640x480-gray-low.jpg | $(DIFF) dalsnuten-invert.txt

dalsnuten-80x49-inv:
	$(JP) --invert --size=80x49 dalsnuten-640x480-gray-low.jpg | $(DIFF) dalsnuten-80x49-inv.txt

dalsnuten-256-html:
	$(JP) dalsnuten-640x480-gray-low.jpg --width=128 --html --html-fontsize=8 | $(DIFF) dalsnuten-256.html

dalsnuten-640x480-low:
	$(JP) dalsnuten-640x480-gray-low.jpg --size=80x25 --invert --border --size=150x45 | $(DIFF) dalsnuten-640x480-gray-low.txt

2xnormal-b:
	$(JP) -b jp2a.jpg jp2a.jpg | $(DIFF) 2xnormal-b.txt

stdin-normal:
	@echo -e "\nTESTING STANDARD INPUT"
	cat jp2a.jpg | $(JP) - | $(DIFF) normal.txt

stdin-40x40:
	cat jp2a.jpg | $(JP) - --width=40 --height=40 | $(DIFF) 40x40.txt

normal-verbose:
	$(JP) --verbose jp2a.jpg 2>&1 | tr -d '\r' | $(DIFF) normal-verbose.txt

160x49:
	$(JP) --width=160x49 jp2a.jpg | $(DIFF) 160x49.txt

10h:
	$(JP) --height=10 jp2a.jpg | $(DIFF) 10h.txt

40x40:
	$(JP) --size=40x40 jp2a.jpg | $(DIFF) 40x40.txt

1x1-inv:
	$(JP) --size=1x1 --invert jp2a.jpg | $(DIFF) 1x1-inv.txt

110x30-i-b:
	$(JP) -i -b --width=110 --height=30 jp2a.jpg | $(DIFF) 110x30-i-b.txt

logo:
	$(JP) --size=80x50 --html --html-fontsize=7 jp2a.jpg | $(DIFF) logo.html

flip-xy-invert:
	$(JP) --flipx --flipy --invert jp2a.jpg | $(DIFF) flip-xy-invert.txt

dalsnuten-640x480-gray-low:
	$(JP) --invert dalsnuten-640x480-gray-low.jpg | $(DIFF) dalsnuten-640x480-gray-low.txt

curl-simple:
	@echo -e "\nALL TESTS OK SO FAR.  IF YOU HAVE COMPILED JP2A WITHOUT CURL, "
	@echo -e "THE FOLLOWING TEST WILL FAIL.\n"
	@echo -e "TESTING CURL DOWNLOADS"
	$(JP) --size=454x207 http://jp2a.sourceforge.net/jp2a.jpg | $(DIFF) normal-curl.txt

curl-redir:
	$(JP) --size=454x207 http://jp2a.sf.net/jp2a.jpg | $(DIFF) normal-curl.txt
