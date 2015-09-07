/*
  Defines the implementation for the Predicate class.

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

#include "Predicate.h"
#include "Params.h"
#include "line_splitter.h"
#include "SingleFeaturePredicate.h"
#include "PrefixSuffixAddPredicate.h"
#include "PrefixSuffixRemovePredicate.h"
#include "PrefixSuffixIdentityPredicate.h"
#include "ContainsStringPredicate.h"
#include "FeatureSequencePredicate.h"
#include "CooccurrencePredicate.h"
#include "FeatureSetPredicate.h"
#include "common.h"
#include "Rule.h"

extern wordType UNK;
extern featureIndexType TRUTH;
extern featureIndexType STATE;

Dictionary PredicateTemplate::name_map;
string1D PredicateTemplate::TemplateNames;
vector<PredicateTemplate> PredicateTemplate::Templates;
sized_memory_pool<Predicate::order_rep_type> Predicate::memory_pool(100);
HASH_NAMESPACE::hash_map<string, string> PredicateTemplate::variables;

relativePosType PredicateTemplate::MaxBackwardLookup = 0;
relativePosType PredicateTemplate::MaxForwardLookup = 0;

class PredicateTemplateDeallocator {
public:
  ~PredicateTemplateDeallocator() {
    for(int i=0 ; i<PredicateTemplate::Templates.size() ; i++)
      PredicateTemplate::Templates[i].deallocate();
  }
};

PredicateTemplateDeallocator deallocator;

void PredicateTemplate::Initialize() {
}

//   The format of a template is as follows:

//   <feature_name> := [A-Za-z0-9]+
//   <value>        := [A-Za-z0-9]+
//   <number>       := -?[0-9]+
//   <template_unit> := <feature_name>_<number> | <feature_name>_<number>:[<number>,<number>]
//   <template> :=  <template_unit> [<template_unit> ...]

//   Examples:

//   noun_-1     means the feature with the name "noun", situated in 
//               the previous sample (assumes interdependence between samples)
//   prep_-1:2   means the feature with the name "prep" appeared in 
//               the previous sample, the current sample or one of the next 2 samples.
//   Obviously, the characters '_' and ':' should not be part of any name. 
//   Otherwise, any non-space character is allowed in constructing the names.

//    We added the prefix/suffix rules to the list of accepted rules.
//    They have modified the <template> as follows:

//    <template_unit> := <feature_name> | <feature_name>_<number> | 
//                       <feature_name>_<number>:<number> |
// 					  <feature_name>::<number>++ | // allows prefixation with <number> chars
// 					  <feature_name>::++<number> | // allows suffixation with <number> chars
// 					  <feature_name>::<number>-- | // returns true if removing <number> chars from the beginning results in a number
// 					  <feature_name>::--<number> | // returns true if removing <number> chars from the end results in a number
// 					  <feature_name>::<number>~~ | // returns true if the first <number> chars are specified
// 					  <feature_name>::~~<number> | // returns true iff the last <number> chars are specified
//                    <feature_name>::<number><> | // returns true iff the string contains the specified substring 
//                    <feature_name>^^<number>      // returns true iff the corresponding feature appears with the specified feature anywhere 
//                                                 // in the training data (not necessarily in this position).
//                    {<f_n>[,<f_n>]}_<number>   | returns true if one of the features have the specified value

//    Of course, the rules' parsing becomes more difficult.

PredicateTemplate::PredicateTemplate(const string1D& ls) {
  static line_splitter us("_"); // underscore splitter
  static line_splitter cs(":"); // colon splitter
  static line_splitter ccs("::");

  tests.resize(ls.size());
  tests.clear();
  string::size_type pos, pos1;

  AtomicPredicate* pred;
  string feature;

  for(string1D::const_iterator i=ls.begin() ; i!=ls.end() ; ++i) {
    // If the feature name starts with a $ sign, then the feature name is an alias
    if((*i)[0] == '$')
      feature = RuleTemplate::variables[i->substr(1)];
    else
      feature = *i;

    if ((pos=feature.find("::")) != feature.npos) { // We have a PrefixSuffixPredicate
      static string f_name;
      relativePosType rel_index = 0;

      pos1 = feature.find("_");
      if(pos1 == feature.npos) // No relative distance in feature index
	f_name = feature.substr(0, pos);
      else
	if(pos1<pos) {
	  f_name = feature.substr(0, pos1);
	  rel_index = atoi1(feature.substr(pos1, pos-pos1));
	  if (rel_index != 0) {
	    if(rel_index < MaxBackwardLookup)
	      MaxBackwardLookup = rel_index;
	    if(rel_index > MaxForwardLookup)
	      MaxForwardLookup = rel_index;
	  }
	}

      if(name_map.find(f_name) == name_map.end()) {
	cerr << "Error reading template: " << endl;
	copy(ls.begin(), ls.end(), ostream_iterator<string>(cerr, " "));
	cerr << endl << "Feature name " << f_name << " was not defined in the feature name file!!" << endl;
	exit(2);
      }
      featureIndexType index = name_map[f_name];

      if((pos1 = feature.find("++",pos+2)) != feature.npos) {
	// We have a PrefixSuffixAddPredicate
	if(pos1 == pos+2)
	  // We have a suffix type of PrefixSuffixAddPredicate
	  pred = new PrefixSuffixAddPredicate(rel_index, index, false, atoi1(feature.substr(pos+4)));
	else
	  pred = new PrefixSuffixAddPredicate(rel_index, index, true, atoi1(feature.substr(pos+2,pos1-pos)));
      }
      else if( (pos1 = feature.find("--", pos+2)) != feature.npos) {
	if(pos1 == pos+2) 
	  pred = new PrefixSuffixRemovePredicate(rel_index, index, false, atoi1(feature.substr(pos+4)));
	else
	  pred = new PrefixSuffixRemovePredicate(rel_index, index, true, atoi1(feature.substr(pos+2,pos1-pos)));
      }
      else if ((pos1 = feature.find("~~", pos+2)) != feature.npos) {
	if(pos1 == pos+2) 
	  pred = new PrefixSuffixIdentityPredicate(rel_index, index, false, atoi1(feature.substr(pos+4)));
	else
	  pred = new PrefixSuffixIdentityPredicate(rel_index, index, true, atoi1(feature.substr(pos+2,pos1-pos)));
      }
      else if ((pos1 = feature.find("<>", pos+2)) != feature.npos)
	pred = new ContainsStringPredicate(rel_index, index, atoi1(feature.substr(pos+2,pos1-pos-2)));
      else {
	cerr << "Unknown rule: " << feature << "!" << endl 
	     << "Please remove or correct it. (Maybe it has extra spaces at the end?)" << endl;
	exit(3);
      }
      tests.push_back(pred);
      static_cast<PrefixSuffixPredicate*>(pred)->addToList();
    }
    else if ((pos = feature.find("^^")) != feature.npos) {
      // Generate a predicate that tests for co-occurrence.
      static string temp, temp1;
      temp.assign(feature, 0, pos);
      temp1.assign(feature, pos+2, feature.size());
		
      if(name_map[temp] == name_map.unknownIndex()) {
	cerr << "The feature " << temp << " is not present in the file template. Please correct the problem!" << endl;
	exit(11);
      }
		
      tests.push_back(new CooccurrencePredicate(atoi1(temp1), name_map[temp]));
    }
    else if ((pos=feature.find("{")) != feature.npos) {
      static string comma=",", cbracket = "}", ul="_", eq = "=", tmp;
      string::size_type pos1 = feature.find(comma, pos+1), pos11 = pos+1;
      string::size_type pos2 = feature.find(cbracket, pos+1);

      FeatureSetPredicate::feature_vector featrs;
      while (pos1 != feature.npos && pos1 <= pos2) {
	tmp.assign(feature, pos11, pos1-pos11);
	FeatureSetPredicate::feature_vector::value_type v = PredicateTemplate::name_map[tmp];
	if (v==PredicateTemplate::name_map.unknownIndex()) {
	  cerr << "The feature " << feature.substr(pos11, pos1) << " is not present in the file template. Please correct the problem!" << endl;
	  exit(11);
	}
	featrs.push_back(v);
	pos11 = pos1+1;
	pos1 = feature.find(comma, pos1+1);
      }

      pos1 = feature.find(cbracket, pos1+1);
      tmp.assign(feature, pos11, pos1-pos11);
      FeatureSetPredicate::feature_vector::value_type v = PredicateTemplate::name_map[tmp];
      if (v==PredicateTemplate::name_map.unknownIndex()) {
	cerr << "The feature " << feature.substr(pos11, pos1) << " is not present in the file template. Please correct the problem!" << endl;
	exit(11);
      }
      featrs.push_back(v);

      relativePosType p;
      if(feature[pos2+1] == '_') {
	tmp.assign(feature, pos2+2, feature.size());
	p = atoi1(tmp);
      }
      else
	p = 0;

      pred = new FeatureSetPredicate(featrs, p);
      tests.push_back(pred);	  
    }
    else if ((pos=feature.find(":[")) != feature.npos) {
      // Generate a range atomic predicate
      static string temp, temp1, temp2, comma = ",", bracket = "]";
      temp.assign(feature, 0, pos); // This is the feature name
      string::size_type pos1 = feature.find(comma, pos+2), pos2;
      temp1.assign(feature, pos+2, pos1);
      pos2 = feature.find(bracket, pos1);
      temp2.assign(feature, pos1+1, pos2);
      relativePosType minp = atoi1(temp1), maxp = atoi1(temp2);
      if(minp < MaxBackwardLookup)
	MaxBackwardLookup = minp;

      if(maxp > MaxForwardLookup)
	MaxForwardLookup = maxp;
      tests.push_back(new FeatureSequencePredicate(minp, maxp, name_map[temp]));
    } else {
      us.split(feature);
      relativePosType rel_index;
      if(us.size() == 1)
	rel_index = 0;
      else
	rel_index = atoi1(us[1]);
		
      if (rel_index != 0) {
	if(rel_index < MaxBackwardLookup)
	  MaxBackwardLookup = rel_index;
	if(rel_index > MaxForwardLookup)
	  MaxForwardLookup = rel_index;
      }
		
      if(name_map.find(us[0]) == name_map.end()) {
	cerr << "Error reading template: " << endl;
	copy(ls.begin(), ls.end(), ostream_iterator<string>(cerr," ")); 
	cerr << endl << "Feature name " << us[0] << " was not defined in the feature name file!!" << endl;
	exit(2);
      }
      pred = new SingleFeaturePredicate(rel_index, name_map[us[0]]);
      tests.push_back(pred);
    }
  }
  relativePosType l = -MaxBackwardLookup;
  if(l<MaxForwardLookup)
    l = MaxForwardLookup;
  MaxForwardLookup = l;
  MaxBackwardLookup = -l;
}

// create all possible instantiations of the current template
void PredicateTemplate::instantiate(const wordType2D& corpus, int sample_ind, wordType2DVector& instances) const {
  static wordType2DVector feature_vector;

  feature_vector.resize(tests.size());
  for(int i=0 ; i<tests.size() ; i++) {
    feature_vector[i].clear();
    tests[i]->instantiate(corpus, sample_ind, feature_vector[i]);
    if(feature_vector[i].size()==0)
      return;
  }

  vector<wordTypeVector::iterator> iterators(feature_vector.size());

  for(int i=0 ; i<tests.size() ; i++)
    iterators[i] = feature_vector[i].begin();

  int pos=0;
  wordTypeVector temp(tests.size());

  while (pos>=0) {
    if(pos==feature_vector.size()) {
      instances.push_back(temp);
      pos--;
    } 
    else {
      if(iterators[pos] == feature_vector[pos].end()) {
	iterators[pos] = feature_vector[pos].begin();
	pos--;
      } 
      else {
	temp[pos] = *iterators[pos];
	iterators[pos]++;
	pos++;
      }
    }
  }

  wordType2DVector temp_v(instances);
  instances.swap(temp_v);	  
}

void PredicateTemplate::identify_strings(const wordType1D& word_id, wordType_set& words) const {
  for(int i=0 ; i<tests.size() ; i++)
    tests[i]->identify_strings(word_id, words);
}

void PredicateTemplate::identify_strings(wordType word_id, wordType_set& words) const {
  for(int i=0 ; i<tests.size() ; i++)
    tests[i]->identify_strings(word_id, words);
}

// // ruleComponents are the strings contained in the output of a rule. The last one is the target
// // of the rule, so it should be ignored when creating the predicate.
// // The format of a rule is:
// // <template_unit>=<value> [<template_unit>=value...] => target
// // and, therefore, the second to last element should be ignored too.
void Predicate::create_from_words(string1D& ruleComponents){
  tokens.resize(ruleComponents.size());
  static line_splitter es("=",false); // equality splitter
  int num_features = static_cast<int>(ruleComponents.size());
  featureIndexType1D name_ids(num_features);
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

    if(s1[0] == '$') {
      static string s3;
      s3.assign(s1, 1, s1.size());
      s1 = RuleTemplate::variables[s3];
    }

    if(template_name == "")
      template_name = s1;
    else
      template_name += " " + s1;
 
    tokens[i] = dict.insert(s2);
  }

  template_id = PredicateTemplate::FindTemplate(template_name);
  if(template_id == -1) {
    cerr << "The predicate ";
    copy(ruleComponents.begin(), ruleComponents.end(), ostream_iterator<string>(cerr, " "));
    cerr << " does not have a recognizable template!" << endl << "Please remove it or add a corresponding template!" << endl;
    exit(3);
  }
  
  create_order();
  hashIndex = hashVal();
}
