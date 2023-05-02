/* Implementation module for multithreaded server management class
   RAB 5/2/16 rev 12/22/17 */

#include <iostream>
#include <sstream>
#include <string>
#include "Worker.h"

using namespace std;

/* helpers */

/** check and potentially clean up a worker.  \n
    <strong>NOT THREAD SAFE</strong> - call only within a critical section.
    @pre This call is within a critical section (e.g., enterCritical() has 
    returned and leaveCritical() has not yet been called), and workers[] 
    has already been initialized in the past.
    @param indx Index of an element of workers[]
    @post Either workers[indx] == 0 or workers[index] does not have state DONE*/

void ManagementData::checkWorker(int indx) {
  if (workers[indx] != 0 && 
      workers[indx]->getState() == ThreadState::DONE) {
    cout << "[main] joining on [" << workers[indx]->getId() << "]" << endl;
    workers[indx]->join();
    cout << "[main] join on [" << workers[indx]->getId() << "] accomplished" 
	 << endl;
    delete workers[indx];
    workers[indx] = 0;
  }
}



/** default constructor
    @sc assign 1 to contin; assign empty to all elements of workers[] */
ManagementData::ManagementData() : 
  mgt_flag(ATOMIC_FLAG_INIT), contin(1), nextID(0) { 
  enterCritical();
  for (int i = 0; i < MAXWORKERS;  i++)
    workers[i] = 0;
  leaveCritical();
}

/** create a worker thread for a new client 
    @param sockp Pointer to new client's socket
    @sc Cleans up any worker threads with state DONE.
    @return 1 on success, 0 if no space remains in workers[]  */
int ManagementData::addWorker(Socket *sockp) {
  enterCritical();
  int indx = 0;  // loop control for finding unused element in workers[] 
  while (indx < MAXWORKERS) {
    checkWorker(indx);
    if (workers[indx] == 0)
      break;
    indx++;
  }
  
  if (indx == MAXWORKERS) {
    leaveCritical();
    return 0;
  }
  // assert:  workers[indx] is available
  workers[indx] = new Worker(nextID++, sockp, this);
  leaveCritical();
  return 1;
}


/** cleanup threads after EXIT encountered 
    @sc performs join() on all worker threads, then deletes those threads */
void ManagementData::cleanupWorkers() {
  enterCritical();
  for (int i = 0;  i < MAXWORKERS;  i++)
    if (workers[i]) {
      cout << "[main] joining on [" << workers[i]->getId() << "]" << endl;
      workers[i]->join();
      cout << "[main] join on [" << workers[i]->getId() << "] accomplished" 
	   << endl;
      delete workers[i];
      workers[i] = 0;
    }
  leaveCritical();
}


/** produce dump of connected clients 
    @sc Cleans up any worker threads with state DONE.
    @return string representing the dump  */

string ManagementData::getDump() {
  stringstream ss("");
  ss << "index\tid\tIP addr\tlabel" << endl;
  enterCritical();
  for (int indx = 0;  indx < MAXWORKERS;  indx++) {
    checkWorker(indx);
    if (workers[indx] != 0) {
      Worker *w = workers[indx];
      ss << indx << "\t" << w->getId() << "\t" << w->getIPAddr() << "\t" 
	 << w->getLabel() << endl;
    }
  }
  leaveCritical();
  return ss.str();
}

