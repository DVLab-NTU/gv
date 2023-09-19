andTest: clean File.o Proof.o Solver.o andTest.o
	g++ -o $@ -std=c++11 -g File.o Proof.o Solver.o andTest.o

File.o: File.cpp
	g++ -c -std=c++11 -g File.cpp

Proof.o: Proof.cpp
	g++ -c -std=c++11 -g Proof.cpp

Solve.o: Solver.cpp
	g++ -c -std=c++11 -g Solver.cpp

andTest.o: andTest.cpp
	g++ -c -std=c++11 -g andTest.cpp

clean:
	rm -f *.o
