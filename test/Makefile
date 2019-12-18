# TESTMAKEFILE
ifndef TESTMAKEFILE

include third_party/Makefile
include libs/Makefile

# 底层增加，删除后请在此修改，顶层不用修改
TESTSUB=test_matrixutils test_httprequest

# 顶层调用的入口
test-all-sub: $(TESTSUB)

# 没放在TESTSUB里面，make时不会执行该order，单可在外面单独 make tpsnr，
tpsnr: test_psnr

# 放在TESTSUB里面
test_matrixutils: $(PRJDIR)/test/test_matrixutils.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# 放在TESTSUB里面
test_psnr: $(PRJDIR)/test/test_psnr.o $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# 放在TESTSUB里面
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