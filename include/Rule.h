// -*- c++ -*-
/*
  Defines the Rule class and associated containers.

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

#ifndef __RULE_H
#define __RULE_H

#include <vector>
#if __GNUG__ < 3
#include <hash_map>
#include <hash_set>
#include <slist>
#else
#include <ext/hash_map>
#include <ext/hash_set>
#include <ext/slist>
#endif

#include <string>
#include "typedef.h"
#include "Dictionary.h"
#include "Predicate.h"
#include "Constraint.h"
#include "Target.h"
#include "Params.h"

// This is an abstract class specifying the interface for the AtomicPredicate
// category.
class Rule {
public:
  // this constructor is used while rule learning.
  Rule(int p_tid, int t_tid, const wordType1D& p_tok, const wordType1D& t_tok);
  Rule(int p_tid, int t_tid, const wordTypeVector& p_tok, const wordTypeVector& t_tok);
  // this constructor is used when we're reading in a rule from a file
  Rule(string1D &ruleComponents);
  // and this one is used when we're copying a rule.
  Rule(const Rule &anotherRule):
    predicate(anotherRule.predicate),
    target(anotherRule.target),
    hashIndex(anotherRule.hashIndex),
#if DEBUG >= 3
    rule_name(anotherRule.rule_name),
#endif
    good(anotherRule.good),
    bad(anotherRule.bad)
  {}

  Rule(const Predicate& p, const Target& t):
    predicate(p),
    target(t),
    good(0),
    bad(0)
  {
    ON_DEBUG(rule_name = printMe());
    hashIndex = hashVal();
  }

  Rule(): target(0), hashIndex(0)
  {
    ON_DEBUG(rule_name = "");
  }

  ~Rule(){}

  bool operator () (const wordType2D& corpus, int word) const {
    return test(corpus, word);
  }

  bool operator () (const wordType2D& corpus, int word, const float2D& context_prob) const {
    return test(corpus, word, context_prob);
  }

  bool constraint_test(const wordType1D& features) const {
    return Constraints.test(features, target);
  }

  bool test(const wordType2D &corpus, int word) const;
  int hashVal(void) const;

  // A probabilistic version of the applicability of a rule: 
  // based on the context probabilities, return the probability of firing.
  double test(const wordType2D& corpus, int word, const float2D& context_prob) const;

  string printMe() const;

  // Updates the counts of the rule as it would do if the rule applies to
  // the given vector
  void update_counts(const wordType1D& vect, int factor) const {
    target.update_counts(vect, factor, good, bad);
  }

  void update_bad_counts(const wordType1D& vect, int factor) const {
    target.update_bad_counts(vect, factor, bad);
  }

  Predicate predicate;
  Target target;
  // change word to this state if constraints hold
  //   wordType targetState;  

  // the stuff that identifies each unique rule.
  int hashIndex;
  
  ON_DEBUG(string rule_name);
  mutable scoreType good;
  mutable scoreType bad;

  // checks if two rules are equal.
  bool operator< (const Rule& rule) const {
    return rule.better(*this, rule.good-rule.bad);
  }
  
  static const int MORE_PREDICATES_FIRST = 0;
  static const int LESS_PREDICATES_FIRST = 1;
  static const int TEMPLATE_FILE_ORDER = 2;
  // The choice of rules is done as follows:
  // 1. Choose the rule with higher score.
  // 2. If scores are equal, choose the one that's more specific (has more predicates intantiated).
  // 3. If equal, choose the one with a higher targetStateID.
  // 4. If equal, choose the one with a lower predicate ID.
  // 5. If equal, choose the one that whose tokens are lower in lexicographic distance.
  bool better(const Rule& rule, int score) const {
    int r_score = rule.good-rule.bad;
    static int size_ordering = Params::GetParams().valueForParameter("ORDER_BASED_ON_SIZE", MORE_PREDICATES_FIRST);

    switch(size_ordering) {
    case LESS_PREDICATES_FIRST:
      return score > r_score ||
	score==r_score && 
	( 
	 predicate.tokens.size() < rule.predicate.tokens.size() ||
	 predicate.tokens.size() == rule.predicate.tokens.size() && 
	 ( 
	  rule.target < target ||
	  target == rule.target && 
	  ( predicate.template_id < rule.predicate.template_id || 
	    predicate.template_id == rule.predicate.template_id && rule.predicate.tokens < predicate.tokens
	    )
	  )
	 );
    case MORE_PREDICATES_FIRST:
      return score > r_score ||
	score==r_score && 
	( predicate.tokens.size() > rule.predicate.tokens.size() ||
	  predicate.tokens.size() == rule.predicate.tokens.size() && 
	  (
	   rule.target < target ||
	   target == rule.target && 
	   ( predicate.template_id < rule.predicate.template_id || 
	     predicate.template_id == rule.predicate.template_id && rule.predicate.tokens < predicate.tokens
	     )
	   )
	  );
    case TEMPLATE_FILE_ORDER:
    default:
      return 
	score > r_score ||
	score==r_score &&
	( 
	 rule.target < target ||
	 target == rule.target && 
	 ( predicate.template_id < rule.predicate.template_id || 
	   predicate.template_id == rule.predicate.template_id && rule.predicate.tokens < predicate.tokens
	   )
	 );
    }
  }

  bool operator==(const Rule& rule) const {
    return target == rule.target && predicate == rule.predicate;
  }

  Rule& operator= (const Rule& rule) {
    if (this != &rule) {
      predicate = rule.predicate;
      target = rule.target;
      good = rule.good;
      bad = rule.bad;
      hashIndex = rule.hashIndex;
      ON_DEBUG(rule_name = rule.rule_name);
    }
    return *this;
  }

  int get_least_frequent_feature_position() const {
    return predicate.get_least_frequent_feature_position();
  }

  static void Initialize();

  static ConstraintSet Constraints;
};

namespace HASH_NAMESPACE {
  template <>
  struct hash<Rule> 
  {
    size_t operator()(const Rule& rule) const {
      return rule.hashIndex;
    }
  };


  template <>
  struct hash<Predicate> {
    size_t operator()(const Predicate& p) const {
      return p.hashIndex;
    }
  };

  template <>
  struct hash<const Predicate*> {
    size_t operator()(const Predicate* p) const {
      return p->hashIndex;
    }
  };
  
  template <>
  struct hash<Rule*> {
    size_t operator()(const Rule* p) const {
      return p->hashIndex;
    }
  };
}

namespace std {
  template <>
  struct equal_to<const Rule*>: public binary_function<const Rule*, const Rule*, bool> {
    bool operator() (const Rule* p1, const Rule* p2) {
      return *p1 == *p2;
    }
  };
  
}

struct equalto {
  bool operator() (const Predicate* p1, const Predicate* p2) {
    return *p1 == *p2;
  }
};
  
// This is a cooked up structure that acts just a hash of rules
// but it represents them based on a hash table of predicates.
// It helps saving up space (since all the rules with the same 
// predicate share it) and it's easier to identify all rules with
// the same predicate (needed to generate "bad counts").
typedef vector<Rule*> rule_list_type;

typedef HASH_NAMESPACE::hash_map<const Predicate*, rule_list_type, HASH_NAMESPACE::hash<const Predicate*>, equalto > predicate_list_hash_map;

class rule_hash;

class rule_hash_const_iterator {
public:
  typedef std::pair<predicate_list_hash_map::const_iterator, rule_list_type::const_iterator> rep_type;
  typedef predicate_list_hash_map::const_iterator predicate_iterator;
  typedef rule_list_type::const_iterator list_iterator;
  typedef Rule value_type;
  typedef rule_hash_const_iterator self;

  rule_hash_const_iterator(rule_hash& p, const predicate_iterator& p_it, const list_iterator& l_it): parent(&p), pred_it(p_it), list_it(l_it) {
  }

  ~rule_hash_const_iterator() {}

  void increment();

  Rule& operator* ();

  rule_hash_const_iterator& operator ++ () {
    increment();
    return *this;
  }

  rule_hash_const_iterator operator ++ (int) {
    self obj = *this;
    increment();
    return obj;
  }

  bool operator == (const rule_hash_const_iterator& it) const {
    return pred_it == it.pred_it && list_it == it.list_it;
  }

  bool operator != (const rule_hash_const_iterator& it) const {
    return !operator == (it);
  }

  Rule* operator->();

  HASH_NAMESPACE::hash_set<Rule>::iterator rule_iterator();

private:
  rule_hash* parent;
  predicate_iterator pred_it;
  list_iterator list_it;
};

class rule_hash {
  friend class rule_hash_const_iterator;
public:
  typedef predicate_list_hash_map pred_list_rep_type;
  typedef HASH_NAMESPACE::hash_set<Rule> real_rep_type;
  typedef real_rep_type::iterator iterator;
  typedef real_rep_type::value_type value_type;
  typedef real_rep_type::key_type key_type;

  rule_hash():on(false) {}
  
  rule_hash(const rule_hash& other_hash): storage(other_hash.storage), pred_list(other_hash.pred_list), on(other_hash.on) {
  }

  rule_hash& operator= (const rule_hash& rh) {
    if(&rh != this) {
      storage = rh.storage;
      pred_list = rh.pred_list;
      on = rh.on;
    }
    return *this;
  }

  pair<iterator, bool> insert(const value_type& key) {
    // First, try to add the target to the known list, if it's not already there
    pair<real_rep_type::iterator,bool> p = storage.insert(key);
    if(on && p.second) {				// The rule is not in the rule set => we need to add it.
      const Predicate* key = &p.first->predicate;
      rule_list_type& list = pred_list[key];
      rule_list_type::iterator lit = ::find(list.begin(), list.end(), &(*p.first));
      if(lit == list.end())
	list.push_back(const_cast<Rule*>(&(*p.first)));
    }

    // Then add the rule to the storage
    return p;
  }

  void swap(rule_hash& hash_set) {
    storage.swap(hash_set.storage);
    pred_list.swap(hash_set.pred_list);
    ::swap(hash_set.on, on);
  }

  void clear() {
    if(on) {
      for(pred_list_rep_type::iterator it = pred_list.begin();
	  it!=pred_list.end();
	  ++it) {
	it->second.clear();
      }
      pred_list.clear();
    }
    storage.clear();
  }

  void destroy() {
    pred_list.clear();
    storage.clear();
    real_rep_type tmp1;
    tmp1.swap(storage);
    pred_list_rep_type tmp2;
    pred_list.swap(tmp2);
  }

  void erase(iterator pos) {
    if(on) {
      pred_list_rep_type::iterator it = pred_list.find(&pos->predicate);
      if(it != pred_list.end()) {
	rule_list_type::iterator lit = ::find(it->second.begin(), it->second.end(), &(*pos));
	if(lit == it->second.begin() && it->second.size()>1) {
	  // This is the more complicated case - we need to recreate the pairing betweeen the
	  // predicate pointer of the first rule and the list of rule pointers.
	  static rule_list_type lst;
	  it->second.erase(lit);
	  lst = it->second;
	  pred_list.erase(it);
	  pred_list.insert(make_pair(&(lst[0]->predicate),lst));
	} else {
	  it->second.erase(lit);
	  if(it->second.size() == 0)
	    pred_list.erase(it);
	}
      }
    }
    storage.erase(pos);
  }

  void turn_on() {
    on = true;
  }

  void turn_off() {
    on = false;
  }

  bool is_on() const {
    return on;
  }

  size_t size() {
    return storage.size();
  }

  iterator find(const Rule& rule) const {
    return storage.find(rule);
  }

  iterator find(Rule& rule) {
    return storage.find(rule);
  }

  iterator begin() {
    return storage.begin();
  }

  iterator end() const {
    return storage.end();
  }

  rule_hash_const_iterator pbegin() {
    ON_DEBUG(assert(on));
    if(pred_list.size() > 0)
      return rule_hash_const_iterator(*this, pred_list.begin(), pred_list.begin()->second.begin());
    return pend();
  }

  rule_hash_const_iterator pend() {
    ON_DEBUG(assert(on));
    return rule_hash_const_iterator(*this, pred_list.end(), fake_list.end());
  }

  rule_hash_const_iterator pbegin(const Predicate& pred) {
    ON_DEBUG(assert(on));
    pred_list_rep_type::iterator it = pred_list.find(&pred);
    if (it == pred_list.end())
      return rule_hash_const_iterator(*this, pred_list.end(), fake_list.end());
    else 
      return rule_hash_const_iterator(*this, it, it->second.begin());
  }

  rule_hash_const_iterator pend(const Predicate& pred) {
    ON_DEBUG(assert(on));
    pred_list_rep_type::const_iterator it = pred_list.find(&pred);
    if (it == pred_list.end())
      return rule_hash_const_iterator(*this, pred_list.end(), fake_list.end());
    else {
      pred_list_rep_type::const_iterator it1 = it;
      ++it1;
      if(it1 == pred_list.end())
	return rule_hash_const_iterator(*this, it1, fake_list.end());
      else
	return rule_hash_const_iterator(*this, it1, it1->second.begin());
    }
  }

  void compute_space(int& s1, int& s2) {
    s1 = s2 = 0;
    for(pred_list_rep_type::const_iterator it = pred_list.begin() ;
	it!=pred_list.end() ; ++it) {
      s1 += it->second.size();
      s2 += it->second.capacity();
    }
  }

  static rule_list_type fake_list;
	
public:
  // A pointer to the hash_set containing the rules
  real_rep_type storage;
  pred_list_rep_type pred_list;
  bool on;
};


inline void rule_hash_const_iterator::increment() {
  if (pred_it == parent->pred_list.end())
    return;
  ++list_it;
  if (list_it == pred_it->second.end()) {
    pred_it++;
    if(pred_it == parent->pred_list.end()) { // We reached the end of the world.. :)
      list_it = parent->fake_list.end();
      return;
    }
    list_it = pred_it->second.begin();
  }
}

inline Rule& rule_hash_const_iterator::operator*()  {
  static Rule fake_rule;
  if(pred_it == parent->pred_list.end() || list_it == pred_it->second.end()) {
    cerr << "Trying to access a rule at the end of the world" << endl;
    return fake_rule;
  }
  
  return const_cast<Rule&>(*const_cast<const Rule*>(*list_it));
}

inline Rule* rule_hash_const_iterator::operator->()  {
  static Rule fake_rule;
  if(pred_it == parent->pred_list.end() || list_it == pred_it->second.end()) {
    cerr << "Trying to access a rule at the end of the world" << endl;
    return &fake_rule;
  }
  return const_cast<Rule*>(*list_it);
}

inline HASH_NAMESPACE::hash_set<Rule>::iterator rule_hash_const_iterator::rule_iterator() {
  static Rule fake_rule;
  if(pred_it == parent->pred_list.end() || list_it == pred_it->second.end()) {
    cerr << "Trying to access a rule at the end of the world" << endl;
    return parent->end();
  }
  static Rule r;
  r = **list_it;

  return parent->find(r);
}  

class RuleTemplate {
public:
  typedef HASH_NAMESPACE::hash_set<Rule> rule_set;
  typedef vector<RuleTemplate> RuleTemplate_vector;

  RuleTemplate(int ptid, int ttid): pred_tid(ptid), target_tid(ttid) {
    if(pt_list.size() <= ptid)
      pt_list.resize(ptid+1);
    if(find(pt_list[ptid].begin(), pt_list[ptid].end(), ttid) == pt_list[ptid].end())
      pt_list[ptid].push_back(ttid);
  }

  static string1D TemplateNames;
  static RuleTemplate_vector Templates;
  static Dictionary name_map;
  static int2D pt_list;
  static HASH_NAMESPACE::hash_map<string, string> variables, rvariables;

  static void instantiate(const wordType2D& corpus, int sample_ind, int pred_tid, 
			  rule_set& instances, bool generateBasedOnPredicate = false, 
			  bool forced_generation = false);

  void identify_strings(wordType word_id, wordType_set& wrds) const {
    PredicateTemplate::Templates[pred_tid].identify_strings(word_id, wrds);
  }

  void identify_strings(const wordType1D& word_id, wordType_set& wrds) const {
    PredicateTemplate::Templates[pred_tid].identify_strings(word_id, wrds);
  }

  static void AddTemplate(int ptid, int ttid, const string& str) {
    if(find(TemplateNames.begin(), TemplateNames.end(), str) == TemplateNames.end()) {
      Templates.push_back(RuleTemplate(ptid, ttid));
      TemplateNames.push_back(str);
    }
  }

  static void Initialize();
  
protected:
  int pred_tid, target_tid;
};

#endif
