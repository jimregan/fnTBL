/*
  The training program for TBL.

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

#include <iostream.h>
#include <fstream.h>
#include <time.h>
#if __GNUC__ < 3
#include <hash_set>
#else
#include <ext/hash_set>
#endif /* __GNUC__ */
#include <stdlib.h>
#include <sys/timeb.h>

#include "typedef.h"
#include "TBLTree.h"
#include "Dictionary.h"
#include "Rule.h"
#include "line_splitter.h"
#include "common.h"
#include "index.h"
#include "Params.h"
#include "trie.h"
#include "Node.h"
#include "io.h"
#include "timer.h"
#include "PrefixSuffixAddPredicate.h"
#include "ContainsStringPredicate.h"
#include "Target.h"
#include <unistd.h>
#include "sample_signature.h"

typedef PredicateTemplate::PredicateTemplate_vector PredicateTemplate_vector;
typedef hash_set<Rule> rule_hash_set;
typedef hash_set<Rule*> rulep_hash_set;
typedef vector<pair<int, int> > position_vector;
typedef vector<Rule> rule_vector;
typedef vector<Rule*> rulep_vector;
typedef word_index<unsigned int, unsigned short> word_index_class;
typedef hash_map<Predicate, int> pred_int_map;

wordType3D corpus;
wordType3DVector ruleTrace;
featureIndexType2D ruleTemplates;
vector<scoreType> costs;
rule_hash allRules;
rule_hash_set newRules;
rule_vector chosen_rules;
pred_int_map pred_index;
ostream* outfile;
string pred_file;
bool test_mode = false;
string output_type = "snow";

word_index_class corpusIndex;
word_index_class classifIndex(1);
word_index_class defaultIndex(2);

bool v_flag = false;
int V_flag = 0;

// When this parameter is turned on, indexing techniques are used to speed up the
// evaluation of the rules.
bool i_flag = false;
int I_flag = 0;
bool _probabilities = false;
bool force_compute = false;
bool all_positive_rules_percent = false;
int erase_bad_rules = -1;
int corpus_size;
int best_rule_index = 0;

position_vector best_rule_applic_places;
typedef vector<featureIndexType> feature_vector;
wordType UNK;

// The threshold under which new rules will not be learned.
scoreType THRESHOLDSCORE = (scoreType)2.5;
int num_classes = 0;
int all_rules_thresh = -10000;
int min_positive_score = 2;

inline bool sample_is_completely_correct(const wordType1D& corpus) {
  for(int i=0 ; i<TargetTemplate::TRUTH_SIZE ; i++)
    if(! TargetTemplate::value_is_correct(corpus[TargetTemplate::STATE_START+i], corpus[TargetTemplate::TRUTH_START]))
      return false;
  return true;
}

inline bool sample_is_completely_incorrect(const wordType1D& corpus) {
  for(int i=0 ; i<TargetTemplate::TRUTH_SIZE ; i++)
    if(TargetTemplate::value_is_correct(corpus[TargetTemplate::STATE_START+i], corpus[TargetTemplate::TRUTH_START]))
      return false;

  return true;
}

inline bool is_state(featureIndexType pos) {
  return TargetTemplate::STATE_START <= pos && pos < TargetTemplate::STATE_START+TargetTemplate::TRUTH_SIZE;
}

struct comp {
  bool operator() (const Rule& a, const Rule& b) {
    return (a.good-a.bad) > (b.good-b.bad);
  }
};

struct wordType1DHash {
  size_t operator() (const wordType1D& p) const {
    static int no_features = PredicateTemplate::name_map.size()+TargetTemplate::name_map.size();
    size_t ind = *p;
    wordType* l=p;
    for(int i=1 ; i<no_features ; i++, ++l)
      ind = 7*ind + *l;

    return ind;
  }
};

struct wordType1D_equal {
  bool operator() (const wordType1D& p1, const wordType1D& p2) {
    static int no_features = PredicateTemplate::name_map.size()+TargetTemplate::name_map.size();
    return equal(p1, p1+no_features, p2);
  }
};

// this is called if a costs file is not specified.
// simply initializes everybody to the default cost of 1.
void initializeDefaultCosts ( int lines ) {
  bool collapse_samples = 
    ! Params::GetParams().valueForParameter("EMPTY_LINES_ARE_SEPARATORS",true) &&
    ! Params::GetParams().valueForParameter("DONT_COLLAPSE_SAMPLES", false);
  if (collapse_samples) {			// We'll collapse the identical samples
    if(v_flag)
      cerr << "Collapsing samples: ";
    hash_map<wordType1D, int, wordType1DHash, wordType1D_equal> samples;
    for(int i=0 ; i<corpus.size() ; i++)
      samples[corpus[i][0]]++;

    static int no_features = PredicateTemplate::name_map.size()+TargetTemplate::name_map.size();
    wordType3D corpus1(samples.size());
    costs.resize(samples.size());

    int k=0;
    for(hash_map<wordType1D, int, wordType1DHash, wordType1D_equal>::iterator i=samples.begin() ; i!=samples.end() ; ++i, ++k) {
      corpus1[k].resize(1);
      corpus1[k][0] = new wordType [no_features];
      copy(i->first, i->first+no_features, corpus1[k][0]);
      costs[k] = i->second;
    }
    corpus.swap(corpus1);
    if(v_flag)
      cerr << "there are " << corpus.size() << " samples in the end" << endl;
    for(int i=0 ; i<corpus1.size() ; i++)
      delete [] corpus1[i][0];
  }
  else {
    costs.resize(lines);
    for (int i = 0; i < lines; i++) {
      costs[i] = DEFAULTCOST;
    }
  }
}

