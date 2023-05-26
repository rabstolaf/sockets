/* add all-access file acl to a file or directory to a particular user
   for RSBE project, RAB May 2023 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>

using namespace std;

int main(int argc, char**argv) {
  const char *prog = argv[0]; 
  if (argc < 3) {
    string msg("Usage:  ");
    msg = msg + prog + " file user";
    cerr << msg << endl;
    return 1;
  }

  // DEBUG
  string cmd0("getpcaps ");
  cmd0 = cmd0 + to_string(getpid()) + " > /tmp/rab.tmp2 ";
  system(cmd0.c_str());  
  system("id > /tmp/rab.tmp3");  

  string cmd("setfacl -m ");
  cmd = cmd + "u:" + argv[2] + ":rwx " + argv[1];
  int status = system(cmd.c_str());
  if (status < 0) {
    string msg(prog);
    msg = msg + ": command failed\n" + cmd + "\nAborting";
    cerr << msg << endl;
    return 1;
  }

  return 0;
}
