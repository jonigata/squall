.SUFFIXES: .cpp .o

TARGET = t
OBJS = main.o

$(TARGET): $(OBJS)
	clang++ -o $(TARGET) $(OBJS) -L../SQUIRREL3/lib -lsquirrel -lsqstdlib

.cpp.o:
	clang++ -std=c++11 -I../SQUIRREL3/include -c -o $@ $^ 

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