// Called to insert a "good" rule into the rule hash. Also updates the good/bad counts for the 
// rule with the actual values
void insertRulesIntoHash(int line, int word, rule_hash_set &ruleSet, 
			 bool check_first = false, sample_signature* signature = NULL)
{
  static const Dictionary& dict = Dictionary::GetDictionary();

  using std::set;
  typedef set<int> int_set;
  int_set v;

  for (rule_hash_set::iterator rule = ruleSet.begin(); 
       rule != ruleSet.end(); ++rule) {

    pred_int_map::iterator it = pred_index.find(rule->predicate);
    if(it == pred_index.end()) {
      // If in test mode, go to the next rule
      if(test_mode)
	continue;

      int sz = pred_index.size()+dict.num_classes;
      it = pred_index.insert(make_pair(rule->predicate, sz)).first;
      *outfile << it->second << " " << rule->predicate.printMe() << "\n";
    }

    v.insert(it->second);
  }


  static const string truth_sep = Params::GetParams().valueForParameter("TRUTH_SEPARATOR");

  if (output_type == "memt") {
    if (!test_mode && truth_sep != "") {
      // If multiple truths are possible, split the sample into as many samples as there are
      // truths, and output a sample for each truth
      static line_splitter ts(truth_sep);

      ts.split(dict[corpus[line][word][TargetTemplate::TRUTH_START]]);
      for(line_splitter::iterator j=ts.begin() ; j!=ts.end() ; ++j) {
	// Output the new class first
	cout << *j;
	for (int_set::iterator i=v.begin() ; i!=v.end() ; i++)
	  cout << ", " << *i;
	cout << ":\n";
      }
    } else {
      cout << dict[corpus[line][word][TargetTemplate::TRUTH_START]];
    
      for (int_set::iterator i=v.begin() ; i!=v.end() ; i++)
	cout << ", " << *i;
      cout << "\n";
    }
  }
  else if (output_type == "snow") {

    if (!test_mode && truth_sep != "") {
      // If multiple truths are possible, split the sample into as many samples as there are
      // truths, and output a sample for each truth
      static line_splitter ts(truth_sep);

      ts.split(dict[corpus[line][word][TargetTemplate::TRUTH_START]]);
      for(line_splitter::iterator j=ts.begin() ; j!=ts.end() ; ++j) {
	// Output the new class first
	cout << dict[*j];
	for (int_set::iterator i=v.begin() ; i!=v.end() ; i++)
	  cout << ", " << *i;
	cout << ":\n";
      }
    } else {
      cout << corpus[line][word][TargetTemplate::TRUTH_START];
    
      for (int_set::iterator i=v.begin() ; i!=v.end() ; i++)
	cout << ", " << *i;
      cout << ":\n";
    }
  }
}

typedef vector<string> string_vector;

void read_count_file(const string& file) {
  istream* f;
  smart_open(f, file);
//   ifstream str(file.c_str());
  string line;
  line_splitter ls;
  string_vector words;
  ticker tk("Reading predicates: ", 2048);

  while (getline(*f, line)) {
    ls.split(line);
    words.resize(ls.size()-1);
    const string_vector& l = ls.data();
    copy(l.begin()+1, l.end(), words.begin());

    Predicate p(words);
    pred_index[p] = atoi(ls[0].c_str());
    tk.tick();
  }
  delete f;
}

// this function computes the good and bad for this particular rule
void computeScoreForRule(Rule& rule) {
  if(force_compute)
    for (int i = 0; i < (int)corpus.size(); i++) {  
      int numWords = (int)corpus[i].size() - PredicateTemplate::MaxForwardLookup;
      for (int j = -PredicateTemplate::MaxBackwardLookup; j < numWords; j++) {
	if(rule.test(corpus[i], j))
	  rule.update_counts(corpus[i][j], costs[i]);
      }
    }
  else {
    int i=rule.get_least_frequent_feature_position();
    bool unindexable_rule = false;

    if(i==-1) {
      unindexable_rule = true;
      i = 0;
    }

    wordType least_frequent = rule.predicate.tokens[i];

    static AtomicPredicate::storage_vector features;
    features.clear();
    PredicateTemplate::Templates[rule.predicate.template_id][i].get_feature_ids(features);

    word_index_class& 
      thisIndex = unindexable_rule ? 
      defaultIndex 
      :
      (
       is_state(features[0]) ?
       classifIndex 
       : 
       corpusIndex
       );

    static AtomicPredicate::position_vector offsets;
    offsets.clear();

    if(unindexable_rule)
      offsets.push_back(0);
    else
      PredicateTemplate::Templates[rule.predicate.template_id][i].get_sample_differences(offsets);

    word_index_class::iterator endp = thisIndex.end(least_frequent);

    int last_i=-1;
    char seen[20000];

    using namespace std;
    for(word_index_class::iterator it = thisIndex.begin(least_frequent); !(it == endp) ; ++it) {
      int i = (*it).line_id();
      if(i!=last_i) {
	fill(seen, seen+corpus[i].size(), 0);
      }
      last_i = i;
      for(AtomicPredicate::position_vector::iterator offset = offsets.begin() ; offset != offsets.end() ; ++offset) {
	int j = (*it).word_id() - *offset;
	if(V_flag >= 5)
	  cout << "@@Index: (" << i << "," << j << ")" << endl;

	if (j>=-PredicateTemplate::MaxBackwardLookup && j<corpus[i].size()-PredicateTemplate::MaxForwardLookup) {
	  if(seen[j])
	    continue;
	  seen[j] = true;
	  if(rule.test(corpus[i], j))
	    rule.update_counts(corpus[i][j], costs[i]);
	}
      }
    }
  }
}

// this function creates a set of rules for a particular word and adds them to the current pool of
// rules.
void createRulesForExample(int line, int word, rule_hash_set &ruleSet, bool returnAllRules = false, bool forced_generation = false)
{
  static rule_hash_set gen_rules;

  for(int t=0; t<PredicateTemplate::Templates.size() ; ++t) {
    RuleTemplate::instantiate(corpus[line], word, t, ruleSet, returnAllRules, forced_generation);
  }
}

