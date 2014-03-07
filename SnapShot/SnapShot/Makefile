CC=g++
CFLAG=-c -Wall

all: application search launcher
application: Peer.o Dealer.o Thread.o SnapShot.o Peer.o  State.o Message.o
	g++ Dealer.o Thread.o SnapShot.o Peer.o  State.o Message.o main.cpp -g  -pthread -o application
search: Search.cpp
	g++ Search.cpp -o search
launcher: launcher.cpp
	g++ launcher.cpp -o launcher
Dealer.o: Utility.h Thread.o SnapShot.o Message.o  Peer.o State.o Dealer.h Dealer.cpp
	g++ $(CFLAG) Dealer.cpp -o Dealer.o  
Thread.o: Utility.h Message.o Thread.h Thread.cpp Dealer.h 
	g++ $(CFLAG) Thread.cpp -o Thread.o  
SnapShot.o: State.o Message.o SnapShot.h SnapShot.cpp
	g++ $(CFLAG) SnapShot.cpp -o SnapShot.o  
Peer.o: State.o Peer.h Peer.cpp
	g++ $(CFLAG) Peer.cpp -o Peer.o  
State.o: State.h State.cpp
	g++ $(CFLAG) State.cpp -o State.o  
Message.o: Message.h Message.cpp
	g++ $(CFLAG) Message.cpp -o Message.o 

clean:
	rm -rf *.o application search launcher
