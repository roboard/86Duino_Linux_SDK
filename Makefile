IFLAGS    = -I./cores
OBJFILES  = io.o irq.o err.o wiring_digital.o wiring_analog.o mcm.o wiring_pulse.o wiring_shift.o wdt.o main.o wiring.o WMath.o WString.o Stream.o Print.o vortex86.o queue.o IPAddress.o i2c.o i2cex.o OSAbstract.o interrupt.o
SRCFILES  = 86Duino.o
EXEFILES  = 86Duino
LIBFILES  = -lstdc++ -lrt -lpthread
OPTIONS   = -Wno-write-strings -trigraphs
CXX = gcc

THIRD_LIB_NAME    = TimerOne
THIRD_LIB_INCLUDE = $(addprefix -I./libraries/,$(THIRD_LIB_NAME))
THIRD_LIB_PATH    = $(addprefix libraries/,$(THIRD_LIB_NAME))
THIRD_OBJ         = $(addsuffix .o,$(THIRD_LIB_NAME))
EXTEN_LIB         =

.PHONY : everything all clean

everything : $(OBJFILES) $(EXEFILES)

all : clean everything

clean :
	-rm -f $(EXEFILES) $(OBJFILES) $(THIRD_OBJ) $(SRCFILES)

%.o: %.cpp
	$(CXX) -c $< $(IFLAGS) $(THIRD_LIB_INCLUDE) $(OPTIONS)

%.o: cores/%.cpp
	$(CXX) -c $< $(IFLAGS) $(OPTIONS)

%.o: $(THIRD_LIB_PATH)/%.cpp
	$(CXX) -c $< $(IFLAGS) $(THIRD_LIB_INCLUDE) $(OPTIONS)

86Duino : $(SRCFILES) $(OBJFILES) $(THIRD_OBJ)
	$(CXX) -o $@ 86Duino.o $(OBJFILES) $(THIRD_OBJ) $(LIBFILES) $(OPTIONS)
