/* Implementation module for Internet communication using Berkeley sockets
   RAB 4/30/16 */

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Socket.h"

using namespace std;

/** helper method for socket initialization
    @sc Initialize descrip, and assign 0 to connected and ip_addr, -1 to port 
*/
void Socket::create_socket() { 
  if ((descrip = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    cerr << "Socket::Socket():" << strerror(errno) << endl;
  ip_addr = 0;
  mark_disconnected();
}

/** helper method to assign connection status to state variables connected, 
    ip_addr, and port */
void Socket::set_connected() {
  struct sockaddr_in ca; // socket address structure for the new client
  socklen_t size = sizeof(struct sockaddr);
  if (!::getpeername(descrip, (struct sockaddr*) &ca, &size)) {
    connected = 1;
    ip_addr = new char[INET_ADDRSTRLEN]; // case for INET6_ADDRSTRLEN ?
    inet_ntop(AF_INET, &(ca.sin_addr), ip_addr, INET_ADDRSTRLEN);
    port = htons(ca.sin_port);
  } else {
    mark_disconnected();
    if (errno != ENOTCONN)
      cerr << "Socket::Socket() getpeername " << strerror(errno) << endl;
  }
}

/** helper method to assign state variables connected, ip_addr, and port
    to indicate disconnectedness */
void Socket::mark_disconnected() { // precondition:  ip_addr is assigned
    connected = 0;
    if (ip_addr != 0) {
      delete [] ip_addr;
      ip_addr = 0;
    }
    port = -1;
}


/* Initialize a Socket
   @param sd Socket descriptor for an existing socket.
   With default first argument, first create a socket and use that descriptor 
   If sd indicates an existing connected socket, assign 1 to state variable 
   connected and proper values to ip_addr, port;  otherwise, assign 0 to 
   connected and ip_addr and -1 to port.  Assumes IPv4.
 */
Socket::Socket(int sd) {
  ip_addr = 0;
  if (sd == -1)
    create_socket();
  else {
    descrip = sd;
    set_connected();
  }
}

/** initialize a socket and attempt to connect to a particular host and port
    @param host Internet name for a computer, e.g., rns202-1.cs.stolaf.edu
    @param port TCP port for server socket on host  */
Socket::Socket(const char *host, int port) {
  create_socket();
  connect(host, port);
}

Socket::~Socket() {
  close();
}

/** Invalidate this socket for further use. 
    If connected, first shut down communication. */
void Socket::close() {
  if (connected and ::shutdown(descrip, SHUT_RDWR) < 0)
      cerr << "Socket::close() shutdown " << strerror(errno) << endl;
  mark_disconnected();
  if (::close(descrip) < 0) 
    cerr << "Socket::close() " << strerror(errno) << endl;
  // cout << "closed socket <" << descrip << ">" << endl; // DEBUG
}

/** Connect this socket to a server 
    @pre State variable connected must be assigned before calling this method
    @param host Internet name for a computer, e.g., rns202-1.cs.stolaf.edu
    @param port TCP port for server socket on host
    @return 1 on success, 0 on failure */
int Socket::connect(const char *host, int port) {
  if (connected) {
    cerr << "Socket::connect() - already marked as connected, skipping" << endl;
    return 0;
  }
  // connected == 0

  struct hostent *hp;
  struct sockaddr_in sa;
  if ((hp = gethostbyname(host)) == NULL) {
    cerr << "Socket::connect() gethostbyname() " << strerror(errno) << endl;
    return 0;
  }
  memset((char *) &sa, '\0', sizeof(sa));
  memcpy((char *) &sa.sin_addr.s_addr, hp->h_addr, hp->h_length);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  
  if (::connect(descrip, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    cerr << "Socket::connect() " << strerror(errno) << endl;
    if (errno == EISCONN)
      //connected = 1;
      set_connected();
    return 0;
  }
  //connected = 1;
  set_connected();
  return 1;  
}


/** Send a message on this socket
    @param msg Message to be transmitted
    @param len Number of characters of msg to transmit
    @return Number of characters transmitted, or -1 on failure */
int Socket::send(const char *msg, int len) {
  if (!connected) {
    cerr << "Attempt to send() on a disconnected Socket" << endl;
    return -1;
  }

  int ret;
  if ((ret = ::send(descrip, msg, len, 0)) < 0) 
    cerr << "Socket::send() " << strerror(errno) << endl;
  return ret;
}

/** Receive a message on this socket
    @param buff Array to receive message
    @param len Maximum number of characters to receive in buff
    @return Number of characters transmitted, or -1 on failure */
int Socket::recv(char *buff, int len) {
  if (!connected) {
    cerr << "Attempt to recv() on a disconnected Socket" << endl;
    return -1;
  }

  int ret;
  if ((ret = ::recv(descrip, buff, len, 0)) < 0) {
    cerr << "Socket::recv() " << strerror(errno) << endl;
    return -1;
  }
  return ret;
}

/** initialize a socket and attempt to associate a port with it 
    @param port TCP port number */
ServerSocket::ServerSocket(int port) : Socket() {
  bound = 0;
  if (port != -1)
    bind(port);
}


/** Associate a port with this server socket
    @pre State variable connected must be assigned before calling this method
    @param port TCP port number
    @return 1 on success, 0 on failure */
int ServerSocket::bind(int port) {
  if (bound) {
    cerr << "ServerSocket::bind() - already marked as bound, skipping" << endl;
    return 0;
  }
  // bound == 0

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = INADDR_ANY;

  if (::bind(descrip, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
    cerr << "ServerSocket::bind() bind " << strerror(errno) << endl;
    return 0;
  }
  if (::listen(descrip, 5) < 0) {
    cerr << "ServerSocket::bind() listen " << strerror(errno) << endl;
    return 0;
  }
  bound = 1;
  return 1;  
}


/** Wait for and accept one connection request from another process, 
    potentially on a different host
    @return Address of a newly allocated Socket for communication with 
    another process.  Return 0 if attempt to accept failed or if this 
    server socket is not bound */ 
Socket *ServerSocket::accept() {
  if (!bound)
    return 0;

  struct sockaddr_in ca; // socket address structure for the new client
  socklen_t size = sizeof(struct sockaddr);
  int clientd;

  cerr << "Waiting for a incoming connection..." << endl;
  if ((clientd = ::accept(descrip, (struct sockaddr*) &ca, &size)) < 0) {
    cerr << "ServerSocket::accept() " << strerror(errno) << endl;
    return 0;
  }
  // ::accept() attempt was successful
  /* Note:  Could retrieve the IP address of client from ca, size */

  return new Socket(clientd);
}
