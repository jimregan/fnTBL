// -*- C++ -*-
/*
  The class timer implements a simple interface to computing time.
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

#ifndef __timer__h__
#define __timer__h__

#include <vector>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

class timer {
public:
  timer() {
	tz.tz_minuteswest = 140;
	gettimeofday(&tv_orig, &tz);
	tv = tv_last = tv_orig;
  }

  ~timer() {}

  void mark() {
	tv_last = tv;
	gettimeofday(&tv, &tz);
  }

  const string& time_since_last_mark() {
	return time_between(tv, tv_last);
  }

  const string& time_since_beginning() {
	return time_between(tv, tv_orig);
  }

  unsigned int milliseconds_since_last_mark() {
	return milliseconds_between(tv, tv_last);
  }

  unsigned int seconds_since_last_mark() {
	return seconds_between(tv, tv_last);
  }

  unsigned int milliseconds_since_beginning() {
	return milliseconds_between(tv, tv_orig);
  }

  unsigned int seconds_since_beginning() {
	return seconds_between(tv, tv_orig);
  }

private:
  unsigned long milliseconds_between(struct timeval& t1, struct timeval& t2) {
	return 1000*(t1.tv_sec-t2.tv_sec) + (t1.tv_usec-t2.tv_usec)/1000;
  }

  unsigned long seconds_between(struct timeval& t1, struct timeval& t2) {
	return static_cast<unsigned long>(floor(milliseconds_between(t1, t2) / 1000 + 0.5));
  }

  const string& time_between(struct timeval& t1, struct timeval& t2) {
	static const long usec_int = 1;
	static const long sec_int = 1000;
	static const long min_int = 60*sec_int;
	static const long hour_int = 60*min_int;
	static const long day_int = 24*hour_int;
	static const long week_int = 7*day_int;

	static int vals[] = {week_int, day_int, hour_int, min_int, sec_int, usec_int};
	static vector<string> names;
	if(names.size() == 0) {
	  names.resize(6);
	  names[0] = "week";
	  names[1] = "day";
	  names[2] = "hour";
	  names[3] = "minute";
	  names[4] = "second";
	  names[5] = "millisecond";
	}
	static string str;
	str = "";
	long millis = 1000*(t1.tv_sec-t2.tv_sec) + (t1.tv_usec-t2.tv_usec)/1000;

	for(int i=0 ; i<names.size() ; i++) {
	  int val = millis / vals[i];
	  if(val > 0)
		str += (str != "" ? ", " : "") + itoa(val) + " " + names[i] + (val>1 ? "s" : "");
	  millis -= val*vals[i];
	}

	if(str == "") {
	  int micros = 1000000*(t1.tv_sec-t2.tv_sec) + (t1.tv_usec-t2.tv_usec);
	  str = itoa(micros) + " microsecond" + (micros > 0 ? "s" : "");
	}
	return str;
  }

  string itoa(int no) {
	static char s[20];
	sprintf(s, "%d", no);
	return s;
  }
	
private:
  struct timeval tv_orig, tv_last, tv;
  struct timezone tz;
};

#endif
