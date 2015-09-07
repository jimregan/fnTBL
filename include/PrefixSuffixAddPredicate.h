// -*- C++ -*-
/*
  This is an extension of the PrefixSuffixPredicate that returns true iff the adding
  a specific prefix/suffix to a given word results in another word in the dictionary.
  Example:
  order + ly = orderly (word = order, suffix = ly) => true
  un + ordered = unordered (word = ordered, prefix = un) => true
  bad + ed = baded (word = bad, suffix = ed) => false
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

#ifndef __PrefixSuffixAddPredicate_h__
#define __PrefixSuffixAddPredicate_h__



#include "PrefixSuffixPredicate.h"
#include "Dictionary.h"
#include "Params.h"
#include "svector.h"

class PrefixSuffixAddPredicate: public PrefixSuffixPredicate {
  typedef PrefixSuffixAddPredicate self;
  typedef PrefixSuffixPredicate super;

public:
  typedef Dictionary::word_trie word_trie;
  typedef svector<wordType> wordType_svector;

public:
  PrefixSuffixAddPredicate(relativePosType sample, storage_type feature, bool is_p = false, char length = 1): 
    super(sample, feature, is_p, length) {}

  PrefixSuffixAddPredicate(const self& pred): super(pred)
  {}

  virtual ~PrefixSuffixAddPredicate() {}

  virtual bool test(const wordType2D& corpus, int sample_ind, const wordType value) const {
    const Dictionary& dict = Dictionary::GetDictionary();
    const string& xfix = dict[value];
    ON_DEBUG(
	     assert (len+2==xfix.size())
	     );
    const string& word = dict[corpus[sample_difference+sample_ind][feature_id]];

    static string str;
    str.resize(0);

    if(is_prefix) {
      str.append(xfix, 0, len);
      str += word;
    } else {
      str = word;
      str.append(xfix, 2, len);
    }

    return dict.find(str) != dict.end();
  }

  self& operator= (const self& pred) {
    super::operator= (pred);
    return *this;
  }

  virtual string printMe(wordType instance) const {
    Dictionary& dict = Dictionary::GetDictionary();
    const string& addition = dict[instance];
    string add_size = itoa(len);
    if(max(-PredicateTemplate::MaxBackwardLookup, +PredicateTemplate::MaxForwardLookup) == 0)
      return PredicateTemplate::name_map[feature_id] + "::" + 
	(is_prefix ? add_size + "++" : "++" + add_size) + "=" + addition;
    else
      return PredicateTemplate::name_map[feature_id] + "_" + 
	itoa(sample_difference) + "::" + 
	(is_prefix ? add_size + "++" : "++" + add_size) + "=" + addition;
  }


  void instantiate(const wordType2D&, int sample_ind, wordTypeVector& instances) const;
  void identify_strings(wordType word_id, wordType_set& word_ids) const;  

  static word_list_rep_type feature_lookup;
  static bool2D seen;

  static void Initialize(int size) {
    char sn[20];
    fill(sn, sn+20, 0);
    for(rep_vector::iterator i=feature_len_pair_list.begin() ; 
	i!=feature_len_pair_list.end() ; 
	++i) {
      sn[static_cast<unsigned>(i->second)] = 1;
    }

    int max = 0;
    for(int i=0 ; i<20 ; i++)
      if(sn[i]!=0)
	max = i;

    feature_lookup.resize(max);
    for(int i=0 ; i<max ; i++)
      feature_lookup[i].resize(size);

    seen.resize(max);
    for(int i=0 ; i<max ; i++) {
      seen[i].resize(size);
      fill(seen[i].begin(), seen[i].end(), false);
    }
    // 	super::Initialize(size, feature_lookup, seen);
  }

  static void Destroy() {
    word_list_rep_type tmp_lst;
    feature_lookup.swap(tmp_lst);
    bool2D bool_tmp;
    seen.swap(bool_tmp);
  }

};
    
inline void PrefixSuffixAddPredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
  Dictionary& dict = Dictionary::GetDictionary();
  wordType word_id = corpus[sample_ind+sample_difference][feature_id];
  const string& word = dict[word_id];
  string::size_type word_len = word.size();
  //   if(word_len<len)
  // 	return;
  static vector<char> v;
  v.resize(word_len);

  static bool cache_word_lists = Params::GetParams().valueForParameter("CACHE_INTERNAL_WORD_LIST", true)==true;
  
  if(cache_word_lists && word_id < seen[len-1].size() && seen[len-1][word_id]) {
    wordType_svector& vect = feature_lookup[len-1][word_id];
    for(wordType_svector::iterator i=vect.begin() ; i!=vect.end() ; ++i) {
      if(is_prefix) {
	if(dict[*i][0] != '+')
	  instances.push_back(*i);
      } else {
	if(dict[*i][0] == '+')
	  instances.push_back(*i);
      }
      ON_DEBUG(assert(dict[*i].size()==len+2));
    }
  }
  else {
    if(is_prefix) { 
      // Here we are trying to identify all prefixes that added to the word will result in
      // a valid word, present in the dictionary. For this, we will be using a trie containing
      // the words in the dictionary, input in reverse order of their characters.

      const word_trie& reverse_trie = dict.get_reverse_trie();

      for(int i=0 ; i<word_len ; i++)
	v[i] = word[word_len-i-1];
	
      word_trie::const_iterator tp=reverse_trie.find(v), tp1 = tp;

      while(1) {
	if(++tp == reverse_trie.end()) {
	  return;
	}
	word_trie::const_iterator p2 = tp;
	const word_trie::level_iterator::sequence_type& vect = (*tp).first;

	p2 = reverse_trie.parent_of_iterator(p2);
	for(int l=word_len+1 ; l<vect.size() ; l++)
	  p2 = reverse_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back())
	  // There is no continuation of the current word with len chars.
	  return;
	  
	if(vect.size() == word_len+len)
	  // Found a correct size continuation.
	  break;
      }

      word_trie::const_level_iterator p(tp.node, tp.value_iters);
      while (p!=reverse_trie.end()) {
	const word_trie::level_iterator::sequence_type& vect = (*p).first;
	word_trie::const_level_iterator p2 = p;

	p2 = reverse_trie.parent_of_iterator(p2);
	for(int l=word_len+1 ; l<vect.size() ; l++)
	  p2 = reverse_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back())
	  // We have reached the end of the trie or the iterator does not correspond
	  // to a continuation of the word
	  break;

	if(vect.size() == word_len+len && (*p).second==true) {
	  // Here we have identified a candidate
	  static string s;
	  s.resize(len);
	  // 		for(int i=word_len+len-1 ; i>=word_len ; --i)
	  // 		  s.push_back(const_cast<char&>(vect[i]));
	  copy(vect.begin()+word_len, vect.begin()+word_len+len, s.begin());
	  reverse(s.begin(), s.end());

	  instances.push_back(dict[s+"++"]);
	}
	++p;
      }
    }
    else {
      // Since we are trying to identify all suffixes that added to the word result in 
      // a valid word, we will perform the same operations as before, only on the direct tree.
      copy(word.begin(), word.end(), v.begin());
      const word_trie& direct_trie = dict.get_direct_trie();

      word_trie::const_iterator tp=direct_trie.find(v), tp1=tp;

      while(1) {
	if(++tp == direct_trie.end())
	  return;
	word_trie::const_iterator p2 = tp;
	const word_trie::level_iterator::sequence_type& vect = (*tp).first;
	p2 = direct_trie.parent_of_iterator(p2);
	for(int l=word_len+1 ; l<vect.size() ; l++)
	  p2 = direct_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back())
	  // There is no continuation of the current word with len chars.
	  return;
	  
	if(vect.size() == word_len+len)
	  // Found a candidate of the correct size.
	  break;
      }

      word_trie::const_level_iterator p(tp.node, tp.value_iters);

      while (p!=direct_trie.end()) {
	const word_trie::level_iterator::sequence_type& vect = (*p).first;
	word_trie::const_level_iterator p2 = p;

	p2 = direct_trie.parent_of_iterator(p2);
	for(int l=word_len+1 ; l<vect.size() ; l++)
	  p2 = direct_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back())
	  // We have reached the end of the trie or the iterator does not correspond
	  // to a continuation of the word
	  break;

	if(vect.size() == word_len+len && (*p).second==true) {
	  static string s;
	  s.resize(len);
	  copy(vect.begin()+word_len, vect.begin()+word_len+len, s.begin());
	  instances.push_back(dict["++"+s]);
	}
	++p;
      }
    }
  }
}

inline void PrefixSuffixAddPredicate::identify_strings(wordType word_id, wordType_set& words) const {
  Dictionary& dict = Dictionary::GetDictionary();
  const string& word = dict[word_id];
  string::size_type word_len = word.size();
  static vector<char> v;
  v.resize(word_len);
  static string plusplus = "++";

  // The following value is true by default, unless specifically disabled
  static bool cache_word_lists = Params::GetParams().valueForParameter("CACHE_INTERNAL_WORD_LIST", true);
  
  if(cache_word_lists && word_id < seen[len-1].size() && seen[len-1][word_id]) {
    wordType_svector& vect = feature_lookup[len-1][word_id];
    for(wordType_svector::iterator i=vect.begin() ; i!=vect.end() ; ++i)
      if(is_prefix) {			// If it's prefix, it should look like <chars>++
	if(dict[*i][0] != '+')
	  words.insert(*i);
      } else {					// If suffix, it should look like ++<chars>
	if(dict[*i][0] == '+')
	  words.insert(*i);
      }
  } else {
    static wordType_set wrds;
    bool not_found = false;
    if(cache_word_lists)
      wrds.clear();
    if(is_prefix) { 
      // Here we are trying to identify all prefixes that added to the word will result in
      // a valid word, present in the dictionary. For this, we will be using a trie containing
      // the words in the dictionary, input in reverse order of their characters.
      const word_trie& reverse_trie = dict.get_reverse_trie();
	  
      for(int i=0 ; i<word_len ; i++)
	v[i] = word[word_len-i-1];
	
      word_trie::const_iterator tp=reverse_trie.find(v), tp1=tp;

      while(1) {
	++tp;
	if(tp == reverse_trie.end()) {
	  not_found = true;
	  break;
	}

	word_trie::const_iterator p2 = tp;

	const word_trie::level_iterator::sequence_type& vect = (*tp).first;
	p2 = reverse_trie.parent_of_iterator(p2);
	for(int l=word_len+1; l<vect.size() ; l++)
	  p2 = reverse_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back()) {
	  not_found = true;
	  // There is no continuation of the current word with len chars.
	  break;
	}

	if(vect.size() == word_len+len)
	  // Found a continuation of the correct size.
	  break;
      }
		
      if(! not_found) {
	word_trie::const_level_iterator p(tp.node, tp.value_iters);
	while (p!=reverse_trie.end()) {
	  const word_trie::level_iterator::sequence_type& vect = (*p).first;
	  word_trie::const_level_iterator p2 = p;

	  p2 = reverse_trie.parent_of_iterator(p2);
	  for(int l=word_len+1 ; l<vect.size() ; l++)
	    p2 = reverse_trie.parent_of_iterator(p2);

	  if(p2.value_iters.back() != tp1.value_iters.back())
	    // We have reached the end of the trie or the iterator does not correspond
	    // to a continuation of the word
	    break;

	  if(vect.size() == word_len+len && (*p).second==true) {
	    // Here we have identified a candidate
	    static string s;
	    s.resize(len);

	    copy(vect.begin()+word_len, vect.begin()+word_len+len, s.begin());
	    reverse(s.begin(), s.end());
	    s.append(plusplus);
	    if(cache_word_lists) {
	      wordType index = dict.insert(s);
	      wrds.insert(index);
	      words.insert(index);
	    }
	    else 
	      words.insert(dict.insert(s));
	  }
	  ++p;
	}
      }
    }
    else {
      // Since we are trying to identify all suffixes that added to the word result in 
      // a valid word, we will perform the same operations as before, only on the direct tree.
      copy(word.begin(), word.end(), v.begin());
      const word_trie& direct_trie = dict.get_direct_trie();

      word_trie::const_iterator tp = direct_trie.find(v), tp1=tp;
      while(1) {
	++tp;
	if(tp == direct_trie.end()) {
	  not_found = true;
	  break;
	}
	word_trie::const_iterator p2 = tp;
	const word_trie::level_iterator::sequence_type& vect = (*tp).first;
	p2 = direct_trie.parent_of_iterator(p2);
	for(int l=word_len+1 ; l<vect.size() ; l++)
	  p2 = direct_trie.parent_of_iterator(p2);

	if(p2.value_iters.back() != tp1.value_iters.back()) {
	  // There is no continuation of the current word with len chars.
	  not_found = true;
	  break;
	}

	if(vect.size() == word_len+len)
	  // Found a continuation of the correct size.
	  break;
      }

      if(! not_found) {
	word_trie::const_level_iterator p(tp.node, tp.value_iters);

	while (p!=direct_trie.end()) {
	  const word_trie::level_iterator::sequence_type& vect = (*p).first;
	  word_trie::const_level_iterator p2 = p;

	  p2 = direct_trie.parent_of_iterator(p2);
	  for(int l=word_len+1 ; l<vect.size() ; l++)
	    p2 = direct_trie.parent_of_iterator(p2);

	  if(p2.value_iters.back() != tp1.value_iters.back())
	    // We have reached the end of the trie or the iterator does not correspond
	    // to a continuation of the word
	    break;

	  if(vect.size() == word_len+len && (*p).second==true) {
	    static string s;
	    s.resize(len);
	    s = plusplus;
	    s.append(vect.begin()+word_len, vect.begin()+word_len+len);
	    if(cache_word_lists) {
	      wordType index = dict.insert(s);
	      wrds.insert(index);
	      words.insert(index);
	    }
	    else 
	      words.insert(dict.insert(s));
	  }
	  ++p;
	}
      }
    }
    if(cache_word_lists && word_id < feature_lookup[len-1].size()) {
      seen[len-1][word_id] = true;
      wordType_svector& vect = feature_lookup[len-1][word_id];
      vect.resize(wrds.size());
      wordType_svector::iterator j=vect.begin();
      for(wordType_set::iterator i=wrds.begin() ; i!=wrds.end() ; ++i, ++j) {
	words.insert(*i);
	ON_DEBUG(assert(dict[*i].size()==len+2));
	*j = *i;
      }
    }
  }
}

#endif
