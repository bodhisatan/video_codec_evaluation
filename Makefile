# configuration
DEBUG ?= 0     # 0: -DNDEBUG   1:-DDEBUG

# set the complier.
ifndef CXX
    CXX=g++ 
endif
 
ifeq (c++, $(findstring c++,$(CXX)))
    CXX=g++
endif

# set the compiler's flags.
ifndef CXXFLAGS
    CXXFLAGS=-O2 -Wall -Wno-sign-compare -g
endif

ifeq ($(DEBUG), 1)
    override CXXFLAGS += -DDEBUG
else
    override CXXFLAGS += -DNDEBUG
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

# 第三方库
# include the third-party requirements
include third_party/Makefile

# set the PREFIEX
ifndef PREFIX
    PREFIX=.
endif

ifndef SRCDIR
    SRCDIR=src
endif

ifndef PRJDIR
    PRJDIR=.
endif

DST=get_frame_seq checkdropframe vpsnr
TEST=test_httprequest test_matrixutils test_psnr

all: $(DST) $(TEST)

# include的子一级makefile 放在default的 all之后
# 测试的makefile
include test/Makefile

test: $(TEST)

tpsnr: test_psnr

# 测试makefile拆分使用，验证后删除
# todo: delete makefile_div_debug
makefile_div_debug:
	@echo "main makefile begin"
	@echo "${FLAG}"
	make other-all
	@echo "main makefile end"

# 本项目封装的libs需要在libs/Makefile中注册，以供全局使用
include libs/Makefile

get_frame_seq: $(SRCDIR)/get_frame_seq.o $(LIBOBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

vpsnr: $(SRCDIR)/vpsnr.o $(LIBOBJ) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

checkdropframe: $(SRCDIR)/check_dropframe.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

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

.PHONY : clean
# clean
clean:
	rm -f $(SRCDIR)/*.o $(DST) $(TEST) $(PRJDIR)/test/*.o

.PHONY : clean_data
clean_data:
	rm -f data/* psnr/data/*