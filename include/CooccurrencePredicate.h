// -*- C++ -*-
/*
  Defines an atomic predicate type: a type of bag-of-words question.
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

#ifndef __CooccurrencePredicate_h__
#define __CooccurrencePredicate_h__

#include "AtomicPredicate.h"
// #include "trie.h"
#include "hash_wrapper.h"
#include "typedef.h"
#include "indexed_map.h"
#include "common.h"

namespace HASH_NAMESPACE {
  template <>
  struct hash<std::pair<featureIndexType, relativePosType> > {
    size_t operator() (const std::pair<featureIndexType, relativePosType>& val) const {
      return 256*val.first + val.second;
    }
  };
}

class CooccurrencePredicate: public AtomicPredicate {
public:
//   typedef trie<wordType, wordTypeVector> word_trie;
  typedef HASH_NAMESPACE::hash_map<wordType, wordTypeVector> word_map;
  typedef CooccurrencePredicate self;
  typedef AtomicPredicate super;

public:
  CooccurrencePredicate(relativePosType r, featureIndexType fid): pos(r), feature_id(fid) {	
	values.insert(make_pair(feature_id, r));
  }

  CooccurrencePredicate(const self& p): 
	pos(p.pos),
	feature_id(p.feature_id)
  {}

  virtual ~CooccurrencePredicate() {}

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
	wordType key = corpus[sample_ind][feature_id];
	word_map& ngram = tries[word_map_index[make_pair(feature_id, pos)]];
	
	word_map::iterator i = ngram.find(key);
	if(i != ngram.end()) {
	  word_map::const_iterator::value_type p = *i;
	  return binary_search(p.second.begin(), p.second.end(), value);
	}
	else
	  return false;
  }

  virtual double test(const wordType2D& corpus, int sample_ind, const wordType value, const float2D& probs) const {
	return 1.0 * (value==corpus[sample_ind+pos][feature_id]);
  }

  self& operator= (const self& pred) {
	if(this != &pred) {
	  pos = pred.pos;
	  feature_id = pred.feature_id;
	}
	return *this;
  }

  void identify_strings(const wordType1D& word_id, wordType_set& words) const;
  void identify_strings(wordType word_id, wordType_set& words) const;
  void instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const;
  void get_sample_differences(position_vector& positions) const {
	positions.push_back(pos);
  }

  string printMe(wordType sample) const;

  void get_feature_ids(storage_vector& features) const {
	features.push_back(feature_id);
  }

  void set_dependencies(vector<bit_vector>& dep) const {
  }

  bool is_indexable() const {
	return false;
  }

  static void Initialize(const string& rule_file = "");
  static bool IsInitialized;
  typedef pair<featureIndexType, relativePosType> index_pair;
  static indexed_map<index_pair, short int > word_map_index;
  static set<index_pair> values;
  static vector<word_map> tries;
protected:
  relativePosType pos;
  featureIndexType feature_id;
};

template <class type>
ostream& operator <<(ostream& ostr, const vector<type>& sv) {
  ostr << sv.size();
  for(int i=0 ; i<sv.size() ; i++)
	ostr << " " << sv[i];
  return ostr;
}

template <class type>
istream& operator >> (istream& istr, vector<type>& sv) {
  int sz;
  istr >> sz;
  sv.resize(sz);
  for(int i=0 ; i<sz ; i++)
	istr >> sv[i];
  return istr;
}


#endif
