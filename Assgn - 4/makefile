all: aouttest1 aouttest2 aouttest3 aouttest4

aouttest1: test1.cpp myfs.h
	g++ -w test1.cpp -o aouttest1
aouttest2: test2.cpp myfs.h
	g++ -w test2.cpp -o aouttest2
aouttest3: test3.cpp myfs.h
	g++ -w test3.cpp -o aouttest3
aouttest4: test4.cpp myfs.h
	g++ -w test4.cpp -o aouttest4 -pthread 
clean:
	rm aouttest1 aouttest2 aouttest3 aouttest4
