CC = g++
CFLAGS = -c -Wall -std=c++11 -pthread
OBJS = main.o apriori.o hashTree.o transTree.o itemTree.o

all: hcrminer
	 rm *.o
	 @echo "\nThis is an Apriori algorithm program using HashTree for support count."
	 @echo "This program was developed by Pei Xu xuxx0884@umn.edu for CSci5523."
	 @echo "This program follows MIT license.\n"
	 @echo "You can find the resource of this program at https://github.com/xupei0610/Apriori.git"
	 @echo "Try ./hcrminer 1000 0.8 large test_result 20 50"
	 @echo "Or run ./run.sh to run a whole test. (It may take quite a long time.)"

hcrminer: $(OBJS)
	$(CC) $(OBJS) -o hcrminer

main.o: main.cc
	$(CC) $(CFLAGS) main.cc

apriori.o: apriori.cc
	$(CC) $(CFLAGS) apriori.cc

hashTree.o: hashTree.cc
	$(CC) $(CFLAGS) hashTree.cc

transTree.o: transTree.cc
	$(CC) $(CFLAGS) transTree.cc

itemTree.o: itemTree.cc
	$(CC) $(CFLAGS) itemTree.cc

clean:
	rm *.o hcrminer
