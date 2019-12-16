# include the third-party requirements
include third_party/Makefile
# 项目封装的libs需要在libs/Makefile中注册，以供全局使用
include libs/Makefile

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

# 命令定义在子一级，调度在顶层，如TEST的test_httprequest定义在test/Makefile，调度在此(顶层和子一级的Makefile解耦合)
DST=get_frame_seq checkdropframe vpsnr
TEST=test_httprequest test_matrixutils test_psnr  # 解耦合方式
# TEST=test-all                                   # 耦合的方式，暂时不用这种

# 逐渐将顶层Makefile具体编译逻辑抽出，只做聚合与调度
all: $(DST) $(TEST)

include test/Makefile # 测试的makefile，至少在all之后
test: $(TEST)
test-all:
	make test-all-sub
tpsnr: test_psnr

include $(SRCDIR)/Makefile

.PHONY : clean
# clean
clean:
	rm -f $(SRCDIR)/*.o $(DST) $(TEST) $(PRJDIR)/test/*.o

.PHONY : clean_data
clean_data:
	rm -f data/* psnr/data/*