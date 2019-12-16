# TESTMAKEFILE
ifndef TESTMAKEFILE

include third_party/Makefile
include libs/Makefile

# 解耦合用，顶层make all未使用，可单独使用 make test-all
TESTSUB=test_matrixutils test_psnr test_httprequest

test-all-sub: $(TESTSUB)

test_matrixutils: $(PRJDIR)/test/test_matrixutils.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test_psnr: $(PRJDIR)/test/test_psnr.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test_httprequest: $(PRJDIR)/test/test_httprequest.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# build.
$(PRJDIR)/test/test_httprequest.o: 
	$(CXX) $(CXXFLAGS) -c -o $(PRJDIR)/test/test_httprequest.o $(PRJDIR)/test/test_httprequest.cpp $(INCLUDES)

$(PRJDIR)/test/test_matrixutils.o: 
	$(CXX) $(CXXFLAGS) -c -o $(PRJDIR)/test/test_matrixutils.o $(PRJDIR)/test/test_matrixutils.cpp $(INCLUDES)

$(PRJDIR)/test/test_psnr.o: 
	$(CXX) $(CXXFLAGS) -c -o $(PRJDIR)/test/test_psnr.o $(PRJDIR)/test/test_psnr.cpp $(INCLUDES)

endif