TARGET = httpclient
GCC = gcc
CFLAGS = 
CC = $(GCC) $(CFLAGS)

SRCS = httpclient.c

OBJS = $(SRCS:%.c=%.o)
VALGRIND = valgrind --tool=memcheck --verbose --log-file

httpclient: $(OBJS)
	$(CC) $(OBJS) -o httpclient

testmemory: $(TARGET)
	$(VALGRIND)=./valgrind.log ./httpclient dtunes.ecn.purdue.edu 80 /ece463/lab1/path_very_long.txt > output1.txt

testall: test1 test2

test1: httpclient
	./httpclient dtunes.ecn.purdue.edu 80 /ece463/lab1/path_very_long.txt > output1.txt
	diff output1.txt test_very_long.txt
test2: httpclient
	./httpclient dtunes.ecn.purdue.edu 80 /ece463/lab1/path_short.txt > output2.txt
	diff output2.txt test_short.txt

.c.o:
	$(CC)  -c $*.c

clean:
	rm -f $(OBJS) $(TARGET)
