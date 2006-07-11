# $Id$

all: normal normal-b 160x49 10h 40x40 1x1-inv 110x30-i-b flip-xy-invert web-normal
	@echo All tests ok.

normal:
	../jp2a jp2a.jpg | diff - normal.txt

normal-b:
	../jp2a -b jp2a.jpg | diff - normal-b.txt

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
	../jp2a http://jp2a.sourceforge.net/jp2a.jpg | diff - normal.txt
