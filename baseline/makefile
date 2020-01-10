all:
	# g++ -std=c++11 -D CONSERVATIVE src/baseline.cpp -o conservative
	# g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy
	# g++ -std=c++11 -D RANDOM src/baseline.cpp -o random
	# g++ -std=c++11 src/pure.cpp -o pure
	# g++ -std=c++11 src/progressive.cpp -o progressive
	g++ -std=c++11 -D refine src/progressive.cpp -o r07944013
	# g++ -std=c++11 -D sto src/progressive.cpp -o progressive_sto
	# g++ -std=c++11 -D rd src/progressive.cpp -o progressive_rd
	# g++ -std=c++11 -D pr src/progressive.cpp -o progressive_pr
	# g++ -std=c++11 -D ba10 src/progressive.cpp -o progressive_ba10
	# g++ -std=c++11 -D ba50 src/progressive.cpp -o progressive_ba50

conservative:
	g++ -std=c++11 -D CONSERVATIVE src/baseline.cpp -o conservative

random:
	g++ -std=c++11 -D RANDOM src/baseline.cpp -o random

greedy:
	g++ -std=c++11 -D GREEDY src/baseline.cpp -o greedy

pure:
	# g++ -std=c++11 src/pure.cpp -o pure

progressive:
	# g++ -std=c++11 src/progressive.cpp -o progressive


clean:
	rm -rf greedy
	rm -rf conservative
	rm -rf random
	rm -rf pure
	rm -rf progressive
	rm -rf progressive_rd
	rm -rf progressive_pr
	rm -rf progressive_sto
	rm -rf progressive_ba10
	rm -rf progressive_ba50
	rm -rf progressive_refine
	rm -rf r07944013
	rm -rf .log.*
