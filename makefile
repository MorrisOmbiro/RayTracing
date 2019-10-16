output: main.o
	g++ main.o -o a
main.o: main.cpp ray_trace.h Objects.h _3d_values.h
	g++ -c main.cpp

target: dependencies
	action 

clean:
	rm *.o a
