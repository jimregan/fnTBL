// -*- c++ -*- 
/*
  Defines the interface for the class Target, which is the representation
  of an internal state of the program.

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

#ifndef _Target_h_
#define _Target_h_

#include "svector.h"
#include <string.h>
#include "Dictionary.h"
#include "typedef.h"
#include "Params.h"
#include "line_splitter.h"

class TargetTemplate {
public:
  typedef std::vector<std::string> string1D;
  typedef std::vector<TargetTemplate> TargetTemplate_vector;
  typedef TargetTemplate self;
  typedef svector<featureIndexType> pos_vector;
  

public:
  static Dictionary name_map;
  static string1D TemplateNames;
  static TargetTemplate_vector Templates;
  static short int STATE_START;
  static short int TRUTH_START;
  static short int TRUTH_SIZE;

public:
  TargetTemplate() {}
  TargetTemplate(const std::vector<std::string>& truth_features);
  TargetTemplate(const self& t): positions(t.positions) {
  }

  static bool value_is_correct(wordType val, wordType truth) {
	static std::string truth_sep = Params::GetParams()["TRUTH_SEPARATOR"];
	if(truth_sep == "") // the simple case - there is only one true value per example
	  return val == truth;
	else {
	  static line_splitter ts(truth_sep);
	  static const Dictionary& dict = Dictionary::GetDictionary();
	  ts.split(dict[truth]);
	  if(ts.size()==1)
		return val == truth;
	  else {
		const std::string& val_str = dict[val];
		for(int i=0 ; i<ts.size() ; i++)
		  if(val_str == ts[i])
			return true;
		return false;
	  }
	}
  }

  scoreType goods(const wordType1D& corpus) const {
	scoreType g = 0;

	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i)
	  g += value_is_correct(corpus[*i+STATE_START], corpus[*i + TRUTH_START]);
	return g;
  }

  scoreType goods(const wordType1D& corpus, const wordTypeVector& words) const {
	scoreType g = 0;
	int pos=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i, ++pos)
	  g += value_is_correct(words[pos], corpus[*i+TRUTH_START]);
	return g;
  }

  scoreType bads(const wordType1D& corpus) const {
	scoreType g=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i)
	  g += ! value_is_correct(corpus[*i+STATE_START], corpus[*i+TRUTH_START]);
	return g;
  }

  scoreType bads(const wordType1D& corpus, const wordTypeVector& words) const {
	scoreType g = 0;
	int pos=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i, ++pos)
	  g += ! value_is_correct(corpus[*i+STATE_START], corpus[*i+TRUTH_START]);
	return g;
  }

  scoreType score(const wordType1D& corpus) const {
	scoreType g=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i)
	  g += 2 * value_is_correct(corpus[*i+STATE_START], corpus[*i+TRUTH_START]) - 1;
	return g;
  }
	
  scoreType score(const wordType1D& corpus, const wordTypeVector& words) const {
	scoreType g = 0;
	int pos=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i, ++pos)
	  g += 2 * value_is_correct(words[pos], corpus[*i+TRUTH_START]) - 1;
	return g;
  }

  bool is_correct(const wordType1D& corpus) const {
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i)
	  if(value_is_correct(corpus[*i+STATE_START], corpus[*i+TRUTH_START]))
		return false;

	return true;
  }

  bool affects(const wordType1D& corpus, const wordTypeVector& insts) const {
	int p=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i, ++p)
	  if(corpus[*i+STATE_START] != insts[p])
		return true;
	return false;
  }

  void instantiate(const wordType1D& corpus, wordType2DVector& instances);

  void apply(wordType1D& corpus, const wordTypeVector& words) const {
	int pos=0;
	for(pos_vector::const_iterator i=positions.begin() ; i!=positions.end() ; ++i, ++pos)
	  corpus[*i+STATE_START] = words[pos];
  }

  std::string printMe(const wordTypeVector& words) {
	static std::string str;
	str = "";
	const Dictionary& dict = Dictionary::GetDictionary();
	static std::string eq = "=", space = " ";
	int pos=0;
	str = TargetTemplate::name_map[positions[0]];
	str.append(eq);
	str.append(dict[words[0]]);
	for(pos_vector::const_iterator i=positions.begin()+1 ; i!=positions.end() ; ++i, ++pos) {
	  str.append(space);
	  str.append(TargetTemplate::name_map[*i]);
	  str.append(eq);
	  str.append(dict[words[pos]]);
	}
	return str;
  }

  static int FindTemplate(const std::string& t_name) {
    string1D::iterator i = find(TemplateNames.begin(), TemplateNames.end(), t_name);
    if (i==TemplateNames.end())
      return -1;
    else
      return distance(TemplateNames.begin(), i);
  }

  void update_counts(const wordType1D& vect, int factor, scoreType& good, scoreType& bad, const wordTypeVector& vals) const {
	int pos=0;
	// We're assuming here that the rule does apply to the sample vect
	for(pos_vector::const_iterator it=positions.begin() ; it!=positions.end() ; ++it, ++pos) {
	  if(value_is_correct(vals[pos], vect[TRUTH_START + *it])) {
		if(! value_is_correct(vect[STATE_START + *it], vect[TRUTH_START + *it]))
		  good += factor;
	  }
	  else
		if(value_is_correct(vect[STATE_START + *it], vect[TRUTH_START + *it]))
		  bad += factor;
	}
  }

  void update_bad_counts(const wordType1D& vect, int factor, scoreType& bad, const wordTypeVector& vals) const {
	int pos=0;
	// We're assuming here that the rule does apply to the sample vect
	for(pos_vector::const_iterator it=positions.begin() ; it!=positions.end() ; ++it, ++pos) {
	  if(value_is_correct(vect[STATE_START + *it], vect[TRUTH_START + *it]) && ! value_is_correct(vals[pos],vect[TRUTH_START + *it])) 
		bad += factor;
	}
  }

public:
  pos_vector positions;
};

class Target {
public:
  typedef Target self;
  explicit Target(int template_id = 0): tid(0), vals(0) {}
  Target(const self& t): tid(t.tid), vals(t.vals) {}
  Target(int template_id, const wordType1D& words): tid(template_id), vals(0) {}
  Target(int template_id, const wordTypeVector& words): tid(template_id), vals(words) {}
  Target(string1D& words) {
	create_from_words(words);
  }

  ~Target() {}

  bool is_correct(const wordType1D& corpus) const {
	return TargetTemplate::Templates[tid].is_correct(corpus);
  }

  bool affects(const wordType1D& corpus) const {
	return TargetTemplate::Templates[tid].affects(corpus, vals);
  }

  void create_from_words(string1D& words);

  void apply(wordType1D& corpus) const {
	TargetTemplate::Templates[tid].apply(corpus, vals);
  }

  int hashVal(int val = 0) const {
	for(unsigned short k=0 ; k<vals.size() ; ++k)
	  val = 7*val + vals[k];
	return val;
  }

  std::string printMe() const {
	return TargetTemplate::Templates[tid].printMe(vals);
  }

  bool operator< (const self& t) const {
	return 
	  t.tid > tid ||
	  t.tid == tid && ( t.vals.size()>vals.size() || 
						t.vals.size() == vals.size() && vals < t.vals
						);
  }

  void update_counts(const wordType1D& vect, int factor, scoreType& good, scoreType& bad) const {
	TargetTemplate::Templates[tid].update_counts(vect, factor, good, bad, vals);
  }

  void update_bad_counts(const wordType1D& vect, int factor, scoreType& bad) const {
	TargetTemplate::Templates[tid].update_bad_counts(vect, factor, bad, vals);
  }

  bool operator== (const self& t) const {
	return tid==t.tid && vals == t.vals;
  }

  self& operator= (const self& t) {
	if(this != &t) {
	  tid = t.tid;
	  vals = t.vals;
	}
	return *this;
  }

public:
  short int tid;
  wordTypeVector vals;
};

#endif
