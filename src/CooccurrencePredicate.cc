/*
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

#include "CooccurrencePredicate.h"
#include "Params.h"
#include "line_splitter.h"
#include "Predicate.h"
using namespace std;

extern wordType3D corpus;
bool CooccurrencePredicate::IsInitialized = false;
indexed_map<CooccurrencePredicate::index_pair, short int> CooccurrencePredicate::word_map_index;
set<CooccurrencePredicate::index_pair> CooccurrencePredicate::values;
vector<CooccurrencePredicate::word_map> CooccurrencePredicate::tries;

void CooccurrencePredicate::Initialize(const string& file) {
  string bigram_file = "";
  if(file != "") {				// Read from the rule file
    istream* istr;
    smart_open(istr, file);
    string line;
    line_splitter ls;
    while (getline(*istr, line) && line[0] == '#') {
      ls.split(line);
      if(ls[0] == "#bigram_list_file:") {
	bigram_file = ls[1];
	break;
      }
    }
    delete istr;
  }

  if(bigram_file == "")
    bigram_file = Params::GetParams()["COOCCURRENCE_CONFIGURATION_FILE"];

  wordType key;
  Dictionary& dict = Dictionary::GetDictionary() ;

  if(bigram_file != "") {
    istream* istr, *is;
    smart_open(istr, bigram_file);
    string line;
    line_splitter ls;
    line_splitter us("_");
    relativePosType pos;
    featureIndexType fid;
    int location = 1;			// The position feature_0 is stored on - can be 0 or 1
    while (getline(*istr, line)) {
      ls.split(line);
      ON_DEBUG(assert(ls.size() == 3));
      us.split(ls[0]);
      fid = PredicateTemplate::name_map[us[0]];
      if(us.size() != 1)
	pos = atoi1(us[1]);
      else
	pos = 0;

      if(pos==0)
	location = 0;
      else
	location = 1;

      if(pos == 0) {
	us.split(ls[1]);
	ON_DEBUG(assert(us.size()==2));
	pos = atoi1(us[1]);
	ON_DEBUG(assert(PredicateTemplate::name_map[us[0]] == fid));
      }

      smart_open(is, ls[2]);
      ON_DEBUG(assert(pos != 0));
      string line;
      word_map_index.insert(make_pair(fid, pos));
      tries.push_back(CooccurrencePredicate::word_map());
      word_map& ngram = tries.back();

      while (getline(*is, line)) {
	ls.split(line);
	key = dict.insert(ls[location]);
	ngram[key].push_back(dict.insert(ls[1-location]));
      }
      delete is;

      for(word_map::iterator ti = ngram.begin() ; ti!=ngram.end() ; ++ti) {
	wordTypeVector& v = (*ti).second;
	sort(v.begin(), v.end());
      }
    }
    delete istr;
  } else {
    for(set<index_pair>::iterator p = values.begin() ; p!=values.end() ; ++p) {
      word_map_index.insert(*p);
      ON_DEBUG(assert(word_map_index[*p] == tries.size()));
      tries.push_back(CooccurrencePredicate::word_map ());
      word_map& ngram = tries.back();
      featureIndexType feature_id = p->first;
      relativePosType pos = p->second;
      for(int i=0 ; i<corpus.size(); ++i) {
	wordType2D& vect = corpus[i];
	int min_ind = max(-PredicateTemplate::MaxBackwardLookup, -PredicateTemplate::MaxBackwardLookup - pos);
	int max_ind = min(vect.size()-PredicateTemplate::MaxForwardLookup, vect.size()-PredicateTemplate::MaxForwardLookup - pos);
		
	for(int j=min_ind ; j<max_ind ; ++j) {
	  key = vect[j][feature_id];
	  ngram[key].push_back(vect[j+pos][feature_id]);
	}
      }
      for(word_map::iterator ti = ngram.begin() ; ti!=ngram.end() ; ++ti) {
	wordTypeVector& v = (*ti).second;
	sort(v.begin(), v.end());
      }
    }
  }

  IsInitialized = true;
}

void CooccurrencePredicate::instantiate(const wordType2D& corpus, int sample_ind, wordTypeVector& instances) const {
  wordType key = corpus[sample_ind][feature_id];
  
  word_map& ngram = tries[word_map_index[make_pair(feature_id, pos)]];

  word_map::iterator tp = ngram.find(key);

  if(tp != ngram.end()) {
    word_map::iterator::value_type p = (*tp);
    instances.insert(instances.end(), p.second.begin(), p.second.end());
  }
}

void CooccurrencePredicate::identify_strings(const wordType1D& features, wordType_set& words) const {
  // This predicate is not indexable, so there is no need to do anything here, except initialize the 
  // bigram mappings.

  if(!IsInitialized)
    Initialize();
}

void CooccurrencePredicate::identify_strings(wordType word_id, wordType_set& words) const {
  if(!IsInitialized)
    Initialize();
}

string CooccurrencePredicate::printMe(wordType element) const {
  static string temp;
  const Dictionary& dict = Dictionary::GetDictionary();

  temp="";
  temp.append(PredicateTemplate::name_map[feature_id]);
  temp.append("^^");
  temp.append(itoa(pos));
  temp.append("=");
  temp.append(dict[element]);
  return temp;
}
