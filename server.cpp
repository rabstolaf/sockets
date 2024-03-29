/** @file
    @brief Main program for multithreaded file server
    @par Requires configuration file execpdc.config for Config object
    @par No command line args  
    @par Environment variables - override Config values if present
     - \c EXECPDC_PORT - overrides Config value for PORT for this server
     - \c EXECPDC_DO_RUN - overrides Config value for DO_RUN
     - \c EXECPDC_JOBE_DIR - overrides Config value for JOBE_DIR
     - \c EXECPDC_CUDA_ARCH - overrides Config value for CUDA_ARCH

    @par This program performs actions according to two kinds of input:
     - <em>services</em> are actions requested over a network connection
       from a machine called the <em>client</em>
     - <em>commands</em> are actions requested via standard input

    @par Predefined commands:
     - \c DUMP - print information about current clients
       <em>Note:</em> Code for this command provides a suggested pattern for 
       adding custom commands.
        - 1 optional argument, name of file for writing that client information.
	  Default is to write to standard output
     - \c EXIT - quit server program\n
       Send DONE message to any clients, wait for thos clients to complete, 
       then shut down this server and exit.  


    @par Protocol (rules for correct communication, for services)
    @parblock
     - <strong><em>Server initialization</em></strong>\n
       Prerequisite:  none
       -# Server starts
       -# Server starts Servers' command thread
       -# Server initializes server socket, then blocks on accept()

     - <strong><em>Client initialization</em></strong>\n
       Prerequisite: <em>Server initialization</em>
       -# Client starts and calls connect() (perhaps within a constructor)
       -# (Server's accept generates server's client socket)
       -# Server sends welcome message to Client
          - \c ACK - indicates success
          - \c NACK - indicates failure, too many clients
          - \c DONE - indicates client should terminate
       -# Client receives welcome message;  \n
          on success, Server allocates and records client thread 

     - <strong><em>Service operation</em></strong>\n
       Prerequisite: <em>Client initialization</em>\n
       Services:  
       - \c LABEL - set client thread's label within Server
         -# Client sends message \c LABEL followed by text
         -# Server assigns that text as that client thread's label, then \n
	    Server sends \c ACK message to Client
       - \c MSG - print text on Server's standard output.  
         <em>Note:</em> Code in for this service (in server and client) 
	 provides a suggested pattern for adding custom services.
         -# Client sends message \c MSG followed by text
         -# Server prints that text, then \n
	    Server sends \c ACK message to Client
	 -# Client receives \c ACK message
       - \c DONE - request Server to send a DONE message to Client
         -# Client sends message \c DONE
         -# Server sends \c DONE message to Client
       - \c EXECPDC - request to carry out backend PDC computation
         -# Client sends message \c EXECPDC followed by a unique workdir name and filename for desired compn
         -# Server prints that text, then \n
	    Server sends \c SUCCESS or \c ERROR message to Client, with stdin/stout from the PDC computation
	 -# Client receives that \c SUCCESS or \c ERROR message
       
     - <strong><em>Client termination</em></strong>\n
       Prerequisite:  Server has already sent DONE message to Client
       -# Client sends \c END message to Server
       -# Client ends;\n  Server performs join() on client thread, then 
          deallocates and forgets it.

     - <strong><em>Server termination</em></strong>\n
       Prerequisite:  <em>Server initialization</em>
       -# Server's command thread detects EXIT condition (e.g., from stdin),
          then marks Server for termination (contin = 0)
       -# Server's command thread performs <em>Client initialization</em>
       -# Server (main thread) detects termination and performs 
          <em>Client termination</em> on every recorded Client
       -# Server's command thread performs <em>Client termination</em>; 
          Server (main thread) performs join() on Server's command thread
       -# Server ends

    @endparblock

   RAB 5/16 rev 12/17 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <thread>
#include "Config.h"
#include "Worker.h"
using namespace std;

void doCommands(ManagementData *mgtp, int port);

int main(int argc, char **argv) {
  char *prog = argv[0];
  int port;

  const char *config_filename = "execpdc.config"; // GENERALIZE
  Config config;
  try {
    config.processFile(config_filename);
  } catch (runtime_error &e) {
    cerr << prog << ":  " << e.what() << "; aborting" << endl;
    return 1;
  }
  port = atoi(config.valueOrEnv("PORT", "EXECPDC_PORT").c_str());

  ServerSocket ssock(port);
  if (ssock.isBound())
    cout << "[main] port " << port << " bound to server socket" << endl;
  else {
    cout << "[main] could not bind port " << port << " to server socket, aborting"
	 << endl;
    return 1;
  }
  // server socket ssock is now set up

  /* Define shared data structure for controlling server */
  ManagementData mgt;
  mgt.do_run = config.valueOrEnv("DO_RUN", "EXECPDC_DO_RUN").c_str();
  mgt.cuda_arch = config.valueOrEnv("CUDA_ARCH", "EXECPDC_CUDA_ARCH").c_str();
  mgt.jobe_dir = config.valueOrEnv("JOBE_DIR", "EXECPDC_JOBE_DIR").c_str();

  /* Create a thread for handling command input */
  thread commandThread(doCommands, &mgt, port);
  
  /* MAIN LOOP: */
  Socket *csockp;  // to hold new client socket
  while (mgt.contin) {
    csockp = ssock.accept();

    if (mgt.contin && !mgt.addWorker(csockp)) {
      cout << "[main] rejecting a new client - maximum client count" << endl;
      csockp->send("NACK", 4);
      csockp->close();
      continue;
    }

  }

  mgt.cleanupWorkers();  
  commandThread.join();
  cout << "[main] command thread join accomplished" << endl;

  return 0;
}


/* command thread and command handlers */

void doDUMP(ManagementData *mgt, const char *buff);
void doUnknown(const char *buff);

/** code for command thread
    @param mgt points to shared data structure for controlling server */
void doCommands(ManagementData *mgtp, int port) {
  const int maxbuff = 100;
  char buff[maxbuff];

  /* command loop */
  while (cin.getline(buff, maxbuff-1)) {

    if (strcmp(buff, "DUMP") == 0) 
      doDUMP(mgtp, buff);

    else if (strcmp(buff, "EXIT") == 0) 
      break;

    else 
      doUnknown(buff);     // unrecognized command type

  }

  cout << "[command thread] EXIT encountered" << endl;
  mgtp->contin = 0;
  
  // the following step unblocks [main] accept(), in order to exit MAIN LOOP
  Socket sock("localhost", port);
}

/* command handlers */

void doDUMP(ManagementData *mgtp, const char *buff) {
  stringstream ss(buff+4);
  string fname;
  ss >> fname; 

  cout << mgtp->getDump() << endl;
}

void doUnknown(const char *buff) {
  cerr << "[command thread] unrecognized command: " << buff << endl;
}

