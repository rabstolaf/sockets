/* Interface module for Internet communication using Berkeley sockets
   RAB 4/30/16 */

/** A class for socket communication */
class Socket {
 protected:
  int descrip;  /**< socket descriptor, or -1 if failed to allocate socket */
  int connected; /**< non-zero if connect() succeeded on this socket */
  char *ip_addr; /**< IP address of connected peer, or 0 if not available  */
  int port;  /**< port on connected peer, or -1 if not available */

 public:
 
  Socket(int sd = -1); 
  Socket(const char *host, int port);
  ~Socket();

  int connect(const char *host, int port);
  int send(const char *msg, int len);
  int recv(char *msg, int len);
  void close();

  /** @return Value of state variable descrip */
  int getDescriptor() { return descrip; } 

  /** @return Non-zero if this socket is connected, 0 otherwise */
  int isConnected() { return connected; } 

  /** @return Value of state variable ip_addr */
  const char * getIPAddr() { return ip_addr; } 

  /** @return Value of state variable port */
  int getPort() { return port; } 

 protected:
  void create_socket(); 
  void set_connected(); 
  void mark_disconnected(); 
};


class ServerSocket : public Socket {
 protected:
  int bound; /**< non-zero if bind() succeeded on this socket */

 public:
  ServerSocket(int port = -1);

  int bind(int port);
  Socket *accept();

  /** @return Non-zero if this socket is connected, 0 otherwise */
  int isBound() { return bound; } 
}; 