void createRulesForExample(int line, int word, const int1D& modified_positions, rulep_hash_set& pRules,
			   feature_vector& modified_states, bool returnAllRules = false, bool add_new_rules = false) {
  // If the special function is turned on, then return all the rules
  // whose predicate is true on the specific example.
  ON_DEBUG(assert(allRules.is_on()));

  static int1D relevant;
  relevant.clear();

  for(int1D::const_iterator i=modified_positions.begin() ; i!=modified_positions.end() ; ++i)
    if (word+PredicateTemplate::MaxBackwardLookup <= *i &&
	*i <= word+PredicateTemplate::MaxForwardLookup)
      relevant.push_back(*i);
  // If the template does not contain the string associated with the
  // difference in positions between the word and real_word - real_word
  // being the position that is changed by the bestRule and word the
  // position in question - the rule either both fires before and after, or
  // doesn't fire neither before nor after, therefore that particular rule
  // is not influenced by the current change, and it should not be considered 
  // for update.
  int t=0;

  bool position_is_modified = (find(relevant.begin(), relevant.end(), word) != relevant.end());
  static rule_hash_set rlss;
  static wordType2DVector tk;

  if(returnAllRules) {
    for (PredicateTemplate_vector::const_iterator thisTemplate=PredicateTemplate::Templates.begin();
	 thisTemplate != PredicateTemplate::Templates.end(); ++thisTemplate, ++t) {
      if(! position_is_modified) {
	int1D::iterator i;
	for(i=relevant.begin() ; i!=relevant.end() ; ++i)
	  if(thisTemplate->depends_on(*i - word, modified_states))
	    break;

	if(i==relevant.end())
	  continue;
      }

      tk.clear();
      PredicateTemplate::Templates[t].instantiate(corpus[line], word, tk);

      for(wordType2DVector::iterator ti=tk.begin() ; ti!=tk.end() ; ++ti) {
	Predicate p(t, *ti);
	int no_rls = 0;
	rule_hash_const_iterator rlend = allRules.pend(p);
	for(rule_hash_const_iterator rl = allRules.pbegin(p) ; rl!=rlend ; ++rl, ++no_rls)
	  if((*rl).constraint_test(corpus[line][word]))
	    pRules.insert(&const_cast<Rule&>(*rl));

      }

      if(add_new_rules && !sample_is_completely_correct(corpus[line][word])) {
	rlss.clear();
	RuleTemplate::instantiate(corpus[line], word, t, rlss);

	for(rule_hash_set::iterator rl=rlss.begin() ; rl!=rlss.end() ; ++rl) {
	  Rule& rule = const_cast<Rule&>(*rl);

	  if(! rule.constraint_test(corpus[line][word]))
	    continue;

	  rule_hash::iterator it = allRules.find(rule);

	  if(it == allRules.end()) {
	    if(! add_new_rules || sample_is_completely_correct(corpus[line][word]))
	      continue;
			
	    pair<rule_hash_set::iterator, bool> p = newRules.insert(rule);
	    if(! p.second)
	      continue;
	    it = p.first;
	    computeScoreForRule(const_cast<Rule&>(*it));
	    if(V_flag >= 2)
	      cerr << "Added new rule: " << it->printMe() << " good: " << it->good << " bad: " << it->bad << endl;
	  }
	}
      }
    } 
  } else {
    for (PredicateTemplate_vector::const_iterator thisTemplate=PredicateTemplate::Templates.begin();
	 thisTemplate != PredicateTemplate::Templates.end(); ++thisTemplate, ++t) {
      if(! position_is_modified) { // This is not a position that gets modified
	int1D::iterator i;
	for(i=relevant.begin() ; i!=relevant.end() ; ++i)
	  if(thisTemplate->depends_on(*i - word, modified_states))
	    break;

	if(i==relevant.end())
	  continue;
      }

      rlss.clear();
      RuleTemplate::instantiate(corpus[line], word, t, rlss);

      for(rule_hash_set::iterator rl=rlss.begin() ; rl!=rlss.end() ; ++rl) {
	Rule& rule = const_cast<Rule&>(*rl);
	if(! rule.constraint_test(corpus[line][word]))
	  continue;
	rule_hash::iterator it = allRules.find(rule);
	if(it == allRules.end()) {
	  pair<rule_hash_set::iterator, bool> p = newRules.insert(rule);
	  if(! p.second)
	    continue;
	  it = p.first;
	  computeScoreForRule(const_cast<Rule&>(*it));
	  if(V_flag >= 2)
	    cerr << "Added new rule: " << it->printMe() << " good: " << it->good << " bad: " << it->bad << endl;
	  // Since the rule has the score computed correctly for this sentence
	  // there is no need to add it to the list of rules that need updating
	} else
	  pRules.insert(&const_cast<Rule&>(*it));
      }
    }
  }
}
	
// This creates a new set of rules
// all templates are valid since this time we padded it correctly.
void createNewRuleSet ( void )
{
  // If the generation is done only on the changed samples,
  // skip this step.
  if(allRules.is_on())
    return;

  ticker tk("Processed lines:", 512);
  allRules.clear();
  static rule_hash_set ruleSet;
  for (int i = 0; i < (int)corpus.size(); i++) {  
    int numWords = (int)corpus[i].size()-PredicateTemplate::MaxForwardLookup;
    for (int j = -PredicateTemplate::MaxBackwardLookup; j < numWords; j++) {
      if(sample_is_completely_correct(corpus[i][j])) continue;

      ruleSet.clear();
      createRulesForExample(i, j, ruleSet);
      insertRulesIntoHash(i, j, ruleSet);
    }
    tk.tick();
  }
  tk.clear();
}

// Eliminates all the rules with good count less than eliminationThreshold.
// It's done so that "useless" rules do not hinder the performance of the algorithm.
void eliminateRules() {
  static const Params& p = Params::GetParams();
  static int eliminationThreshold = static_cast<int>(atoi1(p["ELIMINATION_THRESHOLD"]));

  if(v_flag)
    cerr << "Gathering rules to be eliminated:" << endl;
  rule_hash_set rules_to_eliminate;
  for(rule_hash::iterator i=allRules.begin() ; i!=allRules.end() ; ++i) {
    if(i->good <= eliminationThreshold)
      rules_to_eliminate.insert(*i);
  }
  if(v_flag)
    cerr << "Will eliminate " << rules_to_eliminate.size() << " rules, out of " << allRules.size() << "." << endl;
  for(rule_hash_set::iterator i=rules_to_eliminate.begin() ; i!=rules_to_eliminate.end() ; ++i)
    allRules.erase(allRules.find(*i));

  if(v_flag)
    cerr << "There are " << allRules.size() << " remaining rules." << endl;
}

