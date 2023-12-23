CC=gcc

# Includes
INCLUDES = -I.

# Common flags
COMMONFLAGS += $(INCLUDES)
CFLAGS += -g -Wall   -lm $(COMMONFLAGS)

TARGET = doubleCharErasure

OBJS = galois.o jerasure.o doubleCharErasure.o dcerasure.o measureEC.o

$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

clean:
	rm -rf *.o $(TARGET)


jerasure.o: jerasure.h galois.h
galois.o: galois.h
dcerasure.o: dcerasure.h
measureEC.o: measureEC.h
doubleCharErasure.o: measureEC.h dcerasure.h


