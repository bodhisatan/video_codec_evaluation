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
    LIBS += $(shell pkg-config --libs opencv yaml-cpp libavcodec libavformat libavdevice libavutil)
else
    LIBS=$(shell pkg-config --libs opencv yaml-cpp libavcodec libavformat libavdevice libavutil)
endif

# set the third-party requirements
ifdef INCLUDES
    INCLUDES += $(shell pkg-config --cflags opencv yaml-cpp libavcodec libavformat libavdevice libavutil)
else
    INCLUDES=$(shell pkg-config --cflags opencv yaml-cpp libavcodec libavformat libavdevice libavutil)
endif

# set the PREFIEX
ifndef PREFIX
    PREFIX=.
endif

ifndef SRCDIR
    SRCDIR=src
endif

DST=get_frame_seq checkdropframe vpsnr
TEST=test_httprequest test_matrixutils test_psnr

all: $(DST) $(TEST)

test: $(TEST)

tpsnr: test_psnr

LIBOBJ = $(SRCDIR)/cmdlineutils.o \
         $(SRCDIR)/matrixutils.o \
         $(SRCDIR)/conf.o \
         $(SRCDIR)/ocr.o \
         $(SRCDIR)/frame_drop_detect.o \
		 $(SRCDIR)/psnr.o

get_frame_seq: $(SRCDIR)/get_frame_seq.o $(LIBOBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

vpsnr: $(SRCDIR)/vpsnr.o $(LIBOBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

checkdropframe: $(SRCDIR)/check_dropframe.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test_httprequest: $(SRCDIR)/test/test_httprequest.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test_matrixutils: $(SRCDIR)/test/test_matrixutils.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test_psnr: $(SRCDIR)/test/test_psnr.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# clean
clean:
	rm -f $(SRCDIR)/*.o $(DST) $(TEST) $(SRCDIR)/test/*.o

clean_data:
	rm -f data/* psnr/data/*

# build.
$(SRCDIR)/get_frame_seq.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/get_frame_seq.o $(SRCDIR)/get_frame_seq.cpp $(INCLUDES)

$(SRCDIR)/check_dropframe.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/check_dropframe.o $(SRCDIR)/check_dropframe.cpp $(INCLUDES)

$(SRCDIR)/matrixutils.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/matrixutils.o $(SRCDIR)/matrixutils.cpp $(INCLUDES)

$(SRCDIR)/conf.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/conf.o $(SRCDIR)/conf.cpp $(INCLUDES)

$(SRCDIR)/ocr.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/ocr.o $(SRCDIR)/ocr.cpp $(INCLUDES)

$(SRCDIR)/frame_drop_detect.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/frame_drop_detect.o $(SRCDIR)/frame_drop_detect.cpp $(INCLUDES)

$(SRCDIR)/psnr.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/psnr.o $(SRCDIR)/psnr.cpp $(INCLUDES)

$(SRCDIR)/vpsnr.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/vpsnr.o $(SRCDIR)/vpsnr.cpp $(INCLUDES)

$(SRCDIR)/test/test_httprequest.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/test/test_httprequest.o $(SRCDIR)/test/test_httprequest.cpp $(INCLUDES)

$(SRCDIR)/test/test_matrixutils.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/test/test_matrixutils.o $(SRCDIR)/test/test_matrixutils.cpp $(INCLUDES)

$(SRCDIR)/test/test_psnr.o: 
	$(CXX) $(CXXFLAGS) -c -o $(SRCDIR)/test/test_psnr.o $(SRCDIR)/test/test_psnr.cpp $(INCLUDES)