// This is called once, when the switch in the computation has
// been decided (by default, at the beginning).
void computeScoreForAllRules() {
  allRules.clear();
  
  rule_hash_set ruleSet;
  rulep_hash_set pRules;
  
  ticker tk("Sentences processed: ", 64);
  if(v_flag)
    cerr << "Computing good counts" << endl;
  int rule_threshold = I_flag;

  if(pred_file != "")
    read_count_file(pred_file);

//   if (output_type == "memt") {
//     indexed_map<sample_signature> signatures;
//     unsigned** sample_signatures = new unsigned* [corpus.size()];

//     // In a first pass, we need to identify the predicates applying on each sample
//     for (int i = 0; i < (int)corpus.size(); i++) {
//       int numWords = (int)corpus[i].size()-PredicateTemplate::MaxForwardLookup;
//       sample_signatures[i] = new unsigned [numWords];

//       for (int j = -PredicateTemplate::MaxBackwardLookup; j < numWords; j++) {
	
// 	if(V_flag>=2 && I_flag==0)
// 	  cerr << "[] Adding counts for pair (" << i << "," << j << ")" << endl;
	
// 	ruleSet.clear();
// 	createRulesForExample(i, j, ruleSet, false, true);
// 	sample_signature s;

// 	if (test_mode)
// 	  insertRulesIntoHash(i, j, ruleSet, true, &s);
// 	else
// 	  insertRulesIntoHash(i, j, ruleSet, false, &s);

// // 	signatures[j-PredicateTemplate::MaxBackwardLookup] = signatures.insert(s);
//       }
//     }
//   } 
//   else if (output_type == "snow") {

  // At the first pass compute only the good counts
  for (int i = 0; i < (int)corpus.size(); i++) {  
    int numWords = (int)corpus[i].size()-PredicateTemplate::MaxForwardLookup;
    for (int j = -PredicateTemplate::MaxBackwardLookup; j < numWords; j++) {
      
      if(V_flag>=2 && I_flag==0)
	cerr << "[] Adding counts for pair (" << i << "," << j << ")" << endl;
      
      ruleSet.clear();
      createRulesForExample(i, j, ruleSet, false, true);
      if (test_mode)
	insertRulesIntoHash(i, j, ruleSet, true);
      else
	insertRulesIntoHash(i, j, ruleSet);
    }
    
    // If we want to eliminate rules and the threshold has been reached
    if(I_flag > 0 && allRules.size() > rule_threshold) {
      tk.clear();
      eliminateRules();
      rule_threshold += I_flag;
    }
    tk.tick();
  }

// }
  delete outfile;

  tk.clear();
}

struct rule_count_sorter {
  const hash_map<Rule, int>& cnts;
  rule_count_sorter(const hash_map<Rule, int>& c): cnts(c) {}
  int operator() (const Rule& r1, const Rule& r2) {
    hash_map<Rule, int>::const_iterator f1 = cnts.find(r1), f2=cnts.find(r2);
    return f1->second < f2->second;
  }
};

void AddSemiGoodRules(ostream* rules) {
  rule_hash_set ruleSet;
  hash_map<Rule, int> ruleSemiGoods;

  ticker tk("Sentences processed: ", 1024);

  // At the first pass compute only the good counts
  for (int i = 0; i < (int)corpus.size(); i++) {  
    int numWords = (int)corpus[i].size()-PredicateTemplate::MaxForwardLookup;
    for (int j = -PredicateTemplate::MaxBackwardLookup; j < numWords; j++) {
      // Generate goods only for the samples that are incorrect
      if(V_flag>=2 && I_flag==0)
	cerr << "[] Adding counts for pair (" << i << "," << j << ")" << endl;

      ruleSet.clear();
      createRulesForExample(i, j, ruleSet, false, true);
      for(rule_hash_set::iterator rl=ruleSet.begin() ; rl!=ruleSet.end() ; ++rl) {
	rule_hash::iterator thisRule = allRules.find(*rl);
	if(thisRule != allRules.end() && thisRule->bad == 0)
	  ruleSemiGoods[*thisRule]++;
      }
    }

    tk.tick();
  }
  tk.clear();
  rule_vector rls(ruleSemiGoods.size());
  rls.clear();
  for(hash_map<Rule, int>::iterator it = ruleSemiGoods.begin() ; it!=ruleSemiGoods.end() ; ++it)
    if(it->second>=all_rules_thresh) {
      it->first.good = it->second;
      rls.push_back(it->first);
    }

  sort(rls.begin(), rls.end(), less<Rule>());
  
  for(rule_vector::iterator i=rls.begin() ; i!=rls.end() ; ++i) {
    i->good = 0;
    if(v_flag)
      cerr << "GOOD:" << i->good << " BAD:"
	   << i->bad << " SCORE:" << ruleSemiGoods[*i]
	   << " RULE: " << i->printMe() << endl;
    *rules << "GOOD:" << i->good << " BAD:"
	   << i->bad << " SCORE:" << ruleSemiGoods[*i]
	   << " RULE: " << i->printMe() << endl;
  }
}

