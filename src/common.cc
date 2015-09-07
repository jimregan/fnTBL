/*
  Implements a set of common and useful functions.

  This file is part of the fnTBL distribution.

  Copyright (c) 2001 Johns Hopkins University and Radu Florian and Grace Ngai.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software, fnTBL version 1.0, and associated
  documentation files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use, copy,
  modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished
  to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  
*/

#define _NULL_STRING_LOCAL
#include "common.h"
#include <cstdio>

using namespace std;

int getline(istream& istr, string& line) {
  static char temp[1000000];
  
  if(! istr.getline(temp, 1000000))
    return 0;
  line = temp;
  return 1;
}

int IsBlank(const string& s) {
  for(unsigned i=0 ; i<s.length() && s[i] != '\n'; i++) 
    if(s[i] != ' ' && s[i] != '\t')
      return 0;
  return 1;
}

string itoa(int no) {
  static char s[20];

  sprintf(s, "%d", no);
  return s;
}

const string& ftoa(double f, const string& format) {
  static string r("                                                    ");
  char s[30];
  sprintf(s, format.c_str(), f);
  r = s;
  return r;
}

int atoi1(const string& number) {
  return atoi(number.c_str());
}

double atof1(const string& number) {
  return atof(number.c_str());
}

#include "Params.h"
#include <time.h>
void log_me_in(int argc, char * argv[]) {
  const Params& par = Params::GetParams();
#ifdef DEBUG
  ofstream log_file((par["LOGFILE"]+".debug").c_str(), ios::app);
#else
  ofstream log_file(par["LOGFILE"].c_str(), ios::app);
#endif  

  time_t curtime;
  struct tm *loctime;
  char  buffer[2000];
  /* Get the current time. */
  curtime = time (NULL);  

  /* Convert it to local time representation. */
  loctime = localtime (&curtime);
  
  /* Print it out in a nice format. */
  strftime (buffer, 2000, "%A, %B %d %Y, %H:%M:%S", loctime);
  log_file << buffer << "  ";
  
  for(int i=0 ; i<argc ; i++)
    log_file << argv[i] << " ";
  log_file << endl;
  log_file.close();
}
