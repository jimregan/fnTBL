/*
  Implements the Target class - the representation of a TBL state

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

#include "Target.h"
#include "line_splitter.h"

short int TargetTemplate::STATE_START = 0;
short int TargetTemplate::TRUTH_START = 0;
short int TargetTemplate::TRUTH_SIZE  = 0;
string1D TargetTemplate::TemplateNames;
TargetTemplate::TargetTemplate_vector TargetTemplate::Templates;
Dictionary TargetTemplate::name_map;
using namespace std;

TargetTemplate::TargetTemplate(const vector<string>& truth_features): positions(truth_features.size()) {
  for(int i=0 ; i<truth_features.size() ; i++)
    positions[i] = name_map[truth_features[i]];
}

// Instantiate all the possible values for the targets. Usually, there is only one true value per example,
// but we implemented an extended version - it might be possible to have multiple true values, separated
// by 1 character, stored in the parameter TRUTH_SEPARATOR.
void TargetTemplate::instantiate(const wordType1D& corpus, wordType2DVector& instances) {
  static string truth_sep = Params::GetParams()["TRUTH_SEPARATOR"];
  if(truth_sep.size()>1) 
    cerr << "The TRUTH_SEPARATOR variable has more than 1 character !! This will probably crash the program!!" << endl;

  if (truth_sep == "") {
    instances.resize(1);
    instances[0].resize(positions.size());
    instances[0].clear();
    for(pos_vector::iterator i=positions.begin() ; i!=positions.end() ; ++i)
      instances[0].push_back(corpus[TRUTH_START + *i]);
  } 
  else {
    static wordType2DVector targets;
    targets.resize(positions.size());
    // Big assumption here: the TRUTH_SEPARATOR variable has only 1 character !!
    static line_splitter ts(truth_sep);
    int pos = 0, prod = 1;
    static const Dictionary& dict = Dictionary::GetDictionary();
    for(pos_vector::iterator i=positions.begin() ; i!=positions.end() ; ++i, ++pos) {
      ts.split(dict[corpus[TRUTH_START + *i]]);
      targets[pos].resize(ts.size());
      targets[pos].clear();
      for(int i=0 ; i<ts.size() ; i++)
	targets[pos].push_back(dict[ts[i]]);
      prod *= ts.size();
    }

    static wordTypeVector vals;
    vals.resize(positions.size());
    pos = 0;
    instances.resize(prod);
    instances.clear();
    static vector<wordTypeVector::iterator> iters;
    iters.resize(positions.size());
    for(int i=0 ; i<targets.size() ; i++)
      iters[i] = targets[i].begin();

    // Backtrack to obtain all possible combinations of values
    while (pos>-1) {
      if(pos == targets.size()) {
	instances.push_back(vals);
	pos--;
      }
      else {
	if(iters[pos] == targets[pos].end()) {
	  iters[pos] = targets[pos].end();
	  pos--;
	}
	else {
	  vals[pos] = *iters[pos];
	  iters[pos]++;
	  pos++;
	}
      }
    }
  }
}

void Target::create_from_words(string1D& ruleComponents) {
  vals.resize(ruleComponents.size());
  static line_splitter es("=",false); // equality splitter
  int num_features = static_cast<int>(ruleComponents.size());
  vector<featureIndexType> name_ids(num_features);
  string template_name = "";
  static Dictionary& dict = Dictionary::GetDictionary();

  for(int i=0 ; i<num_features ; i++) {
    string::size_type p = ruleComponents[i].find("=");
    if(p == string::npos) {
      cerr << "The string " << ruleComponents[i] << " does not have an equality sign in it - the rule is corrupt." << endl;
      exit(1);
    }

    string 
      s1 = ruleComponents[i].substr(0, p),
      s2 = ruleComponents[i].substr(p+1);

    if(template_name == "")
      template_name = s1;
    else
      template_name += " " + s1;
 
    vals[i] = dict.insert(s2);
  }

  tid = TargetTemplate::FindTemplate(template_name);
  if(tid == -1) {
    cerr << "The predicate ";
    copy(ruleComponents.begin(), ruleComponents.end(), ostream_iterator<string>(cerr, " "));
    cerr << " does not have a recognizable template!" << endl << "Please remove it or add a corresponding template!" << endl;
    exit(3);
  }  
}
