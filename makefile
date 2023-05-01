CC=g++

COMPILIER=-std=c++11

LINK_THREAD=-pthread

COMMAND=-o

all: casual non_casual

casual:
	$(CC) $(COMPILIER) $(COMMAND) casual project2_2_casual.cpp $(LINK_THREAD)

non_casual:
	$(CC) $(COMPILIER) $(COMMAND) non_casual project2_2_noncasual.cpp $(LINK_THREAD)

clean:
	rm -rf *.o casual non_casual
