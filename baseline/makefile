all:
	g++ -std=c++11 -D CONSERVATIVE src/baseline.cpp -o conservative
	g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy
	g++ -std=c++11 -D RANDOM src/baseline.cpp -o random
	g++ -std=c++11 src/pure.cpp -o pure
	g++ -std=c++11 src/progressive.cpp -o progressive

conservative:
	g++ -std=c++11 -D CONSERVATIVE src/baseline.cpp -o conservative

random:
	g++ -std=c++11 -D RANDOM src/baseline.cpp -o random

greedy:
	g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy

pure:
	g++ -std=c++11 src/pure.cpp -o pure

progressive:
	g++ -std=c++11 src/progressive.cpp -o progressive


clean:
	rm -rf greedy
	rm -rf conservative
	rm -rf random
	rm -rf pure
	rm -rf progressive
	rm -rf .log.*
