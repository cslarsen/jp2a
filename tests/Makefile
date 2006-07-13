# $Id$

JP = ../src/jp2a
DIFF = diff --brief -

# For renewing, uncomment this
#DIFF = cat - >

test: intro \
normal \
normal-b \
160x49 \
10h \
40x40 \
1x1-inv \
110x30-i-b \
flip-xy-invert \
2xnormal-b \
normal-verbose \
fry-normal \
fry-invert \
fry-80x49-inv \
fry-160-html \
fry-jp2a-2000x2000 \
stdin-40x40 \
stdin-normal \
test-curl
	@echo -e "\nALL TESTS OK"

intro:
	@echo
	@echo "NOTE: Some tests may break on your particular platform."
	@echo "      This does not necessarily mean your compiled jp2a"
	@echo "      is broken.  What you DO want to check is that there"
	@echo "      is no segfaults and stuff like that."
	@echo
	@echo TESTING VARIOUS JP2A RUNS

normal:
	$(JP) jp2a.jpg | $(DIFF) normal.txt

normal-b:
	$(JP) -b jp2a.jpg | $(DIFF) normal-b.txt

fry-jp2a-2000x2000:
	$(JP) --size=2000x2000 fry.jpg jp2a.jpg | $(DIFF) fry-jp2a-2000x2000.txt

fry-normal:
	$(JP) fry.jpg | $(DIFF) fry-normal.txt

fry-invert:
	$(JP) --invert fry.jpg | $(DIFF) fry-invert.txt

fry-80x49-inv:
	$(JP) --invert --size=80x49 fry.jpg | $(DIFF) fry-80x49-inv.txt

fry-160-html:
	$(JP) fry.jpg --width=80 --html --html-fontsize=8 | $(DIFF) fry-160.html

2xnormal-b:
	$(JP) -b jp2a.jpg jp2a.jpg | $(DIFF) 2xnormal-b.txt

stdin-normal:
	@echo -e "\nTESTING STANDARD INPUT"
	cat jp2a.jpg | $(JP) - | $(DIFF) normal.txt

stdin-40x40:
	cat jp2a.jpg | $(JP) - --width=40 --height=40 | $(DIFF) 40x40.txt

normal-verbose:
	$(JP) --verbose jp2a.jpg 2>&1 | $(DIFF) normal-verbose.txt

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

big:
	$(JP) --size=1024x768 --html --html-fontsize=2 jp2a.jpg | $(DIFF) big.html

flip-xy-invert:
	$(JP) --flipx --flipy --invert jp2a.jpg | $(DIFF) flip-xy-invert.txt

test-curl:
	@echo -e "\nALL TESTS SO FAR OK.  IF YOU HAVENT COMPILED JP2A WITH CURL,"
	@echo -e "THEN THE FOLLOWING TEST WILL FAIL.\n"
	@echo -e "TESTING CURL DOWNLOADS"
	$(JP) --size=78x18 http://jp2a.sourceforge.net/jp2a.jpg | $(DIFF) normal-curl.txt
