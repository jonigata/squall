.SUFFIXES: .cpp .o

LINKOPTS = -L../SQUIRREL3/lib -lsquirrel -lsqstdlib

all: simple export_object klass rawfunc table

simple: simple.o
	clang++ -o $@ $^ $(LINKOPTS)

export_object: export_object.o
	clang++ -o $@ $^ $(LINKOPTS)

klass: klass.o
	clang++ -o $@ $^ $(LINKOPTS)

rawfunc: rawfunc.o
	clang++ -o $@ $^ $(LINKOPTS)

table: table.o
	clang++ -o $@ $^ $(LINKOPTS)

.cpp.o:
	clang++ -std=c++11 -I../SQUIRREL3/include -c -o $@ $^ 

clean:
	rm -f simple.o export_object.o klass.o rawfunc.o table.o
	rm -f simple export_object klass rawfunc table

rebuild:
	make clean && make