// This function gets the bad and good counts of the rules
const Rule &getBestRule(void )
{
  best_rule_applic_places.clear();
  static position_vector temp_pos;
  temp_pos.clear();

  static int iter = -1;
  iter++;

  if(v_flag)
    cerr << "There are " << allRules.size() << " rules at iteration " << iter << "." << endl;
  rule_hash_set::iterator bestRule = allRules.begin();
  scoreType bestScore = (scoreType)-1000.0;

  int iteration=0;
  bool start = true;
  for (rule_hash_set::iterator thisRule = allRules.begin();
       thisRule != allRules.end(); ++thisRule, ++iteration) {
    Rule& rule = const_cast<Rule&>(*thisRule);

    if(allRules.is_on()) {
      if(*bestRule < rule) {
	bestScore = rule.good-rule.bad;
	bestRule = thisRule;
      }
    } else {
      scoreType currentScore = rule.good;
      temp_pos.clear();
      // If the current rule is not good enough
      if(!start && !rule.better(*bestRule, currentScore))
	continue;

      if(i_flag) {
	int i=rule.get_least_frequent_feature_position();
	bool unindexable_rule = false;
		
	if(i==-1) {
	  unindexable_rule = true;
	  i = 0;
	}
		
	wordType least_frequent = rule.predicate.tokens[i];
		
	static AtomicPredicate::storage_vector features;
	features.clear();
	PredicateTemplate::Templates[rule.predicate.template_id][i].get_feature_ids(features);
		
	word_index_class& 
	  thisIndex = unindexable_rule ? 
	  defaultIndex 
	  :
	  (
	   is_state(features[0]) ? 
	   classifIndex 
	   : 
	   corpusIndex
	   );
		
	static AtomicPredicate::position_vector offsets;
	offsets.clear();
		
	if(unindexable_rule)
	  offsets.push_back(0);
	else
	  PredicateTemplate::Templates[rule.predicate.template_id][i].get_sample_differences(offsets);

	word_index_class::iterator endp = thisIndex.end(least_frequent);
	int last_i = -1;
	static bit_vector seen;
	for(word_index_class::iterator it = thisIndex.begin(least_frequent); !(it == endp) ; ++it) {
	  int i = (*it).line_id(); 
	  wordType2D& line = corpus[i];

	  if(i!=last_i) {
	    seen.clear();
	    seen.resize(line.size());
	    fill(seen.begin(), seen.end(), false);
	  }
	  last_i = i;

	  for(AtomicPredicate::position_vector::iterator off = offsets.begin() ; off != offsets.end() ; ++off) {
	    relativePosType offset = *off;
	    wordType j = (*it).word_id() - offset;
	    if(-PredicateTemplate::MaxBackwardLookup<=j &&
	       j < line.size()-PredicateTemplate::MaxForwardLookup &&
	       rule.test(line, j)) {
	      if(seen[j])
		continue;

	      seen[j] = true;
	      temp_pos.push_back(pair<int, int>(i, j));
	      rule.update_bad_counts(line[j], costs[i]);
	    }
	  }
	  if(rule.good-rule.bad < bestScore)
	    break;
	}
      }
      else {
	// For each sentence
	for (int i = 0; i < (int)corpus.size() && currentScore >= bestScore; i++) {
	  // For each word in the sentence
	  wordType2D& line = corpus[i];
	  int last_index = line.size() - PredicateTemplate::MaxForwardLookup;
	  for (int j=-PredicateTemplate::MaxBackwardLookup ; 
	       j < last_index && rule.good-rule.bad >= bestScore; 
	       j++) {

	    // only the already-correct instances will bring down the scores.
	    if(rule.test(line, j)) {
	      temp_pos.push_back(pair<int, int>(i, j));
	      rule.update_bad_counts(line[j], costs[i]);
	    }
	  }
	}
      }
      currentScore = rule.good - rule.bad;
      if(start || rule.better(*bestRule, currentScore)) {
	if (currentScore < THRESHOLDSCORE) 
	  continue;
	bestScore = currentScore;
	bestRule = thisRule;
	best_rule_applic_places = temp_pos;
	start = false;
      }
    }
  }
  return *bestRule;
}

