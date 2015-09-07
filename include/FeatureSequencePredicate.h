// -*- C++ -*-
/*
  Implements an atomic predicate type: returns true if the feature appears
  in a sequence of positions around the current position.

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

#ifndef __FeatureSequencePredicate__h__
#define __FeatureSequencePredicate__h__

#include "AtomicPredicate.h"
#include "typedef.h"
#include "Dictionary.h"
#include "Rule.h"

class FeatureSequencePredicate : public AtomicPredicate {
public:
  typedef AtomicPredicate super;
  typedef FeatureSequencePredicate self;

public:
  FeatureSequencePredicate(relativePosType samp_st, relativePosType samp_end, featureIndexType fid):
	sample_start(samp_st),
	sample_end(samp_end),
	feature_id(fid)
  {}

  FeatureSequencePredicate(const self& p):
	sample_start(p.sample_start),
	sample_end(p.sample_start),
	feature_id(p.feature_id)
  {}

  virtual ~FeatureSequencePredicate() {}

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
	for(relativePosType i=sample_start ; i<=sample_end ; ++i)
	  if(corpus[i+sample_ind][feature_id] == value)
		return true;
	return false;
  }

  virtual double test(const wordType2D& corpus, int sample_ind, const wordType value, const float2D& probs) const {
	double prob = 1.0;
	for(relativePosType pos=sample_start ; pos<=sample_end ; ++pos)
	  prob *= (1.0-probs[pos+sample_start][value]);
	return 1.0 - prob;
  }

  virtual string printMe(wordType instance) const;

  self& operator= (const self& pred) {
	if(this != &pred) {
	  sample_start = pred.sample_start;
	  sample_end = pred.sample_end;
	  feature_id = pred.feature_id;
	}
	return *this;
  }

  void instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
	instances.resize(sample_end-sample_start+1);
	for(relativePosType pos=sample_start ; pos<=sample_end ; ++pos)
	  instances[pos-sample_start] = corpus[pos+sample_ind][feature_id];
  }

  void identify_strings(const wordType1D& word_id, wordType_set& words) const {
	words.insert(word_id[feature_id]);	
  }

  void identify_strings(wordType word_id, wordType_set& words) const {
	words.insert(word_id);
  }

  void get_sample_differences(position_vector& positions) const {
	for(relativePosType pos=sample_end ; pos>=sample_start ; --pos) 
	  positions.push_back(pos);
  }

  void get_feature_ids(storage_vector& features) const {
	features.push_back(feature_id);
  }

  void set_dependencies(vector<bit_vector>& dep) const;

  bool is_indexable() const { 
	return true;
  }

protected:
  relativePosType sample_start, sample_end;
  featureIndexType feature_id;
};

inline string FeatureSequencePredicate::printMe(wordType instance) const {
  Dictionary& dict = Dictionary::GetDictionary();
  static string str;
  str.assign(PredicateTemplate::name_map[feature_id] + ":[" + itoa(sample_start) + "," + itoa(sample_end) + "]");
  if(RuleTemplate::rvariables.find(str) != RuleTemplate::rvariables.end())
	str = RuleTemplate::rvariables[str];
  str.append("=");
  str.append(dict[instance]);
  return str;
}

inline void FeatureSequencePredicate::set_dependencies(vector<bit_vector>& dep) const {
  for(relativePosType pos = sample_start ; pos<=sample_end ; ++pos) 
	dep[pos-PredicateTemplate::MaxBackwardLookup][feature_id] = true;
}

#endif
