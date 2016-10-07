CC = g++
CFLAGS = -c -Wall -std=c++11
OBJS = main.o apriori.o hashTree.o

all: hcrminer
	 rm *.o
	 @echo "\nThis is an Apriori algorithm program using HashTree for support count."
	 @echo "This program was developed by Pei Xu xuxx0884@umn.edu for CSci5523."
	 @echo "This program follows MIT license.\n"
	 @echo "You can find the resource of this program at https://github.com/xupei0610/Apriori.git"
	 @echo "Try ./hcrminer 50 0.8 small test_result 20 50"
	 @echo "Or run ./run.sh to run a whole test. (It may take quite a long time.)"

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
