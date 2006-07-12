# $Id$

all: intro normal normal-b 160x49 10h 40x40 1x1-inv 110x30-i-b flip-xy-invert 2xnormal-b normal-verbose stdin-normal stdin-40x40 web-normal
	@echo -e "\nALL TESTS OK."

intro:
	@echo TESTING VARIOUS JP2A OPTIONS

normal:
	../jp2a jp2a.jpg | diff - normal.txt

normal-b:
	../jp2a -b jp2a.jpg | diff - normal-b.txt

2xnormal-b:
	../jp2a -b jp2a.jpg jp2a.jpg | diff - 2xnormal-b.txt

stdin-normal:
	@echo -e "\nTESTING STANDARD INPUT"
	cat jp2a.jpg | ../jp2a - | diff - normal.txt

stdin-40x40:
	cat jp2a.jpg | ../jp2a - --width=40 --height=40 | diff - 40x40.txt

normal-verbose:
	../jp2a --verbose jp2a.jpg 2>&1 | diff - normal-verbose.txt

160x49:
	../jp2a --width=160x49 jp2a.jpg | diff - 160x49.txt

10h:
	../jp2a --height=10 jp2a.jpg | diff - 10h.txt

40x40:
	../jp2a --size=40x40 jp2a.jpg | diff - 40x40.txt

1x1-inv:
	../jp2a --size=1x1 --invert jp2a.jpg | diff - 1x1-inv.txt

110x30-i-b:
	../jp2a -i -b --width=110 --height=30 jp2a.jpg | diff - 110x30-i-b.txt

big:
	../jp2a --size=1024x768 --html --html-fontsize=2 jp2a.jpg | diff - big.html

flip-xy-invert:
	../jp2a --flipx --flipy --invert jp2a.jpg | diff - flip-xy-invert.txt

web-normal:
	@echo -e "\nTESTING CURL DOWNLOADS"
	../jp2a http://jp2a.sourceforge.net/jp2a.jpg | diff - normal.txt
