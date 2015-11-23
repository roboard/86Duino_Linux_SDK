IFLAGS    = -I./cores
OBJFILES  = io.o irq.o err.o wiring_digital.o wiring_analog.o mcm.o wiring_pulse.o wiring_shift.o wdt.o main.o wiring.o WMath.o WString.o Stream.o Print.o vortex86.o queue.o IPAddress.o i2c.o i2cex.o OSAbstract.o
SRCFILES  = test.o
EXEFILES  = test.exe
# -lstdcxx is not needed if using gxx/g++ to link. 
LIBFILES  = -lstdc++ -lrt
#LIBFILES  =
OPTIONS   = -Wno-write-strings -trigraphs
# GNU Make will set CXX to correct C++ compiler if it is not set.
# You can use other compiler from command line of make, ex:
# make CXX=gcc
# CXX       = i586-pc-msdosdjgpp-gcc
CXX = gcc

.PHONY : everything all clean

everything : $(OBJFILES) $(EXEFILES)

all : clean everything

clean :
	-rm -f $(EXEFILES) $(OBJFILES) $(SRCFILES)

%.o: %.cpp
	$(CXX) -c $< $(IFLAGS) $(OPTIONS)

%.o: cores/%.cpp
	$(CXX) -c $< $(IFLAGS) $(OPTIONS)

test.exe : $(SRCFILES) $(OBJFILES)
	$(CXX) -o $@ test.o $(OBJFILES) $(LIBFILES) $(OPTIONS)
