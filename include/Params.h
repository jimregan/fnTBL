// -*- C++ -*-
/*
  Implements the Params class, a flexible way to pass parameters to a program,
  through a file.

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

#ifndef _Params_h_
#define _Params_h_

#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "common.h"
#include "hash_wrapper.h"

class Params {
  typedef HASH_NAMESPACE::hash_map<std::string, std::string> rep_type;
  typedef rep_type::iterator iterator;
  typedef rep_type::const_iterator const_iterator;

  rep_type commands;
  static std::string * nullStr;
  static std::string filename;

public:
  Params(const char * str = getenv("DDINF"));

  inline const std::string& operator[] (std::string param) const {
    const_iterator i = commands.find(param);
    if(i == commands.end())
      return *nullStr;
    return (*i).second;
  }

  const std::string& valueForParameter(std::string param) const {
    const_iterator i = commands.find(param);
    if(i == commands.end()) {
      std::cerr << "The parameter " << param << " is not defined in the parameters file ($DDINF)! Exiting.." << std::endl;
      exit(1);
    }
    return (*i).second;
  }

  const std::string valueForParameter(std::string param, std::string default_value) const {
    const_iterator i = commands.find(param);
    if(i == commands.end())
      return default_value;
    else
      return (*i).second;
  }

  template <class Type>
  Type valueForParameter(std::string param, Type default_value) const {
    const_iterator i=commands.find(param);
    if(i==commands.end())
      return default_value;
    else
      return static_cast<Type>(atoi1(i->second));
  }

  iterator begin() {
    return commands.begin();
  }

  const_iterator begin() const {
    return commands.begin();
  }

  iterator end() {
    return commands.end();
  }

  const_iterator end() const {
    return commands.end();
  }

  static void Initialize(const std::string& fn = "") {
    if (fn == "") {
      char *str = getenv("DDINF");
	  
      if(str == 0) {
	std::cerr << "No parameter file was given and the shell variable DDINF is not set up! Exiting.." << std::endl;
	exit(1);
      }
      filename = str;
    } 
    else
      filename = fn;
    nullStr = new std::string("");
  }

  static const Params& GetParams() {
    if (filename=="")
      Initialize();
    static Params p(filename.c_str());
	
    return p;
  }
  
  friend std::ostream& operator << (std::ostream&, const Params&);
};
#endif
