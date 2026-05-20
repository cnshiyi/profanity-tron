CC ?= g++
CDEFINES =
SOURCES = Dispatcher.cpp Mode.cpp precomp.cpp profanity.cpp SpeedSample.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = profanity.x64

ifeq ($(OS),Windows_NT)
	LDFLAGS = OpenCL.lib bcrypt.lib
	CFLAGS = -c -std=c++11 -Wall -O2 -I./OpenCL/include
	CLEAN = del /Q *.o *.obj $(EXECUTABLE).exe 2>nul || exit 0
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework OpenCL
		CFLAGS = -c -std=c++11 -Wall -mmmx -O2
	else
		LDFLAGS = -s -lOpenCL -mcmodel=large
		CFLAGS = -c -std=c++11 -Wall -mmmx -O2 -mcmodel=large
	endif
	CLEAN = rm -rf *.o $(EXECUTABLE)
endif

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(CDEFINES) $< -o $@

clean:
	$(CLEAN)

