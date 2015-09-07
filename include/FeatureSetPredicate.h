// -*- C++ -*-
#ifndef _FeatureSetPredicate_h_
#define _FeatureSetPredicate_h_
/*
  
 This type of predicate returns true if one of the features recorded 
 in it has the specified value. It is used to implement a "feature set".
 For instance, it can be used when doing prepositional phrase attachment,
 to record the parents in WordNet of a given word.

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


#include "AtomicPredicate.h"
#include <vector>
#include "Rule.h"
#include "line_splitter.h"

class FeatureSetPredicate : public AtomicPredicate {
public:
  typedef AtomicPredicate super;
  typedef FeatureSetPredicate self;
  typedef vector<unsigned short> feature_vector;

  FeatureSetPredicate() {}
  FeatureSetPredicate(const self& p): features(p.features), pos(p.pos) {}

  FeatureSetPredicate(const feature_vector& f, featureIndexType p): features(f), pos(p) {}

  ~FeatureSetPredicate() {}

  virtual bool test(const wordType2D& corpus, int feature_index, const wordType value) const {
	const wordType1D& vect = corpus[feature_index+pos];

	for(feature_vector::const_iterator i=features.begin() ; i!=features.end() ; ++i)
	  if(vect[*i] == value)
		return true;

	return false;
  }

  virtual double test(const wordType2D& corpus, int feature_index, const wordType value, const float2D& context_prob) const {
	double p = 1.0;

	for (feature_vector::const_iterator i=features.begin() ; i!=features.end() ; ++i)
	  p *= 1-context_prob[feature_index+pos][*i];

	return 1.0 - p;
  }

  virtual void instantiate(const wordType2D& corpus, int feature_index, wordTypeVector& instances) const {
	static bit_vector null_features;
	static bool instantiated = false;
	if (!instantiated) {
	  instantiated = true;
	  const Params& par = Params::GetParams();
	  string null_ftrs;
	  if((null_ftrs = par["NULL_FEATURES"]) != "") {
		line_splitter cs(",");
		cs.split(null_ftrs);
		const Dictionary & dict = Dictionary::GetDictionary();
		null_features.resize(dict.size());
		for(int i=0 ; i<cs.size() ; i++)
		  null_features[dict[cs[i]]] = true;
	  }
	}
	  
	const wordType1D& vect = corpus[feature_index+pos];
	
	if(null_features.size()>0) {
	  static wordTypeVector loc_insts;
	  loc_insts.clear();

	  for(feature_vector::const_iterator i=features.begin() ; i!=features.end() ; ++i)
		if(! null_features[vect[*i]])
		  loc_insts.push_back(vect[*i]);

	  instances.resize(loc_insts.size());
	  copy(loc_insts.begin(), loc_insts.end(), instances.begin());
	}
	else {
	  instances.resize(features.size());
	  wordTypeVector::iterator j = instances.begin();
	  for (feature_vector::const_iterator i=features.begin() ; i!=features.end() ; ++i, ++j)
		*j = vect[*i];
	}
  }

  virtual void identify_strings(const wordType1D& vect, wordType_set& words) const {
	for (feature_vector::const_iterator i=features.begin() ; i!=features.end() ; ++i)
	  words.insert(vect[*i]);
  }

  virtual void identify_strings(wordType word_id, wordType_set& words) const {
  }

  virtual string printMe(wordType wrd) const;
  virtual void set_dependencies(vector<bit_vector>& deps) const {
  }

  virtual void get_feature_ids(storage_vector& fts) const {
	fts.resize(features.size());
	copy(features.begin(), features.end(), fts.begin());
  }

  virtual bool is_indexable() const {
	return true;
  }

  virtual void get_sample_differences(position_vector& positions) const {
	positions.push_back(pos);
  }
  
protected:
  feature_vector features;
  featureIndexType pos;
};

inline string FeatureSetPredicate::printMe(wordType wrd) const {
  const Dictionary& dict = Dictionary::GetDictionary();
  
  static string str, clb = "{";
  str = clb;
  for(int i=0 ; i<features.size()-1 ; i++) {
	str.append(PredicateTemplate::name_map[features[i]]);
	str.append(",");
  }
  str.append(PredicateTemplate::name_map[features.back()]);
  str.append("}");

  if (RuleTemplate::rvariables.find(str) != RuleTemplate::rvariables.end())
	str = RuleTemplate::rvariables[str];

  if(pos>0 || PredicateTemplate::MaxBackwardLookup < 0 || PredicateTemplate::MaxForwardLookup>0) {
	str.append("_");
	str.append(itoa(pos));
  }
  str.append("=");
  str.append(dict[wrd]);
  return str;
}
#endif
