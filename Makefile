

testApp: test.o tinySockets.h tinySockets.o
	g++ -o testApp test.o tinySockets.o

clean:
	rm *.o

tinySockets.o: tinySockets.h tinySockets.cpp
	g++ -c -o tinySockets.o tinySockets.cpp

test.o: tinySockets.h test.cpp
	g++ -c -o test.o test.cpp


