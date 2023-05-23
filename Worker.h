/* Interface module for multithreaded server thread class
   RAB 5/2/16 rev 12/22/17 */

#include <thread>
#include "Socket.h"
#include "ManagementData.h"

using namespace std;

/** A class representing a thread that implements services for a client */
class Worker : public thread {
 protected:
  int id;  /**< unique identifier for this worker thread */
  char *label;  /**< label for this worker thread */
  Socket *sockp;  /**< socket for communicating with client */
  ThreadState state; /**< current status of this worker thread */
  static int MAXBUFF; /**< buffer size for socket communication */

 public:
  Worker(int i = -1, Socket *sp = 0, const ManagementData *mgt = 0);

  ~Worker();
  int getId() { return id; }
  char *getLabel() { return label; }
  ThreadState getState() { return state; }
  const char *getIPAddr() { return sockp->getIPAddr(); }
  void setLabel(const char *lbl);
  static char *cloneString(const char *str);

 protected:
  void doWork(const ManagementData *mgt);

  // message handlers
  void doDONE(void);
  void doLABEL(const char *buff);
  void doMSG(const char *buff);
  void doEXECPDC(const char *buff, const ManagementData*mgt);
  void doUnknown(const char *buff);
}; 

