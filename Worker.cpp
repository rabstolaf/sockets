/* Implementation module for multithreaded server thread class
   RAB 5/2/16 rev 12/22/17 */

#include <iostream>
#include <cstring>
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>
#include "Worker.h"

int Worker::MAXBUFF = 200;

/** helper method
    @param str C-style string (null-terminated array of characters)
    @return Dynamically allocated copy of str */
char *Worker::cloneString(const char *str) {
  char *tmp = new char[strlen(str)+1];
  strcpy(tmp, str);
  return tmp;
}

/** @param i ID number for this worker thread 
    @param sp Pointer to socket for communicating with client */
Worker::Worker(int i, Socket *sp, const ManagementData *mgt) 
  : thread(&Worker::doWork, this, mgt), id(i), sockp(sp), state(RUNNING) {
  label = cloneString("");
}

/* NOTE about thread constructor call above:  
   methods have an implicit first argument, namely, the object whose state 
   variables they act on.  That object argument must be explicitly passed 
   for this 2-argument thread constructor */

Worker::~Worker() {
  if (label != 0) {
    delete [] label;
    label = 0;
  }
  delete sockp;
  sockp = 0;
}

/** assign label to this worker
    @param lbl C-style string 
    @sc A dynamically allocated copy of lbl is assigned to state var \c label */
void Worker::setLabel(const char *lbl) { 
  if (label != 0)
    delete [] label;
  label = cloneString(lbl); 
}


/** Code for worker thread to execute.
    @param mgt Points to shared data structure for controlling server */
void Worker::doWork(const ManagementData *mgt) {
  char buff[MAXBUFF];  /* message buffer */
  int ret;  /* return value from a call */

  cout << "[" << id << "] started" << endl;
  // cout << "[" << id << "] socket <" << sockp->getDescriptor() << ">" << endl; //DEBUG


  // send welcome message
  if (mgt->contin) 
    strcpy(buff, "ACK");
  else
    strcpy(buff, "DONE");
  sockp->send(buff, strlen(buff));  

  while (strcmp(buff, "DONE") != 0) {
    ret = sockp->recv(buff, MAXBUFF-1);

    buff[ret] = '\0';  // add terminating nullbyte to received array of char
    cout << "[" << id << "] Received message (" << ret << " chars):" << endl 
	 << buff << endl;

    if (strcmp(buff, "DONE") == 0 || !mgt->contin) 
      doDONE();

    else if (strncmp(buff, "LABEL ", 6) == 0) 
      doLABEL(buff);

    else if (strncmp(buff, "MSG ", 4) == 0) 
      doMSG(buff);

    else if (strncmp(buff, "SCRIPT ", 4) == 0) 
      doSCRIPT(buff);

    else 
      doUnknown(buff);     // unrecognized message type

  }
  // receive END message from client
  if ((ret = sockp->recv(buff, MAXBUFF-1)) != -1) {
    buff[ret] = '\0';
    cout << "[" << id << "] Received " << buff << " from client " 
	 << label << endl;
  }
  state = DONE;
}

/* message handlers */

/** Handle DONE message */
void Worker::doDONE(void) {
      sockp->send("DONE", 4);
      cout << "[" << id << "] sent termination message" << endl;
}

/** Handle LABEL message */
void Worker::doLABEL(const char *buff) {
      setLabel(buff+6);
      sockp->send("ACK", 3);
      cout << "[" << id << "] sent acknowledgment" << endl;
}

/** Handle MSG message */
void Worker::doMSG(const char *buff) {
      cout << buff+4 << endl;
      sockp->send("ACK", 3);
      cout << "[" << id << "] sent acknowledgment" << endl;
}

/** Handle SCRIPT message */
void Worker::doSCRIPT(const char *buff) {
      cout << buff+7 << endl << endl;
      
      // buff+7 is filename of script input
      /*
      const int maxfile = 100000;
      char *inbuff = new char [maxfile];
      ifstream infile;
      infile.open(buff+7, ios::in);
      infile.read(inbuff, maxfile-1);
      inbuff[infile.gcount()] = '\0';
      cout << inbuff << endl;
      */

      //      char *cmd = new char [MAXBUFF];
      stringstream ss;
      ss << "./standalone " << buff+7 << " > " << buff+7 << ".out 2> "
	  << buff+7 << ".err"; 
      int status = system(ss.str().c_str());

      cout << ss.str().c_str() << endl << "exit status " << status << endl;

      sockp->send("ACK", 3);
      cout << "[" << id << "] sent acknowledgment" << endl;
}

/** Handle unknown message */
void Worker::doUnknown(const char *buff) {
      sockp->send("NACK", 4);
      stringstream ss(buff);
      string type;
      ss >> type;  
      cout << "[" << id << "] sent negative acknowledgment - unknown message type " << type.substr(0,10)<< endl;
}

