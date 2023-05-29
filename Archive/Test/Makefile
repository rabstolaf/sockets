# Makefile for socket examples

CFLAGS = -Wall -std=c++11 -I..

all: testsend testreceiver 


SENDER_OBJS = testsend.o ../Socket.o
testsend: $(SENDER_OBJS)
	g++ $(CFLAGS) -o testsend $(SENDER_OBJS)

testsend.o:  testsend.cpp ../Socket.h
	g++ $(CFLAGS) -c testsend.cpp

RECEIVER_OBJS = testreceiver.o ../Socket.o
testreceiver: $(RECEIVER_OBJS)
	g++ $(CFLAGS) -o testreceiver $(RECEIVER_OBJS)

testreceiver.o:  testreceiver.cpp ../Socket.h
	g++ $(CFLAGS) -c testreceiver.cpp


