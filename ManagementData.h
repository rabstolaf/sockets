/* Interface module for multithreaded server management class
   RAB 5/2/16 rev 12/22/17 */

#include <atomic>
#include <string>

#define MAXWORKERS 10

using namespace std;
class Worker;
class Socket;

/** States for a Worker object */
enum ThreadState { RUNNING, DONE };

/** data structure for managing control of the server */
class ManagementData {
 protected:
  Worker *workers[MAXWORKERS];  /**< shared array of worker threads */
  atomic_flag mgt_flag;  /**< for mutually exclusive access to workers[] */

 public:
  /** continue serving while contin is non-zero.  
      The type atomic<int> prevents "race condition" errors if multiple 
      threads try to access contin at the same time */
  atomic<int> contin; 
  atomic<int> nextID;  /**< next unique ID number for a worker */
  /** shared among all threads */
  string do_run;
  string cuda_arch;
  string jobe_runs;
  string addfacl;

  ManagementData();
  int addWorker(Socket *sockp);
  void cleanupWorkers();
  string getDump();

 protected:
  /** enter a critical section of code for the shared data structure workers[]
      @pre mgt_flag has been initialized, and 
      this thread does not hold mutually exclusive access to workers[] 
      @post This thread holds mutually exclusive access to workers[] */
  void enterCritical() { while (mgt_flag.test_and_set()) {} }

  /** leave a critical section of code for the shared data structure workers[]
      @pre mgt_flag has been initialized, and 
      this thread holds mutually exclusive access to workers[]  
      @post This thread does not hold mutually exclusive access to workers[] */
  void leaveCritical() { mgt_flag.clear(); }

  // helpers - NOT THREAD SAFE - only call within a critical section
  void checkWorker(int indx);

};

