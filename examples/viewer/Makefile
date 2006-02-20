
COMPILER=g++
CFLAGS=-g -I ../.. `pkg-config --cflags gtk+-2.0`
LDFLAGS=-I ../.. `pkg-config --libs gtk+-2.0` -lgnutls -lgsasl


viewer: viewer.o ../../libvmime-debug.a
	$(COMPILER) $(LDFLAGS) -o viewer viewer.o ../../libvmime-debug.a

viewer.o: viewer.cpp
	$(COMPILER) $(CFLAGS) -c -o viewer.o viewer.cpp

../../libvmime-debug.a:
	@echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	@echo "!  You must build VMime library before compiling the example  !"
	@echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	@false

