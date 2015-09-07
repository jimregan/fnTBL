// This is a -*- C++ -*- file !!
/*
  Defines a character splitter, much like the perl s///, but it
  splits only on a set of characters.

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

#ifndef _line_splitter_h_
#define _line_splitter_h_

#include <cstdlib>
#include <string>
#include <vector>

class line_splitter {
protected:
  unsigned int noWords;
  std::vector<std::string> words;
  std::string separators;
  // if sep_not_included is true, the separators are not included in the result. Otherwise, separators that
  // are not white space are included.
  bool sep_not_included;
  unsigned char seps[256];
public:
  typedef std::vector<std::string>::iterator iterator;
  typedef std::vector<std::string>::const_iterator const_iterator;
  // Constructor : the argument is a string of separator chars - default space and tab
  line_splitter(const std::string& separats = " \t\r\n", bool s=true) :  noWords(1000), words(noWords), separators(separats), sep_not_included(s)
  {
    for(int i=0 ; i<256 ; i++)
      seps[i] = 0;
	  
    for(unsigned i=0 ; i<separators.size() ; i++)
      seps[static_cast<int>(separators[i])] = 1;
  }

  ~line_splitter() {
  }

  bool isseparator(unsigned char c) {
    return seps[c] == 1;
  }

  // splits the line
  void split(const std::string& line);
  
  const std::string& operator[] (int i) const {
#ifdef DEBUG
    assert(i>=0 && static_cast<unsigned>(i)<noWords);
#endif
    return words[i];
  }

  // returns the i-th word in the line
  std::string& operator[] (int i) {
#ifdef DEBUG
    assert(i>=0 && static_cast<unsigned>(i)<noWords);
#endif
    return words[i];
  }

  // returns the number of words in the split line
  int size() {
    return noWords;
  }

  iterator begin() {
    return words.begin();
  }

  iterator end() {
    return words.end();
  }

  const_iterator begin() const {
    return words.begin();
  }

  const_iterator end() const {
    return words.end();
  }

  // returns a reference to the vector of words in the split line
  std::vector<std::string>& data() {
    return words;
  }

protected:
  void add_word(char word[]) {
    if(words.size() <= noWords)
      words.push_back(word);
    else
      words[noWords] = word;
    noWords++;
  }
};

#include <ctype.h>
// #include <std/bastring.cc>

inline void line_splitter::split(const std::string& line) {

  int index = -1, length = line.length(), wind=0;
  unsigned char prev = ' ', curr;
  noWords = 0;
  const char * temp = line.c_str();
  static char word[100000];
  word[0] = '\0';
  bool prev_separator = true;

  while(++index < length) {
    curr = temp[index];
    if(isseparator(curr)) { // we have a separator here..
      if(prev_separator) {
	if(sep_not_included || isspace(curr))
	  continue;
	word[0] = curr;
	word[1] = '\0';
	wind = 0;
	add_word(word);
      }
      else {
	word[wind] = '\0';
	add_word(word);
	wind = 0;
	if(!sep_not_included && !isspace(curr)) {
	  word[0] = curr; word[1] = '\0';
	  wind = 0;
	  add_word(word);
	}
      }
      prev_separator = true;
    }
    else {                              // Here the current word is not a separator..
      word[wind++] = curr;
      prev_separator = false;
    }
    prev = curr;
  }
  if(wind) {
    word[wind] = '\0';
    add_word(word);
  }    
  words.resize(noWords);
}


#endif                // ! _line_splitter_h_
