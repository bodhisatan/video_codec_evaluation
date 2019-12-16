include third_party/Makefile
include libs/Makefile

TEST=test_httprequest test_matrixutils test_psnr

other-all:
	@echo "other makefile begin"
	@echo $(LIBS)
	@echo $(PRJDIR)
	@echo $(DEBUG)
	@echo "other makefile end"

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