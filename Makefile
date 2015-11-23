IFLAGS    = -I./cores
OBJFILES  = io.o irq.o err.o wiring_digital.o wiring_analog.o mcm.o wiring_pulse.o wiring_shift.o wdt.o main.o wiring.o WMath.o WString.o Stream.o Print.o vortex86.o queue.o IPAddress.o i2c.o i2cex.o OSAbstract.o interrupt.o
SRCFILES  = 86Duino.o
EXEFILES  = 86Duino
LIBFILES  = -lstdc++ -lrt -lpthread
OPTIONS   = -Wno-write-strings -trigraphs
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

86Duino : $(SRCFILES) $(OBJFILES)
	$(CXX) -o $@ 86Duino.o $(OBJFILES) $(LIBFILES) $(OPTIONS)
