# $Id$

JP = ../src/jp2a

all: intro normal normal-b 160x49 10h 40x40 1x1-inv 110x30-i-b flip-xy-invert 2xnormal-b normal-verbose stdin-normal stdin-40x40 web-normal
	@echo -e "\nALL TESTS OK."

intro:
	@echo TESTING VARIOUS JP2A OPTIONS

normal:
	$(JP) jp2a.jpg | diff - normal.txt

normal-b:
	$(JP) -b jp2a.jpg | diff - normal-b.txt

2xnormal-b:
	$(JP) -b jp2a.jpg jp2a.jpg | diff - 2xnormal-b.txt

stdin-normal:
	@echo -e "\nTESTING STANDARD INPUT"
	cat jp2a.jpg | $(JP) - | diff - normal.txt

stdin-40x40:
	cat jp2a.jpg | $(JP) - --width=40 --height=40 | diff - 40x40.txt

normal-verbose:
	$(JP) --verbose jp2a.jpg 2>&1 | diff - normal-verbose.txt

160x49:
	$(JP) --width=160x49 jp2a.jpg | diff - 160x49.txt

10h:
	$(JP) --height=10 jp2a.jpg | diff - 10h.txt

40x40:
	$(JP) --size=40x40 jp2a.jpg | diff - 40x40.txt

1x1-inv:
	$(JP) --size=1x1 --invert jp2a.jpg | diff - 1x1-inv.txt

110x30-i-b:
	$(JP) -i -b --width=110 --height=30 jp2a.jpg | diff - 110x30-i-b.txt

big:
	$(JP) --size=1024x768 --html --html-fontsize=2 jp2a.jpg | diff - big.html

flip-xy-invert:
	$(JP) --flipx --flipy --invert jp2a.jpg | diff - flip-xy-invert.txt

web-normal:
	@echo -e "\nTESTING CURL DOWNLOADS"
	$(JP) http://jp2a.sourceforge.net/jp2a.jpg | diff - normal.txt
