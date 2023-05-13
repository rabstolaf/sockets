#include <iostream>
#include <fstream>
#include "Config.h"

Config::Config(const char *filename) : map() {
    ifstream cfile;
    char buff[maxbuff];  // input buffer
    char *val;  // pointer to hold the value string in a key/value pair
    
    cfile.open(filename, ios::in);
    if (cfile) {
      while (cfile.getline(buff, maxbuff)) {
	if (*buff == '\0' || *buff == '#')
	  continue;
	for (val = buff;  *val != '\0' && *val != '=';  val++)
	  ;
	if (*val == '\0')
	  continue;
	*val++ = '\0';  
	insert(pair<string,string>((const char *)buff, val)); 
      }
    }
    cfile.close();
}

/* replace  skip_Config_test  by  main  for a test program */
#define MAIN skip_Config_test

int MAIN() {
  Config config("execpdc.config");

  map<string,string>::iterator it;
  for (it = config.begin();  it != config.end();  it++)
    cout << it->first << " " << it->second << endl;

  return 0;
}
