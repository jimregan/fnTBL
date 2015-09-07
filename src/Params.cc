/*
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

#include "Params.h"
#include "line_splitter.h"

using namespace std;

string * Params::nullStr = new string("");
string Params::filename = "";

Params::Params(const char * filename)
{
  if(filename == NULL || string(filename) == "") {
    cerr << "The parameter file is undefined !! Try setting the DDINF environment variable !" << endl;
    exit(1);
  }

  ifstream f(filename);
  
  string line;
  line_splitter lineSplitter("# \r\t=;", false);
  int noWords=0, lineNo=0;
  
  while(getline(f, line)) {
    lineSplitter.split(line);
    if(lineSplitter.size()==0 || lineSplitter[0] == "#" || (noWords = lineSplitter.size()) == 0)
      continue;
    if(noWords != 4) {
      cerr << "Error at line " << lineNo << " in file " << filename << "!!" << endl;
      cerr << "There are " << noWords << " words on line " << lineNo << " instead of 4:" << endl;
      for(int ii=0 ; ii<lineSplitter.size()-1 ; ++ii)
	cerr << lineSplitter[ii] << ",";
      cerr << lineSplitter[lineSplitter.size()-1] << endl;
      exit(20);
    }
    string s = lineSplitter[2];
    unsigned pos;
    while ((pos = s.find("${", 0)) != static_cast<unsigned>(-1)) { // We have a variable ${NAME}
      unsigned pos1 = s.find("}",pos+2);
      if (pos1 == static_cast<unsigned>(-1)) {
	cerr << "Error!! Variable not finished in the parameter file!" << endl;
	exit(1);
      }
      string sub;
      copy(s.begin()+pos+2, s.begin()+pos1, inserter(sub,sub.begin()));
      if (commands.find(sub) == commands.end()) {
	cerr << "Error!! Variable " << sub << "not defined! Exiting..." << endl;
	exit(1);
      }
      string replacement = commands[sub];
      s.replace(pos, pos1-pos+1, commands[sub]);
    }
    commands[lineSplitter[0]] = s;
    lineNo++;
    
  }
  f.close();
}

ostream& operator << (ostream& ostr, const Params& p) {
  for(Params::const_iterator i=p.commands.begin(); i!=p.commands.end() ; i++) 
    ostr << (*i).first << " = " << (*i).second << " ; " << endl;
  return ostr;
}
