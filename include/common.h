// -*- C++ -*-
/*
  A set of useful definitions and functions.

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

#ifndef _common_h_
#define _common_h_

#include <string>

#include <hash_wrapper.h>
#include <math.h>

#if __GNUC__ < 3
#include <pfstream.h>
#else
#include <fstream>
#endif
#include <iostream>

namespace HASH_NAMESPACE {
  template<>
  struct hash<std::string> {
    size_t operator() (std::string s) const {
      return __stl_hash_string(s.c_str());
    }
  };
}

int getline(std::istream&, std::string&);
std::string itoa(int);
const std::string& ftoa(double, const std::string& = "%f");
int atoi1(const std::string&);
double atof1(const std::string&);

inline void smart_open(std::istream *& f, std::string str) {
  if(str == "-") 
    f = &std::cin;
  else if(str.size()>3 && str.rfind(".gz") == str.size()-3)
#if __GNUG__<3
    f = new ipfstream(("|zcat "+str).c_str());
#else
  f = new std::ifstream(("|zcat "+str).c_str());
#endif
  else if(str.size()>4 && str.rfind(".bz2") == str.size()-4)
#if __GNUG__<3
    f = new ipfstream(("|bzcat "+str).c_str());
#else
  f = new std::ifstream(("|bzcat "+str).c_str());
#endif
  else
    f = new std::ifstream(str.c_str());

  if(!*f) {
    std::cerr << "Could not open the file " << str << " for reading ! Exiting..." << std::endl;
    exit(111);
  }
}

inline void smart_open(std::ostream*& g, std::string str) {
  if(str=="-")
    g = &std::cout;
  else if(str.size()>3 && str.rfind(".gz") == str.size()-3)
#if __GNUG__<3
    g = new opfstream(("|gzip -c > "+str).c_str());
#else
  g = new std::ofstream(("|gzip -c > "+str).c_str());
#endif
  else if(str.size()>4 && str.rfind(".bz2") == str.size()-4)
#if __GNUG__<3
    g = new opfstream(("|bzip2 -c > "+str).c_str());
#else
  g = new std::ofstream(("|bzip2 -c > "+str).c_str());
#endif
  else
    g = new std::ofstream(str.c_str());

  if(!*g) {
    std::cerr << "Could not open the file " << str << " for writing ! Exiting..." << std::endl;
    exit(112);
  }
}

typedef unsigned short constit_type;

template <class Value>
inline Value sqr(const Value & val) {
  return val*val;
}

#if DEBUG >= 3
#define ON_DEBUG(x) x
#else
#define ON_DEBUG(x)
#endif

inline void tick(int current_line, bool force = false) {
  static unsigned prev_line = 1;
  if(current_line == 1) {
    std::cerr << "Processed lines: 0";
    prev_line = 1;
  }
  
  if((current_line>>6) > ((current_line-1)>>6) || force) {
    for(int p=0 ; p<=log(static_cast<double>(prev_line+1))/log(10.0) ; p++)
      std::cerr << "\b";
    std::cerr << current_line;
    prev_line = current_line;
  }
}

class ticker {
  std::string initial_string;
  int interval;
  int line;
  int prev_line;
  bool displayed;
  std::ostream& stream;

public:
  ticker(const std::string s="Processed lines: ", int i=64, std::ostream& str = std::cerr): 
    initial_string(s), interval(0), line(1), prev_line(0), displayed(false), stream(str) 
  {
    interval = static_cast<int>(log(static_cast<double>(i))/log(2.0));
  }

  void tick(int val = -1, bool force = false) {
    if(val != -1)
      line = val;

    if(! displayed) {
      stream << initial_string << " " << line;
      prev_line = line;
      displayed = true;
    }
	
    if((line>>interval) > ((line-1)>>interval) || force) {
      for(int p=0 ; p<=log(static_cast<double>(prev_line+1))/log(10.0) ; p++)
	stream << "\b";
      stream << line;
      stream.flush();
      prev_line = line;
    }
	
    line++;
  }


  void clear() {
    std::string s=initial_string + itoa(prev_line);
    int l = s.length();
    for(int k=0 ; k<=l ; k++)
      stream << "\b";
    for(int k=0 ; k<=l ; k++)
      stream << " ";
    for(int k=0 ; k<=l ; k++)
      stream << "\b";
	
    line = 1;
    displayed = false;
  }
	
};

void log_me_in(int, char* []);

template<class type, class alloc>
std::ostream& operator <<(std::ostream& ostr, const std::vector<type, alloc>& sv) {
  ostr << sv.size();
  for(int i=0 ; i<sv.size() ; i++)
    ostr << " " << sv[i];
  return ostr;
}

template <class type, class alloc>
std::istream& operator >> (std::istream& istr, std::vector<type, alloc>& sv) {
  int sz;
  istr >> sz;
  sv.resize(sz);
  for(int i=0 ; i<sz ; i++)
    istr >> sv[i];
  return istr;
}

template <class array_type = std::vector<int> >
struct ArrayIndexSorter {
  const array_type& counts;
  ArrayIndexSorter(const array_type& c): counts(c) {}
  
  bool operator() (int i1, int i2) const {
    return counts[i1] < counts[i2];
  }
};

#endif
