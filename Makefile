all:
	g++ neuralnetwork.c SpaceInvaders.cpp -lGL -lglut -lm -fopenmp -O3 -Wall -Wextra -Wno-unused-parameter -o SpaceInvaders.out

run:
	./SpaceInvaders.out

clean:
	rm *.out