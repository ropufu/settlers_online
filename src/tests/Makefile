
PNAME = tests.out

CC = g++-8

CFLAGS = -std=c++17 -Wall

PATHINC = -I./../../../aftermath/src -I./../../../opensource -I./..

PATHLIB =

LDFLAGS =

.VPATH: .

$(PNAME): $(patsubst %.cpp, %.o, $(wildcard *.cpp))
	$(CC) $^ -o $@ $(PATHLIB) $(LDFLAGS) $(LDLIBS)
	rm -rf *.o *.d

%.o: %.cpp
	$(CC) $< $(CFLAGS) -c -MD $(PATHINC)

include $(wildcard *.d)

.PHONY: clean

clean:
	rm -rf *.o *.d

