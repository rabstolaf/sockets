/* Client-side use of Berkeley socket calls -- send one message to server
   Command-line args:
     1.  (optional) label for this client
   RAB 5/16 rev 12/17 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include "Socket.h"
#include "Config.h"

using namespace std;

#define MAXBUFF 100000
const char *label_env = "CLIENT_LABEL";
const char *default_label = "Client";

int main(int argc, char **argv) {
  //  char *prog = argv[0];
  const char *host;
  int port;
  const char *label = 0;
  int ret;  /* return value from a call */

  const char *config_filename = "execpdc.config"; // GENERALIZE
  Config config(config_filename);
  host = config["SERVER"].c_str();
  port = atoi(config["PORT"].c_str());
  cout << host << " " << port << endl;

  if (label == 0) 
    label = getenv(label_env);
  if (label == 0)
    label = default_label;
  cout << "Label: " << label << endl;

  Socket sock(host, port);
  if (sock.isConnected()) 
    cout << "Connected." << endl;
  else {
    cout << "Could not connect, aborting" << endl;
    return 1;
  }
  
  char buff[MAXBUFF];  /* message buffer */
  stringstream ss;  /* stringstream for constructing messages */

  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0) {
    cout << "Could not receive welcome message from server, aborting" << endl;
    return 1;
  } 
  buff[ret] = '\0';
  if (strcmp(buff, "DONE") == 0) {
    cout << "Server rejected new connection, aborting" << endl;
    return 1;
  }
  // assert:  welcome message received and connection to server completed

  ss.str(""); 
  ss << "LABEL " << label;
  strcpy(buff, ss.str().c_str());
  if ((ret = sock.send(buff, strlen(buff))) < 0) {
    cout << "Could not send LABEL message - send() failed" << endl;
    return 1;
  }
  cout << "LABEL message: " << ret << " characters sent" << endl;
  if ((ret = sock.recv(buff, MAXBUFF-1)) < 0) {
    cout << "Could not receive response from LABEL message - send() failed" 
	 << endl;
    return 1;
  } else {
    buff[ret] = '\0';
    cout << "LABEL message acknowledged " << buff << endl;
  }

  while (strcmp(buff, "DONE") != 0) {
    cout << "Enter a one-line message to send (max " << MAXBUFF-1 << 
      " chars), or DONE to quit" << endl;
    if (!cin.getline(buff, MAXBUFF)) {
      cout << "Error or end of input -- aborting" << endl;
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
    cout << ret << " characters sent" << endl;

    if ((ret = sock.recv(buff, MAXBUFF-1)) < 0)
      return 1;
    else {
      buff[ret] = '\0';
      cout << ret << " characters received" << endl << buff << endl;
    }
  }

  cout << "Termination message received" << endl;
  sock.send("END", 3);
  return 0;

}
