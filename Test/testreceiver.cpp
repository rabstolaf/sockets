/* receiver testing utility - echo messages received over a network port
   to standard output.  
   Requires one command line arg:  
     1.  port number to use (on this machine). 
   RAB 4/16 revised for testreceiver 12/17 */

#include <iostream>
#include <cstring>
#include "Socket.h"

using namespace std;

#define MAXBUFF 10000

int main(int argc, char **argv) {
  char *prog = argv[0];
  int port;

  if (argc < 2) {
    cerr << "Usage:  " << prog << " port" << endl;
    return 1;
  }
  port = atoi(argv[1]);

  ServerSocket ssock(port);
  if (ssock.isBound())
    cerr << prog << ": port " << port << " bound to server socket" << endl;
  else {
    cerr << prog << ": could not bind port " << port << " to server socket, aborting"
	 << endl;
    return 1;
  }
  
  Socket *csockp;
  int contin = 1;

  while (contin) {
    csockp = ssock.accept();

    char buff[MAXBUFF];  /* message buffer */
    int ret;  /* return value from a call */
    ret = csockp->recv(buff, MAXBUFF-1);

    buff[ret] = '\0';  // add terminating nullbyte to received array of char
    cerr << prog << ": received message from " << csockp->getIPAddr() << "[" 
	 << csockp->getPort() << "] (" << ret << " chars):" << endl 
	 << buff << endl;
    if (!strcmp(buff, "END_TESTRECEIVER"))
      contin = 0;
    else 
      cout << buff << endl;
    delete csockp;
    
  }
  cerr << prog << ": shutting down" << endl;
  return 0;
}
