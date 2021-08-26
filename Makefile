TARGET = test_deque.o test_hashmap.o test_hashset.o
CC := g++
CFLAGS = -lm -Wall -g

.PHONY:
all: $(TARGET)

test_deque.o : test_deque.cpp
	$(CC) $(CFLAGS) test_deque.cpp -o test_deque.o

test_hashmap.o : test_hashmap.cpp
	$(CC) $(CFLAGS) test_hashmap.cpp -o test_hashmap.o

test_hashset.o : test_deque.cpp
	$(CC) $(CFLAGS) test_deque.cpp -o test_hashset.o

.PHONY:
clean:
	rm -rf *.o