// Now that we've got the best rule, we have to go ahead and update the corpus.
void applyBestRule (const Rule &bestRule)
{
  static int1D placesToChange;
  static wordType2DVector prevPositions;
  bit_vector processed, changingPosition;
  static int 
    STATE_START = TargetTemplate::STATE_START,
    TRUTH_START = TargetTemplate::TRUTH_START;
  
  int bestRuleTtid = bestRule.target.tid;
  static wordTypeVector best_rule_target(TargetTemplate::TRUTH_SIZE);
  const TargetTemplate::pos_vector& best_rule_positions = TargetTemplate::Templates[bestRuleTtid].positions;
  
  fill(best_rule_target.begin(), best_rule_target.end(), 0);
  int t = 0;
  for(TargetTemplate::pos_vector::const_iterator itt=best_rule_positions.begin() ;
      itt != best_rule_positions.end();
      ++itt, ++t) 
    best_rule_target[*itt] = bestRule.target.vals[t];

  if(allRules.is_on()) {
    rulep_hash_set pRules;
    // For each position that needs to be updated
    // we need to update the goods and bads for all 
    // the rules that might apply in the context.

    int i=bestRule.get_least_frequent_feature_position();
    bool unindexable_rule = false;

    if(i==-1) {
      unindexable_rule = true;
      i = 0;
    }

    wordType least_frequent = bestRule.predicate.tokens[i];

    static AtomicPredicate::storage_vector features;
    features.clear();
    PredicateTemplate::Templates[bestRule.predicate.template_id][i].get_feature_ids(features);

    word_index_class& 
      thisIndex = unindexable_rule ? 
      defaultIndex 
      :
      (
       is_state(features[0]) ? 
       classifIndex 
       : 
       corpusIndex
       );

    static AtomicPredicate::position_vector offsets;
    offsets.clear();

    if(unindexable_rule)
      offsets.push_back(0);
    else
      PredicateTemplate::Templates[bestRule.predicate.template_id][i].get_sample_differences(offsets);

    // One more step is needed here. We need to copy the current index before we iterate on it, because
    // it will change in the case of classification indices, resulting in incorrect behavior.

    word_index_class index(thisIndex.get_type());
    index.copy_data_field(thisIndex, least_frequent);
    word_index_class::iterator endp = index.end(least_frequent);
    static wordType2DVector old_corpus;
    static feature_vector modified_states;

    modified_states.resize(bestRule.target.vals.size());
    int ppp=0;
    for(TargetTemplate::pos_vector::const_iterator itt=best_rule_positions.begin() ; 
	itt != best_rule_positions.end() ; ++itt, ++ppp) 
      modified_states[ppp] = *itt + STATE_START;

    for(word_index_class::iterator it = index.begin(least_frequent); !(it == endp) ;) {
      int i = (*it).line_id();

      newRules.clear();
      placesToChange.clear();
      changingPosition.clear();
      processed.clear();
      processed.resize(corpus[i].size());
      changingPosition.resize(corpus[i].size());

      int sz = corpus[i].size()-PredicateTemplate::MaxForwardLookup;
      while (it!=endp && i == (*it).line_id()) {
	for(AtomicPredicate::position_vector::iterator off = offsets.begin() ; off != offsets.end() ; ++off) {
	  relativePosType offset = *off;
	  int j = (*it).word_id() - offset;
	  if(j>=-PredicateTemplate::MaxBackwardLookup && j<sz && // j is inside the scope of change
	     !changingPosition[j] &&
	     bestRule.test(corpus[i], j)) {                      // and the best rule applies on sample corpus[i][j]
	    placesToChange.push_back(j);
	    changingPosition[j] = true;
	    ruleTrace[i][j].push_back(best_rule_index);
	  }
	}
	++it;
      }

      prevPositions.resize(placesToChange.size());
      for(wordType2DVector::iterator itt=prevPositions.begin() ; itt!=prevPositions.end() ; ++itt) {
	itt->resize(TargetTemplate::TRUTH_SIZE);
	copy(best_rule_target.begin(), best_rule_target.end(), itt->begin());
      }

      static int feature_set_size = RuleTemplate::name_map.size();

      old_corpus.resize(corpus[i].size());
      for(int j=0 ; j<corpus[i].size() ; j++)
	old_corpus[j].resize(feature_set_size);

      // For each positions that needs updating
      for(int1D::iterator pos = placesToChange.begin() ; pos!=placesToChange.end() ; ++pos) {
	wordType j = *pos;

	if(V_flag >= 3) {
	  cerr << "Processing sentence " << i << ", word " << static_cast<int>(j) << " :" << endl;
	  cerr << "--------------------------------------------------------" << endl;
	}

	int min_pos = max(-PredicateTemplate::MaxBackwardLookup, 
			  static_cast<int>(j+PredicateTemplate::MaxBackwardLookup)),
	  max_pos = min(corpus[i].size()-1-PredicateTemplate::MaxForwardLookup, 
			static_cast<unsigned int>(j+PredicateTemplate::MaxForwardLookup));

	for(int k=min_pos ; k<=max_pos ; ++k) {
	  if(processed[k]) // We have already processed the position
	    continue;

	  if(V_flag >= 3) {
	    cerr << "Processing inner word " << k << ":" << endl;
	    cerr << "***************************************" << endl;
	  }


	  pRules.clear();

	  // Call createRulesForExample such that it does not add new rules
	  // The last parameter is true if at least one of the states has a correct value,
	  // in which case at least one rule will need to update its bad counts. 
	  createRulesForExample(i, k, placesToChange, pRules, modified_states, !sample_is_completely_incorrect(corpus[i][k]));

	  for(int k1=0 ; k1<corpus[i].size() ; k1++)
	    copy(corpus[i][k1], corpus[i][k1]+feature_set_size, old_corpus[k1].begin());

	  // Now, pRules contain the rules that applied in the old state
	  int pp=0;
	  for(int1D::iterator p = placesToChange.begin() ; p!=placesToChange.end() ; ++p, ++pp) {
	    for(TargetTemplate::pos_vector::const_iterator tid = best_rule_positions.begin() ; 
		tid != best_rule_positions.end();
		++tid) {
	      wordType& corpus_value = corpus[i][*p][STATE_START + *tid];
	      classifIndex.erase(corpus_value, i, *p);
	      ::swap(corpus_value, prevPositions[pp][*tid]);
	      classifIndex.insert(corpus_value, i, *p);
	      ON_DEBUG(assert(classifIndex.find(corpus_value, i, *p) != classifIndex.end(corpus_value)));
	    }
	  }

	  /*
	    Change the good and/or bad counts for the rules that got modified..
	    The conditions for updating the good/bad counts are:
	    1. r_p(b(s)) == false (s is the sample, r the variable rule1 from below and b the bestRule)
	    => good(r) needs to be decreased by costs[i] * |{j | r_j(s)==truth_j(s) && s_j!=truth_j(s)}|
	    => bad(r)  needs to be decreased by costs[i] * |{j | r_j(s)!=truth_j(s) && s_j==truth_j(s)}|
	    2. r_p(b(s)) == true (the rule s still applies to the sample)
	    => good(r) needs to be decreased by costs[i] * |{j | r_j(s)==truth_j(s) && s_j!=truth_j(s) && b_j(s)==truth_j(s)}|
	    => bad(r)  needs to be decreased by costs[i] * |{j | r_j(s)!=truth_j(s) && s_j==truth_j(s) && b_j(s)!=truth_j(s)}|
	    For simplicity, the constraint is part of the predicate.
	  */		  

	  for(rulep_hash_set::iterator rl=pRules.begin() ; rl!=pRules.end() ; ++rl) {
	    Rule& rule1 = const_cast<Rule&>(**rl);
	    TargetTemplate::pos_vector& poss = TargetTemplate::Templates[rule1.target.tid].positions;
	    if(! rule1.predicate.test(corpus[i], k) || !rule1.constraint_test(corpus[i][k])) { // r_p(b(s)) == false
	      int pp = 0;
	      for(TargetTemplate::pos_vector::const_iterator jj=poss.begin() ; jj!=poss.end() ; ++jj) {
		if(TargetTemplate::value_is_correct(rule1.target.vals[pp], old_corpus[k][TRUTH_START + *jj])) {
		  if(! TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj])) {
		    rule1.good -= costs[i];

		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- goods have been decreased" << endl;
		  }
		} 
		else
		  if(TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj])) {
		    rule1.bad -= costs[i];
					
		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- bads have been decreased" << endl;
		  }
	      }
	    }
	    else { // r_p(b(s)) == true
	      int pp=0;
	      for(TargetTemplate::pos_vector::const_iterator jj=poss.begin() ; jj!=poss.end(); ++jj, ++pp) {
		if(TargetTemplate::value_is_correct(rule1.target.vals[pp], old_corpus[k][TRUTH_START + *jj])) {
		  if(! TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj]) && 
		     TargetTemplate::value_is_correct( corpus[i][k][STATE_START + *jj],  corpus[i][k][TRUTH_START + *jj])) {
		    rule1.good -= costs[i];
					
		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- goods have been decreased" << endl;
		  }
		}
		else
		  if(TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj]) &&
		     ! TargetTemplate::value_is_correct(corpus[i][k][STATE_START + *jj], corpus[i][k][TRUTH_START + *jj])) {
		    rule1.bad -= costs[i];

		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- bads have been decreased" << endl;
		  }
	      }
	    }
	  }

	  // Create the rules that apply on the new state of the current sample
	  pRules.clear();

	  // Call createRulesForExample such that it adds new rules
	  createRulesForExample(i, k, placesToChange, pRules, modified_states, !sample_is_completely_incorrect(corpus[i][k]), true);

	  for(int k1=0 ; k1<corpus[i].size() ; k1++)
	    copy(corpus[i][k1], corpus[i][k1]+feature_set_size, old_corpus[k1].begin());
		  
	  // pRules contains now all the rules that apply in the new state
	  pp = 0;
	  for(int1D::iterator p = placesToChange.begin() ; p!=placesToChange.end() ; ++p, ++pp) {
	    for(TargetTemplate::pos_vector::const_iterator tid = best_rule_positions.begin() ; 
		tid != best_rule_positions.end();
		++tid) {
	      wordType& corpus_value = corpus[i][*p][STATE_START + *tid];
	      classifIndex.erase(corpus_value, i, *p);
	      ::swap(corpus_value, prevPositions[pp][*tid]);
	      classifIndex.insert(corpus_value, i, *p);
	      ON_DEBUG(assert(classifIndex.find(corpus_value, i, *p) != classifIndex.end(corpus_value)));
	    }
	  }
	  // corpus is now in its original condition

	  for(rulep_hash_set::iterator rl=pRules.begin() ; rl!=pRules.end() ; ++rl) {
	    Rule& rule1 = const_cast<Rule&>(**rl);
	    TargetTemplate::pos_vector& poss = TargetTemplate::Templates[rule1.target.tid].positions;
	    if(! rule1.predicate.test(corpus[i], k)|| !rule1.constraint_test(corpus[i][k])) { // r_p(b(s)) == false
	      int pp = 0;
	      for(TargetTemplate::pos_vector::const_iterator jj=poss.begin() ; jj!=poss.end(); ++jj, ++pp) {
		if(TargetTemplate::value_is_correct(rule1.target.vals[pp], old_corpus[k][TRUTH_START + *jj])) {
		  if(! TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj])) {
		    rule1.good += costs[i];

		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- goods have been increased" << endl;
		  }
		} 
		else
		  if(TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj])) {
		    rule1.bad += costs[i];
					
		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- bads have been increased" << endl;
		  }
	      }
	    }
	    else { // r_p(b(s)) == true
	      int pp = 0;
	      for(TargetTemplate::pos_vector::const_iterator jj=poss.begin() ; jj!=poss.end(); ++jj, ++pp) {
		if(TargetTemplate::value_is_correct(rule1.target.vals[pp], old_corpus[k][TRUTH_START + *jj])) {
		  if(! TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj]) && 
		     TargetTemplate::value_is_correct( corpus[i][k][STATE_START + *jj],  corpus[i][k][TRUTH_START + *jj])) {
		    rule1.good += costs[i];
					
		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- goods have been increased" << endl;
		  }
		}
		else
		  if(  TargetTemplate::value_is_correct(old_corpus[k][STATE_START + *jj], old_corpus[k][TRUTH_START + *jj]) &&
		       ! TargetTemplate::value_is_correct( corpus[i][k][STATE_START + *jj],  corpus[i][k][TRUTH_START + *jj])) {
		    rule1.bad += costs[i];

		    if(V_flag>=3)
		      cerr << rule1.printMe() << " good: " << rule1.good << " bad: " << rule1.bad << " -- bads have been increased" << endl;
		  }
	      }
	    }
	  }

	  processed[k] = true;
	}
      }

      int pp = 0;
      for(int1D::iterator p = placesToChange.begin() ; p!=placesToChange.end() ; ++p, ++pp) {
	for(TargetTemplate::pos_vector::const_iterator tid = best_rule_positions.begin() ; 
	    tid != best_rule_positions.end();
	    ++tid) {
	  wordType& corpus_value = corpus[i][*p][STATE_START + *tid];
	  classifIndex.erase(corpus_value, i, *p);
	  corpus_value = best_rule_target[*tid];
	  classifIndex.insert(corpus_value, i, *p);
	}
      }
	  
      // Add to the hash of rules all the rules that were newly generated at this step
      for(rule_hash_set::iterator rl = newRules.begin() ; rl!=newRules.end() ; ++rl)
	allRules.insert(*rl);
    }

    rule_hash_set::const_iterator p = allRules.find(bestRule);
    if(p->good != 0 || p->bad != 0)
      cerr << "!! " << bestRule.printMe() << " does not have 0 goods and bads (good: " << 
	bestRule.good << ", bad: " << bestRule.bad << ") !!" << endl;
  }
  else  // This is the original behaviour of TBL
    for(position_vector::iterator pos=best_rule_applic_places.begin() ; 
	pos!=best_rule_applic_places.end() ; 
	++pos) {
      int tt = 0;
      for(TargetTemplate::pos_vector::const_iterator tid = best_rule_positions.begin() ; 
	  tid != best_rule_positions.end();
	  ++tid, ++tt) {
	wordType& corpus_value = corpus[pos->first][pos->second][STATE_START + *tid];
	classifIndex.erase(corpus_value, pos->first, pos->second);
	corpus_value = best_rule_target[*tid];
	classifIndex.insert(corpus_value, pos->first, pos->second);
      }
    }
  if (erase_bad_rules>-1) {
    static rule_vector tmp;
    tmp.clear();

    for(rule_hash_set::iterator thisRule = allRules.begin() ;
	thisRule != allRules.end() ; ++thisRule) {
      if(thisRule->good == 0 && thisRule->bad>3)
	// 		allRules.erase(thisRule);
	tmp.push_back(*thisRule);
    }

    if(v_flag)
      cerr << "Removing " << tmp.size() << " rules." << endl;
	
    for(int i=0 ; i<tmp.size() ; i++) {
      if(V_flag > 4)
	cerr << i << ": Erasing rule " << tmp[i].printMe() << " (" << tmp[i].good << "," << tmp[i].bad << ")" << endl;
      allRules.erase(allRules.find(tmp[i]));
    }
  }
}

