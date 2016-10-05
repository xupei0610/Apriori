CC = g++
CFLAGS = -c -Wall -std=c++11

OBJS = main.o apriori.o hashTree.o

	# @echo "This is an Apriori algorithm program with HashTree support."
	# @echo "This program was developed by Pei Xu xuxx0884@umn.edu."
	# @echo "This program follows MIT license.\n"
	# @echo "Begin making main program: hcrminer..."
	#
all: hcrminer
	 rm *.o
	 @echo "\nThis is an Apriori algorithm program with HashTree support."
	 @echo "This program was developed by Pei Xu xuxx0884@umn.edu for CSci5523."
	 @echo "This program follows MIT license.\n"
	 @echo "Try ./hcrminer 100 0.2 rules 50 50"

hcrminer: $(OBJS)
	$(CC) $(OBJS) -o hcrminer

main.o: main.cc
	$(CC) $(CFLAGS) main.cc

apriori.o: apriori.cc
	$(CC) $(CFLAGS) apriori.cc

hashTree.o: hashTree.cc
	$(CC) $(CFLAGS) hashTree.cc

clean:
	rm *.o hcrminer
