CC = g++
CFLAGS = -c -std=c++11
OBJS = main.o apriori.o manager.o transTree.o itemTree.o hashTree.o

all: hcrminer
	 rm *.o
	 @echo "\nThis is an Apriori algorithm program using HashTree for support count."
	 @echo "This program was developed by Pei Xu xuxx0884@umn.edu for CSci5523."
	 @echo "This program follows MIT license.\n"
	 @echo "You can find the resource of this program at https://github.com/xupei0610/Apriori.git"
	 @echo "Try ./hcrminer 50 0.8 small test_result 20 50"
	 @echo "Or run ./run.sh to run a whole test. (It may take quite a long time.)"

hcrminer: $(OBJS)
	$(CC) -pthread $(OBJS) -o hcrminer

main.o: main.cc
	$(CC) $(CFLAGS) main.cc

apriori.o: ./lib/apriori.cc
	$(CC) $(CFLAGS) ./lib/apriori.cc

itemTree.o: ./lib/itemTree.cc
	$(CC) $(CFLAGS) ./lib/itemTree.cc

transTree.o: ./lib/transTree.cc
	$(CC) $(CFLAGS) ./lib/transTree.cc

manager.o: ./lib/manager.cc
	$(CC) $(CFLAGS) ./lib/manager.cc

hashTree.o: ./lib/hashTree.cc
	$(CC) $(CFLAGS) ./lib/hashTree.cc

clean:
	rm *.o hcrminer
