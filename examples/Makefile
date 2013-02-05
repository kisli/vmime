all: example1 example6

example1: example1.cpp ../libvmime-debug.a
	g++-mp-4.8 -o example1 example1.cpp -I.. ../libvmime-debug.a -L /opt/local/lib -lgnutls -lgsasl -liconv

example6: example6.cpp ../libvmime-debug.a
	g++-mp-4.8 -o example6 example6.cpp -I.. ../libvmime-debug.a -L /opt/local/lib -lgnutls -lgsasl -liconv

