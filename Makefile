
CXX=g++ -Wall
CXXFLAGS=-O3
CXXFLAGS_DEBUG=-g

SRC=hemeXtract.cc

hemeXtract: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

debug: $(SRC)
	$(CXX) $(CXXFLAGS_DEBUG) -o $@ $<

.phony: clean
clean:
	rm -f hemeXtract