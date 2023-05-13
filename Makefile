# Makefile for socket examples

CFLAGS = -Wall -std=c++11

all: sender receiver client server execpdc # sender2 receiver2 

Socket.o:  Socket.cpp Socket.h
	g++ $(CFLAGS) -c Socket.cpp


SENDER_OBJS = sender.o Socket.o
sender: $(SENDER_OBJS)
	g++ $(CFLAGS) -o sender $(SENDER_OBJS)

sender.o:  sender.cpp Socket.h
	g++ $(CFLAGS) -c sender.cpp

RECEIVER_OBJS = receiver.o Socket.o
receiver: $(RECEIVER_OBJS)
	g++ $(CFLAGS) -o receiver $(RECEIVER_OBJS)

receiver.o:  receiver.cpp Socket.h
	g++ $(CFLAGS) -c receiver.cpp


SENDER2_OBJS = sender2.o Socket.o
sender2: $(SENDER2_OBJS)
	g++ $(CFLAGS) -o sender2 $(SENDER2_OBJS)

sender2.o:  sender2.cpp Socket.h
	g++ $(CFLAGS) -c sender2.cpp

RECEIVER2_OBJS = receiver2.o Socket.o
receiver2: $(RECEIVER2_OBJS)
	g++ $(CFLAGS) -o receiver2 $(RECEIVER2_OBJS)

receiver2.o:  receiver2.cpp Socket.h
	g++ $(CFLAGS) -c receiver2.cpp


Config.o:  Config.cpp Config.h
	g++ $(CFLAGS) -c Config.cpp

CLIENT_OBJS = client.o Socket.o Config.o
client: $(CLIENT_OBJS)
	g++ $(CFLAGS) -o client $(CLIENT_OBJS)

client.o:  client.cpp Socket.h
	g++ $(CFLAGS) -c client.cpp

EXECPDC_OBJS = execpdc.o Socket.o Config.o
execpdc: $(EXECPDC_OBJS)
	g++ $(CFLAGS) -o execpdc $(EXECPDC_OBJS)

execpdc.o:  execpdc.cpp Socket.h execpdc.config
	g++ $(CFLAGS) -c execpdc.cpp

SERVER_OBJS = server.o Socket.o Worker.o ManagementData.o Config.o
server: $(SERVER_OBJS)
	g++ $(CFLAGS) -pthread -o server $(SERVER_OBJS)

server.o:  server.cpp Socket.h ManagementData.h Worker.h
	g++ $(CFLAGS) -c server.cpp

Worker.o: Worker.cpp Socket.h ManagementData.h Worker.h 
	g++ $(CFLAGS) -c Worker.cpp

ManagementData.o: ManagementData.cpp ManagementData.h
	g++ $(CFLAGS) -c ManagementData.cpp

tarball:
	cd .. ; tar cfz sockets.tar.gz sockets/{{sender,receiver,client,execpdc,server}.cpp,{Socket,Worker}.{cpp,h},Makefile.sockets}

clean:
	rm -rf sender receiver client execpdc server *.o
