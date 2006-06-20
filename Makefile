# These directories must include jpeglib.h
# http://www.ijg.org

INCLUDE = /sw/include
LIBDIR = /sw/lib

CXXFLAGS = -g -W -Wall -I$(INCLUDE)
LDFLAGS = -L$(LIBDIR) -ljpeg

TARGETS = jp2a

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
