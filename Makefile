# These directories must include jpeglib.h
# http://www.ijg.org

INCLUDE = /sw/include
LIBDIR = /sw/lib

CFLAGS = -g -O3 -W -Wall -I$(INCLUDE)
CXXFLAGS = -g -O3 -W -Wall -I$(INCLUDE)
LDFLAGS = -L$(LIBDIR) -ljpeg

TARGETS = jp2a jp2a-more

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
