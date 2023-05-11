/* Client-side use of Berkeley socket calls -- send one message to server
   Command-line args:
     1.  name of host to connect to (use  localhost  to connect to same host)
     2.  port number to use. 
     3.  (optional) label for this client
   RAB 5/16 rev 12/17 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "Socket.h"
#include <chrono>
#include <ctime>

using namespace std;

#define MAXBUFF 100000
const char *label_env = "CLIENT_LABEL";
const char *default_label = "Client";

int main(int argc, char **argv) {
  char *prog = argv[0];
  char *host;
  int port;
  const char *label = 0;
  int ret;  /* return value from a call */
  stringstream ss;  /* utility stringstream */
  const char *msg;  /* utility string pointer */

  // initial log entry
  ofstream log;
  log.open("execpdc_local.log", ios_base::app);
  if (!log) {
    cerr << "unable to open log file, aborting" << endl;
    return 1;
  }
  time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
  log << endl << "===== " << ctime(&now) << endl;

  if (argc < 3) {
    ss.str("");
    ss << "Usage:  " << prog << " host port" << endl; 
    cerr << ss.str();
    log << ss.str();
    return 1;
  }
  host = argv[1];
  port = atoi(argv[2]);
  if (argc > 3) 
    label = argv[3];

  if (label == 0) 
    label = getenv(label_env);
  if (label == 0)
    label = default_label;
  log << "Label: " << label << endl;

  Socket sock(host, port);
  if (sock.isConnected()) 
    log << "Connected." << endl;
  else {
    msg = "Could not connect, aborting";
    cerr << msg << endl;
    log << msg << endl;
    return 1;
  }
  
  char buff[MAXBUFF];  /* message buffer */

  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0) {
    
    cerr << "Could not receive welcome message from server, aborting" << endl;
    return 1;
  } 
  buff[ret] = '\0';
  if (strcmp(buff, "DONE") == 0) {
    msg = "Server rejected new connection, aborting";
    cerr << msg << endl;
    log << msg << endl;
    return 1;
  }
  // assert:  welcome message received and connection to server completed

  ss.str(""); 
  ss << "LABEL " << label;
  strcpy(buff, ss.str().c_str());
  if ((ret = sock.send(buff, strlen(buff))) < 0) {
    msg = "Could not send LABEL message - send() failed";
    cerr << msg << endl;
    log << msg << endl;
    return 1;
  }
  log << "LABEL message: " << ret << " characters sent" << endl;
  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0) {
    msg = "Could not receive response from LABEL message - send() failed";
    cerr <<  msg << endl;
    log << msg << endl;
    return 1;
  } else {
    buff[ret] = '\0';
    log << "LABEL message acknowledged " << buff << endl;
  }

  /* cout << "Enter a one-line message to send (max " << MAXBUFF-1 << 
     " chars), or DONE to quit" << endl; */
  if (!cin.getline(buff, MAXBUFF)) {
    msg = "Error or end of input -- aborting";
    cerr << msg << endl;
    log << msg << endl;
    return 1;
  }
  if (strcmp(buff, "DONE") == 0)
    ; /* outgoing message same as input message */
  else { // content of a message provided
    ss.str("");
    ss << "SCRIPT " << buff;
    strcpy(buff, ss.str().c_str());
  }
  if ((ret = sock.send(buff, strlen(buff))) < 0)
    return 1;
  log << ret << " characters sent" << endl;
  
  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0)
    return 1;
  else {
    buff[ret] = '\0';
    ss.str("");
    ss << ret << " characters received" << endl << buff << endl;
    log << ss.str();
    cout << buff;
  }

  /*  if ((ret = sock.send("DONE", 4)) < 0)
    return 1;
  log << "DONE message sent" << endl;

  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0) {
    msg = "Could not receive response from DONE message - send() failed";
    cerr <<  msg << endl;
    log << msg << endl;
    return 1;
  } 
  buff[ret] = '\0';
  log << "DONE message acknowledged: " << buff << endl;
  */

  log << "sending END message" << endl;
  sock.send("END", 3);
  return 0;

}
