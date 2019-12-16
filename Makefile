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

ifndef TESTDIR
    TESTDIR=test
endif

ifndef PRJDIR
    PRJDIR=.
endif

# 强耦合，底层修改，这里也会生效
DST=dst-all 
TEST=test-all

all: $(DST) $(TEST)

include $(TESTDIR)/Makefile
test-all:
	make test-all-sub

include $(SRCDIR)/Makefile
dst-all:
	make dst-all-sub

.PHONY : clean
clean:
	rm -f $(SRCDIR)/*.o $(DST) $(TEST) $(PRJDIR)/test/*.o

.PHONY : clean_data
clean_data:
	rm -f data/* psnr/data/*