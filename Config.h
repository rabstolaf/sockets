#include <map>
#include <string>
using namespace std;

/* configuration files should consist of lines of the form
       key=value
   blank lines, comment lines starting with #, and lines without = are skipped*/

class Config : public map<string,string> {
private:
  int maxbuff = 200;  // max length of input line, including newline
  
public:
  Config() : map() {}
  Config(const char *fname) : map() { processFile(fname); }
  void processFile(const char *); 
  string valueOrEnv(const char *key, const char *envName);
};
    
