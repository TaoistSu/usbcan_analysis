
all:
	#gcc -o hello -L. -lcontrolcan -lpthread -lusb main.c
	#arm-none-linux-gnueabi-gcc -o can_analyse -L. -L /home/light/controlcan -lcontrolcan -lpthread can_analyse.c
	g++ -o can_analyse_rev can_analyse_rev.cpp /home/light/controlcan/libcontrolcan.so  -lpthread 

clean:
	rm -f *.o can_analyse_rev