void usage(const string& progname ) {
  cerr << "USAGE: " << progname << " trainingfile rulesfile <options>" << endl
       << "OPTIONS: " << endl
       << "  -F <file>                - defines the parameter file (if not defined uses the shell variable $DDINF)" << endl
       << "  -threshold <thr>         - sets the treshold for learning (will stop when a rule with this score is reached - default 3)" << endl
       << "  -allPositiveRules <no>   - will generate at the end all the rules that have 0 bad counts and good above no - see documentation" << endl
       << "  -minPositiveScore <val>  - will enforce this minimum threshold if the number in the previous flag is not positive (either n% or -n)" << endl
       << endl;
}

int main(int argc, char *argv[]) {
  timer tm;
  if (argc < 3) {
    usage(argv[0]);
    exit(1);
  }

  bool f_flag = true;
  bool o_flag = false;

  string ruleTemplateFile = "";
  string rule_file = "";
  string tree_file = "tree_file.dat";
  string base_name = "memt";

  pred_file = "";

  for (int i = 3; i < argc; i++) {
    if (!strcmp("-templates", argv[i]) && i+1 < argc) 
      ruleTemplateFile = argv[++i];
    else if(!strcmp("-f", argv[i]))
      f_flag = true;
    else if(!strcmp("-v", argv[i])) {
      v_flag = true;
      V_flag = 1;
    }
    else if(!strcmp("-V",argv[i])) {
      V_flag = atoi1(argv[++i]);
      v_flag = 1;
    }
    else if(!strcmp("-c", argv[i]))
      force_compute = true;
    else if(!strcmp("-o", argv[i])) {
      f_flag = false;
      o_flag = true;
    }
    else if(!strcmp("-F", argv[i])) {
      Params::Initialize(argv[++i]);
    }
    else if(!strcmp("-outputType",argv[i])) {
      output_type = argv[++i];
    }
    else if(!strcmp("-baseName", argv[i]))
      base_name = argv[++i];
    else if(!strcmp("-readCounts",argv[i])) {
      pred_file = argv[++i];
      test_mode = true;
    }
    else {
      cerr << "Invalid option " << argv[i] << endl;
      exit(-1);
    }
  }

  if (output_type != "snow" && output_type != "memt") {
    cerr << "Don't know how to output for type " << output_type << "!" << endl;
    exit (113);
  }

  log_me_in(argc, argv);
  RuleTemplate::Initialize();
  bool is_stdin = false;

  string file_name = argv[1];

  if (file_name == "-") {
    is_stdin = true;
    timeb * tp = new timeb;
    ftime(tp);
    srand(tp->millitm);
    string new_file = "/tmp/temp";
    ipfstream tmpfile ("|tempfile");
    if (tmpfile) {
      tmpfile >> new_file;
      tmpfile.close();
    } else {
      int i = rand();
      new_file += itoa(i);
    }

    file_name = new_file;
    string line;
    ofstream of(file_name.c_str());
    while (getline(cin, line)) 
      of << line << endl;
    of.close();
  }
  studyData(file_name);

  Rule::Initialize();

  const Dictionary& dict = Dictionary::GetDictionary();

  if( ceil(log(static_cast<double>(dict.size()))/log(static_cast<double>(2))) > 8*sizeof(wordType) ) {
    cerr << "The representation size is wrong!! There are " << dict.size() << " words, "
	 << "but only " << static_cast<long long>(1ll << 8*sizeof(wordType)) << " possible values." << endl
	 << "Edit the src/Makefile file, modifying the variable WORD_TYPE to the next possible type " << endl 
	 << "(i.e. char -> short int, short int -> int, etc). Just to be safe," << endl
	 << "remember to do a \"make clean\" before recompilation. See the documentation for further explanation." << endl;
    cerr << "Aborting the execution." << endl << endl;
    exit(199);
  }

  if(v_flag)
    cerr << "The dictionary has " << Dictionary::GetDictionary().size() << " words!" << endl;

  readInData (const_cast<char *>(file_name.c_str()));
  initializeDefaultCosts(corpus.size());
  erase_bad_rules = Params::GetParams().valueForParameter("ERASE_USELESS_RULES", -1);

//   generate_index();

//   ostream *rules;
//   smart_open(rules, argv[2]);
  smart_open(outfile, argv[2]);


  allRules.turn_on();
  computeScoreForAllRules();

  if(v_flag)
    cerr << "Cleaning up: string sets";
  PrefixSuffixAddPredicate::Destroy();
  ContainsStringPredicate::Destroy();

  if(v_flag)
    cerr << "... predicate space";
  //    Predicate::clear_memory();
  Predicate::deallocate_all();
  if(v_flag)
    cerr << "... corpus";
  clear_corpus();

  if(v_flag)
    cerr << "... dictionary";

  Dictionary::GetDictionary().destroy();

  if(v_flag)
    cerr << "... svector space";
  svector<wordType>::deallocate_all();
  svector<char>::deallocate_all();

  if(v_flag)
    cerr << "=> done." << endl;
  cerr << "Overall running time: " << tm.time_since_beginning() << " (" << tm.milliseconds_since_beginning() << " milliseconds) " << endl;

  if(is_stdin) {
    if(v_flag)
      cerr << "Removing the temporary file " << file_name << endl;
    execlp("/bin/rm", "rm", file_name.c_str(), (char *) NULL);
    // execvp("/bin/rm", const_cast<char **>(args));
  }
}
