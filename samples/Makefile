.SUFFIXES: .cpp .o

LINKOPTS = -L../SQUIRREL3/lib -lsquirrel -lsqstdlib

all: simple klass

simple: simple.o
	clang++ -o $@ $^ $(LINKOPTS)

klass: klass.o
	clang++ -o $@ $^ $(LINKOPTS)

.cpp.o:
	clang++ -std=c++11 -I../SQUIRREL3/include -c -o $@ $^ 

clean:
	rm -f simple.o klass.o
	rm -f simple klass

