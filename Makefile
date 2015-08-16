# To build the project
# 1. Edit settings below
# 2. run 'make dep' 
# 3. run 'make'

# C++ compiler
CPP=mpicxx
# C++ options
CPPFLAGS= --std=c++11 -g
# Link options
LDFLAGS=-pthread -static
# BNB-Solver folder
BNBROOT=../BNB-solver

# Do not edit below this line
INCPATH=-I$(BNBROOT)
LIBS=$(BNBROOT)/util/common/util.a $(BNBROOT)/problems/optlib/optlib.a $(BNBROOT)/libjson/libjson.a
COMPFLAGS=$(INCPATH) $(CPPFLAGS)
LINKFLAGS=$(LDFLAGS) $(LIBS)

	
bnbdg.exe : bnbdg.o
	$(CPP) -o bnbdg.exe bnbdg.o $(LINKFLAGS) 

-include deps.inc

clean: 
	rm *.o *.exe deps.inc

dep:
	$(CPP) $(COMPFLAGS) -MM -c *.cpp > deps.inc

.cpp.o:
	$(CPP) $(COMPFLAGS) -c $<

