
PNAME = black_marsh.out

CC = g++-7

CFLAGS = -std=c++14 -Wall

PATHINC = -I./../../../aftermath -I./../../../opensource -I./..

PATHLIB =

LDFLAGS = -lstdc++fs

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

