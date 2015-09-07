/*
  Defines the implemntation for the Constraint class.

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

#include "Constraint.h"
#include "common.h"
#include "Predicate.h"
#include "line_splitter.h"
#include "Dictionary.h"
#include "Rule.h"

extern int TRUTH;

Constraint::Constraint(const string& line1) {
  line_splitter ls;
  ls.split(line1);
  features.resize(ls.size()-2);
  
  for(int i=0 ; i<ls.size()-2 ; i++)
	features[i] = RuleTemplate::name_map[ls[i]];
  target_feature = RuleTemplate::name_map[ls[ls.size()-2]];

  int sz = ls.size()-2;
  istream* istr;
  smart_open(istr, ls[ls.size()-1]);

  string line;
  static wordTypeVector v1, v2;
  v1.resize(sz);
  
  const Dictionary& dict = Dictionary::GetDictionary();
  bit_vector seen(Dictionary::num_classes+1);

  while (getline(*istr, line)) {
	fill(seen.begin(), seen.end(), false);
	ls.split(line);
	
	bool unknown = false;
	for(int i=0 ; i<sz ; i++) {
	  wordType wid = dict[ls[i]];
	  if(dict.wasUnknown()) {
		unknown = 1;
		break;
	  }
	  v1[i] = wid;
	}

	if(unknown)
	  continue;

	int no_seen = 0;
	for(int i=sz ; i<ls.size() ; i++) {
	  wordType wid = dict[ls[i]];
	  if(wid <= Dictionary::num_classes && ! dict.wasUnknown()) {
		seen[wid] = true;
		no_seen++;
	  }
	}

	// No actual words have been seen
	if (no_seen == 0)
	  continue;

 	constraint[v1] = seen;
  }
}

bool Constraint::test(const wordType1D& feature_vector, const Target& target) const {
  TargetTemplate& templ = TargetTemplate::Templates[target.tid];
  TargetTemplate::pos_vector::iterator p;
  if((p = find(templ.positions.begin(), templ.positions.end(), target_feature-TargetTemplate::TRUTH_START)) ==
	 templ.positions.end()) // The constraint is not on any feature from the current target
	return true;

  static vector<wordType> v;
  initialize_vector(v, feature_vector);

  rep_type::const_iterator it = constraint.find(v);

  return it==constraint.end() || it->second[target.vals[p-templ.positions.begin()]];
}

bool Constraint::test(const wordType1D& feature_vector, int class_id) const {
  static vector<wordType> v;
  initialize_vector(v, feature_vector);

  rep_type::const_iterator it = constraint.find(v);

  return it==constraint.end() || it->second[class_id];
}

void ConstraintSet::read(const string& file_name) {
  istream* istr;
  smart_open(istr, file_name.c_str());
  string line;
  line_splitter ls;

  while (getline(*istr, line)) {
	ls.split(line);
	if(ls.size() == 0 || line[0]=='#')
	  continue;

	constraints.push_back(Constraint(line));
  }

  delete istr;
}
