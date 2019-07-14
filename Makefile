# set the complier.
ifndef CXX
    CXX=g++
endif
 
ifeq (c++, $(findstring c++,$(CXX)))
	CXX=g++
endif

# set the compiler's flags.
ifndef CXXFLAGS
    CXXFLAGS=-O2 -DNDEBUG -Wall -Wno-sign-compare -g 
endif

ifeq (g++, $(findstring g++,$(CXX)))
    override CXXFLAGS += -std=c++11
else ifeq (clang++, $(findstring clang++,$(CXX)))
    override CXXFLAGS += -std=c++11
else ifeq ($(CXX), c++)
    ifeq ($(shell uname -s), Darwin)
        override CXXFLAGS += -std=c++11
    endif
endif

# set the third-party requirements
ifdef LIBS
    LIBS += $(shell pkg-config --libs opencv yaml-cpp)
else
    LIBS=$(shell pkg-config --libs opencv yaml-cpp)
endif

# set the third-party requirements
ifdef INCLUDES
    INCLUDES += $(shell pkg-config --cflags opencv yaml-cpp)
else
    INCLUDES=$(shell pkg-config --cflags opencv yaml-cpp)
endif

# set the PREFIEX
ifndef PREFIX
    PREFIX=.
endif

ifndef SRCDIR
    SRCDIR=src
endif

LIBOBJ = $(SRCDIR)/get_frame_seq.o

get_frame_seq: $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)


# clean
clean:
	rm -f $(SRCDIR)/*.o get_frame_seq data/*

# build.
$(SRCDIR)/get_frame_seq.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/get_frame_seq.o $(SRCDIR)/get_frame_seq.cpp $(INCLUDES)


