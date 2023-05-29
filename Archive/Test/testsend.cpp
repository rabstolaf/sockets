/* send testing utility - send command line arguments or standard input to 
   a named testreceiver 
    - config is name of configuration file, default ./testsend.config */
const char *USAGE="Usage:    testsend [-c config] dest [msg...]";
/*  - dest is identifier of a testreceiver, specified in config 
    - msg is words of single message to send;  if omitted, copy standard input
   config file has format 
      name host port
   RAB 4/16, modified for test system 12/17 */

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include "Socket.h"

using namespace std;

#define MAXBUFF 10000

int main(int argc, char **argv) {
  char *prog = argv[0];
  char *host = 0;
  int port = -1;
  char buff[MAXBUFF];  /* message buffer */
  int ret;  /* return value from a call */

  const char *config = "testsend.config";
  string dest("");  // name for destination testreceiver
  string msg("");  // command-line message, or empty string if none provided

  char **next = argv+1;
  if (*next != 0 && !strcmp(*next, "-c")) {
    if (*++next == 0) {
      cerr << USAGE << endl; 
      return 1;
    }
    config = *next;
    next++;
  }
  // flags parsed

  if (*next == 0) {
    cerr << USAGE << endl;
    return 1;
  }
  dest = *next++;
  while (*next) {
    msg = msg + " " + *next++;
  }
  if (msg != "")
    msg = msg.substr(1) + "\n";
  cerr << "DEBUG:  msg = ." << msg << "." << endl;
  // command-line args parsed: config, dest, msg determined

  ifstream conf(config);  
  if (!conf.good()) {
    cerr << prog << ": missing, empty, or unreadable config file " << config 
	 << ", aborting" << endl;
    return 1;
  }

  stringstream ss;
  string tmp("");
  while (conf.good()) {
    conf.getline(buff, MAXBUFF);
    ss.str(buff);
    ss >> tmp;
    if (tmp == dest)
      break;
  }
  if (tmp == dest) {
    ss >> tmp; 
    host = new char[strlen(tmp.c_str())+1];
    strcpy(host, tmp.c_str());
    ss >> port;
  } else {
    cerr << prog << ": destination " << dest << " not found in config file " 
	 << config << ", aborting" << endl;
    return 1;
  }

  Socket sock(host, port);
  if (sock.isConnected()) 
    cerr << prog << ": connected to " << sock.getIPAddr() << "[" 
	 << sock.getPort() << "]" << endl;
  else {
    cout << prog << ": could not connect, aborting" << endl;
    return 1;
  }
  
  int nchars;  /* number of bytes recently read */
  if (msg != "") {
    strcpy(buff, msg.c_str());
    nchars = strlen(msg.c_str());  
  } else {
    nchars = read(0, buff, MAXBUFF);  // we assume that captures everything...
  }
  // buff holds entire message to send
  
  if ((ret = sock.send(buff, nchars-1)) < 0)
    return 1;
  cout << prog << ": " << ret << " characters sent" << endl;
  return 0;
}
