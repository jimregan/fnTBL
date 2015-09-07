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
#include <stdio.h>

#if __GNUC__ < 3
#include <hash_set>
#else
#include <ext/hash_set>
#endif /* __GNUC__ */

#include "TBLTree.h"
#include "Dictionary.h"
#include "Node.h"
#include "Params.h"
#include <math.h>
#include "line_splitter.h"
#include "Target.h"

extern wordType3D corpus;
extern wordType3DVector ruleTrace;  

int Node::LastID = 0;
extern bool v_flag;

ostream& operator << (ostream& ostr, const Node& nd) {
  const Dictionary& dict = Dictionary::GetDictionary();

  ostr << "ID " << nd.ID << " " << "RULE_ID " << nd.ruleID << " ";

  int1D order(nd.classCounts.size());
  iota(order.begin(), order.end(), 0);

  sort(order.begin(), order.end(), ReverseSorter<int>(nd.classCounts));

  if(! nd.yesChild) {
    int l=0;
    for(; l<order.size() && nd.classCounts[order[l]]>0 ; l++);
    ostr << l ;

    for(int i=0 ; i<l ; i++)
      ostr << " " << dict[order[i]] << " " << nd.classCounts[order[i]];
  }
  else 
    ostr << 0;

  return ostr;
}

istream& operator >> (istream& istr, Node& nd) {
  Dictionary& dict = Dictionary::GetDictionary();

  static string str1, str2;
  int l, val;
  
  istr >> str1 >> nd.ID >> str2 >> nd.ruleID >> l;

  nd.classCounts.clear();
  nd.classCounts.resize(Dictionary::num_classes);
  fill(nd.classCounts.begin(), nd.classCounts.end(), 0);

  for(int i=0 ; i<l ; i++) {
    istr >> str1 >> val;
    int index = dict.insert(str1);
    if (index >= dict.num_classes)
      cerr << "Aha ! index: " << index << " " << str1 << " " << nd.ID << " " << nd.ruleID << endl;
    nd.classCounts[ index ] = val;
  }

  return istr;
}

//=========================================================================
// This one is used when we're creating a node recursively from a TBL split
//=========================================================================

Node::Node(example_index1D &myExamples)
{
  examples = myExamples;
  ID = LastID++;
  createLeaf();
  yesChild = noChild = NULL;
  ruleID = -1;
}

// ==========================================================================
// This function is used when we're reading in a node from a list.
// The format of the file is:
// - first all the rules used are listed (including the ones obtained from 
//   growing the obtained decision tree.
// - then the tree is represented in a breadth-first manner.
// ==========================================================================

void Node::initialize(const string &line)
{
  line_splitter ls;
  ls.split(line);
  const Dictionary& dict = Dictionary::GetDictionary();

  ID = atoi1(ls[1].c_str());

  classCounts.resize(Dictionary::num_classes);
  fill(classCounts.begin(), classCounts.end(), 0);

  for (int i = 2; i < (int)ls.size(); ++i) {
    if (ls[i] == "RULE_ID") 
      ruleID = atoi1(ls[++i]);
    else {
      wordType chunkIndex = dict.getIndex(ls[i]);
      int thisCount = atoi1(ls[++i].c_str());
      classCounts[chunkIndex] = thisCount;
      totalCount += thisCount;
    }
  }
}

// ==========================================================================
// This function splits the samples in the node based on the current rule.
// The split is done only if the resulting number of samples in both children
// is bigger than REASONABLE_SPLIT, to avoid "over-splitting".
// ==========================================================================

void Node::split(int rule_no)
{
  static unsigned max_ind = TBLTree::rules.size();
  if(rule_no == max_ind) {
    return;
  }
  static int REASONABLE_SPLIT = Params::GetParams().valueForParameter("REASONABLE_SPLIT", 10);
  // tell the example that it's currently at this node.

  int yes = 0, no = 0, pos = 0;
  wordType nextYes=max_ind, nextNo=max_ind;

  static bit_vector applies;
  applies.resize(examples.size());
  applies.clear();
  
  for (example_index1D::iterator sample = examples.begin() ;
       sample != examples.end() ; sample++, pos++) {
    const wordTypeVector& rule_list = ruleTrace[sample->first][sample->second];
    wordTypeVector::const_iterator it = lower_bound(rule_list.begin(), rule_list.end(), rule_no);
    if (it==rule_list.end())
      no++;
    else
      if(*it == rule_no) {
	yes++;
	applies[pos] = true;
	if(it+1 != rule_list.end())
	  nextYes = min(nextYes, *(it+1));
      }
      else {
	nextNo = min(nextNo, *it);
	no++;
      }
  }

  if(yes >= REASONABLE_SPLIT && no >= REASONABLE_SPLIT) {
    splitExamples(applies);
    ruleID = rule_no;
    clearCountsAndProbs();
    yesChild->split(nextYes);
    noChild->split(nextNo);
  } else 
    split(nextNo);
}

// This function is called if a split is found.
void Node::splitExamples(const bit_vector& yes_sample)
{
  // first partition the examples into yes and no
  example_index1D yesExamples, noExamples;
  bit_vector::const_iterator is_yes_sample = yes_sample.begin();

  for (example_index1D::iterator example = examples.begin();
       example != examples.end(); ++example, ++is_yes_sample)
    if(*is_yes_sample)
      yesExamples.push_back(*example);
    else
      noExamples.push_back(*example);

  yesChild = new Node(yesExamples);
  noChild = new Node(noExamples);
  example_index1D temp;
  examples.swap(temp);
}

// This function is called when all rules have been applied.
//  main thing to do is to get the counts
void Node::createLeaf(void)
{
  classCounts.resize(Dictionary::num_classes);
  fill(classCounts.begin(), classCounts.end(), 0);
  string truth_sep = Params::GetParams()["TRUTH_SEPARATOR"];

  for (example_index1D::iterator example = examples.begin(); 
       example != examples.end(); ++example) {
    if(truth_sep != "" &&  corpus[example->first][example->second][TargetTemplate::TRUTH_START] >= Dictionary::num_classes) {
      static line_splitter ts(truth_sep);
      static const Dictionary& dict = Dictionary::GetDictionary();
      ts.split(dict[corpus[example->first][example->second][TargetTemplate::TRUTH_START]]);
      for(int it=0 ; it<ts.size() ; it++)
	classCounts[dict[ts[it]]]++;
    } else {
      classCounts[corpus[example->first][example->second][TargetTemplate::TRUTH_START]]++;
    }
  }
  // 	classCounts[corpus[example->first][example->second][TargetTemplate::TRUTH_START]]++;
}

// ============================================================================================== //
// ==  There are 2 versions of this member function. The first generates all the rules that    == //
// ==  apply to the samples in the node, and constructs for each one a list of the rules that  == //
// ==  apply on that particular position - for efficiency.                                     == //
// ============================================================================================== //

void Node::growDT(void)
{
  rule_hash_map dt_rules;

  static rule_list_type nodeRuleApplied;
  int local_corpus_size = examples.size();

  for(int i=0 ; i< local_corpus_size ; i++)
    nodeRuleApplied[i].clear();
  nodeRuleApplied.clear();
  computeEntropy();

  if(fabs(entropy) > 1e-3) { 
    // Only if the entropy is non-zero - otherwise, it's an exercise in futility..
    createRules(dt_rules, nodeRuleApplied);
    growDT(dt_rules, nodeRuleApplied);
  }
}

// ======================================================================== 
// the next functions will attempt to grow a new branch from an existing node.
// it stops if the majority chunk for one of its resulting children might
// be different.
// ======================================================================== 

void Node::growDT(rule_hash_map& dt_rules, const rule_list_type& rules_applied) {
  static int REASONABLE_DT_SPLIT = Params::GetParams().valueForParameter("REASONABLE_DT_SPLIT", 10);

  if(examples.size() < REASONABLE_DT_SPLIT) {
    cerr << "CAN'T GROW DT ANY MORE FOR NODE " << ID << endl;
    return;
  }

  if(v_flag)
    cerr << "GROWING DT FOR NODE " << ID << " WITH " << examples.size() << " EXAMPLES" << endl;
  // only leaf nodes are grown.

  computeEntropy();

  int bestRule = findBestRule(dt_rules, rules_applied);

  if (bestRule != dt_rules.size()) {
    splitExamplesByRule(bestRule, rules_applied);
    ruleID = TBLTree::GetRuleIndex(dt_rules[bestRule]);
    dt_rules.erase(bestRule);
    if(yesChild) {
      clearCountsAndProbs();
      yesChild->growDT();
      noChild->growDT();
    }
  } 
}

void Node::computeEntropy() {
  // compute the entropy of the node
  int total = 0;
  entropy = 0;
  for (int1D::iterator count = classCounts.begin(); count != classCounts.end(); ++count) {
    total += *count;
    if (*count > 0) 
      entropy -= *count*log(static_cast<double>(*count));
  }

  if (total > 0) {
    entropy /= total;
    entropy += log(static_cast<double>(total));
  }
}

void Node::addSimpleTemplates(int1D& simple_predicate_template_ids,
			      int1D& simple_truth_template_ids) {
  PredicateTemplate::PredicateTemplate_vector& templates=PredicateTemplate::Templates;
  TargetTemplate::TargetTemplate_vector& ttemplates = TargetTemplate::Templates;
  static vector<string> v(1);
  v[0] = TargetTemplate::name_map.direct_access(0);
  TargetTemplate tt(v);
  ttemplates.push_back(tt);
  simple_truth_template_ids.push_back(ttemplates.size()-1);
  
  for(int i=0 ; i<PredicateTemplate::name_map.size()-1 ; ++i) {
    // 	static vector<string> v(1);
    string1D::const_iterator si;
    if((si=find(PredicateTemplate::TemplateNames.begin(), PredicateTemplate::TemplateNames.end(), PredicateTemplate::name_map[i])) ==
       PredicateTemplate::TemplateNames.end()) {
      PredicateTemplate::TemplateNames.push_back(PredicateTemplate::name_map[i]);
      v[0] = PredicateTemplate::name_map[i];
      PredicateTemplate pt(v);
      templates.push_back(pt);
      simple_predicate_template_ids.push_back(templates.size()-1);
    } else {
      simple_predicate_template_ids.push_back(si-PredicateTemplate::TemplateNames.begin());
    }

    RuleTemplate::AddTemplate(simple_truth_template_ids[0], simple_predicate_template_ids.back(), 
			      PredicateTemplate::name_map[i] + " " + TargetTemplate::name_map.direct_access(0) + "1");
  }
}

// create a batch of DT-like predicate rules and pick the best one.
void Node::createRules(rule_hash_map &dt_rules, rule_list_type& node_rules_applied)
{
  // To be able to create simple rules, we need a list of "simple predicate templates"
  // = templates that generate rules of the form "<feature>=<val> => <new_class>"
  static int1D simple_predicate_template_ids;
  static int1D simple_truth_template_ids;

  PredicateTemplate::PredicateTemplate_vector& templates = PredicateTemplate::Templates;
  //   TargetTemplate::TargetTemplate_vector& ttemplates = TargetTemplate::Templates;
  static unsigned short fake_rule_index = Dictionary::GetDictionary()["FAKE_CLASS"];
  static bool initialized = false;

  if(! initialized) {
    addSimpleTemplates(simple_predicate_template_ids, simple_truth_template_ids);
    // 	static vector<string> v(1);
    // 	v[0] = TargetTemplate::name_map.direct_access(0);
    // 	TargetTemplate tt(v);
    // 	ttemplates.push_back(tt);
    // 	simple_truth_template_ids.push_back(ttemplates.size()-1);
	
    // 	for(int i=0 ; i<PredicateTemplate::name_map.size()-1 ; ++i) {
    // 	  static vector<string> v(1);
    // 	  v[0] = PredicateTemplate::name_map[i];
    // 	  PredicateTemplate pt(v);
    // 	  templates.push_back(pt);
    // 	  simple_predicate_template_ids.push_back(templates.size()-1);
    // 	  RuleTemplate::AddTemplate(simple_truth_template_ids[0], simple_predicate_template_ids.back(), 
    // 								PredicateTemplate::name_map[i] + " " + TargetTemplate::name_map.direct_access(0) + "1");
    // 	}
    initialized = true;
  }

  // We need to generate the simple rules that will discriminate between 
  // the samples.
  static wordTypeVector fake_vec(1);
  fake_vec[0] = fake_rule_index;
  for (example_index1D::iterator example = examples.begin(); example != examples.end();
       ++example) {
    short_int1D_pair1D& v = node_rules_applied[example->first];
    static int1D fake;
    short_int1D_pair p(example->second, fake);
    short_int1D_pair1D::iterator it = lower_bound(v.begin(), v.end(), p);

    for(int1D::iterator t_id = simple_predicate_template_ids.begin() ; t_id != simple_predicate_template_ids.end() ; ++t_id) {
      static wordType2DVector tk;
      tk.clear();
      templates[*t_id].instantiate(corpus[example->first], example->second, tk);
      for (wordType2DVector::const_iterator wi = tk.begin() ; wi!=tk.end() ; ++wi) {
	Rule r(*t_id, simple_truth_template_ids[0], *wi, fake_vec);
		
	if(it==v.end() || it->first != example->second)
	  it = v.insert(it, p);
		
	it->second.push_back( dt_rules.insert(r) );
      }
    }

    ON_DEBUG(assert(it!=v.end()); )
      sort(it->second.begin(), it->second.end(), less<int>());
  }
}

typedef map<unsigned, unsigned> sparse_vector;

int Node::findBestRule(rule_hash_map &dt_rules,
		       const rule_list_type& node_rules_applied)
{
  static float min_entropy_gain = atof1(Params::GetParams().valueForParameter("MINIMUM_ENTROPY_GAIN", string("0.05")));
  double bestEntropy = (entropy-min_entropy_gain)*examples.size();
  static int REASONABLE_DT_SPLIT = Params::GetParams().valueForParameter("REASONABLE_DT_SPLIT", 10);
  int bestRule = dt_rules.size();
  static string truth_sep = Params::GetParams()["TRUTH_SEPARATOR"];

  static vector<sparse_vector> counts;

  counts.resize(dt_rules.size());

  for(int i=0 ; i<dt_rules.size() ; i++)
    counts[i].clear();

  example_index1D::iterator example = examples.begin();
  rule_list_type::const_iterator i=node_rules_applied.begin();
  short_int1D_pair1D::const_iterator j=i->second.begin();

  while (example != examples.end() && i!=node_rules_applied.end()) {
    if(example->first < i->first)
      ++example;
    else 
      if(example->first > i->first) {
	++j;
	while(j == i->second.end()) {
	  ++i;
	  if(i==node_rules_applied.end())
	    break;
	  j = i->second.begin();
	}
      }
      else { //example->first == i->first
	if(example->second < j->first)
	  ++example;
	else if (example->second > j->first) {
	  ++j;
	  while(j == i->second.end()) {
	    ++i;
	    if(i==node_rules_applied.end())
	      break;
	    j = i->second.begin();
	  }
	} 
	else {
	  for(int1D::const_iterator k=j->second.begin() ; k!=j->second.end() ; ++k) {
	    if(truth_sep != "" &&  corpus[i->first][j->first][TargetTemplate::TRUTH_START] >= Dictionary::num_classes) {
	      static line_splitter ts(truth_sep);
	      static const Dictionary& dict = Dictionary::GetDictionary();
	      ts.split(dict[corpus[i->first][j->first][TargetTemplate::TRUTH_START]]);
	      for(int it=0 ; it<ts.size() ; it++)
		counts[*k][dict[ts[it]]]++;
	    } else {
	      // 			if(corpus[i->first][j->first][TargetTemplate::TRUTH_START] > Dictionary::num_classes)
	      // 			  cerr << "Hmm - " << i->first << " " << j->first << " " << corpus[i->first][j->first][TargetTemplate::TRUTH_START] << endl;
	      counts[*k][corpus[i->first][j->first][TargetTemplate::TRUTH_START]]++;
	    }
	  }
	  ++example;
	  ++j;
	  while(j == i->second.end()) {
	    ++i;
	    if(i==node_rules_applied.end())
	      break;
	    j = i->second.begin();
	  }
	}
      }
  }

  for(int i=0 ; i<dt_rules.size() ; i++) {
    const sparse_vector& cnts = counts[i];
    double posEnt = 0.0, negEnt = 0.0;
    unsigned int posN=0, negN = 0;
    int prev_ind = 0;
    for(sparse_vector::const_iterator j=cnts.begin() ; j!=cnts.end() ; ++j) {
      for(int k=prev_ind ; k<j->first ; k++) {
	int v = classCounts[k];
	if(v > 0)
	  negEnt -= v * log(static_cast<double>(v));
	negN += v;
      }
      prev_ind = j->first+1;
      int val = j->second;
      int nval = classCounts[j->first]-val;
      if(val > 0)
	posEnt -= val * log(static_cast<double>(val));
      if(nval > 0)
	negEnt -= nval*log(static_cast<double>(nval));
      posN += val;
      negN += nval;
    }

    for(int k=prev_ind ; k<Dictionary::num_classes ; k++) {
      int v = classCounts[k];
      if(v > 0)
	negEnt -= v * log(static_cast<double>(v));
      negN += v;
    }  

    if( posN> REASONABLE_DT_SPLIT && negN > REASONABLE_DT_SPLIT ) {
      posEnt /= posN;
      posEnt += log(static_cast<double>(posN));
      negEnt /= negN;
      negEnt += log(static_cast<double>(negN));
      double ent = posEnt*posN+negEnt*negN;
      if(ent < bestEntropy) {
	bestEntropy = ent;
	bestRule = i;
      }
    }
  }

  return bestRule;
}

void Node::splitExamplesByRule(int bestRuleID, const rule_list_type& rules_applied)
{
  // first partition the examples into yes and no
  example_index1D yesExamples, noExamples;
  example_index1D::iterator example = examples.begin();
  rule_list_type::const_iterator i=rules_applied.begin();
  short_int1D_pair1D::const_iterator j=i->second.begin();

  while (example != examples.end() && i!=rules_applied.end()) {
    if(example->first < i->first) {
      noExamples.push_back(*example);
      ++example;
    }
    else
      if(example->first > i->first)
	++i;
      else
	if(example->second > j->first) {
	  ++j;
	  while(j == i->second.end()) {
	    ++i;
	    if(i==rules_applied.end())
	      break;
	    j = i->second.begin();
	  }
	} else if(example->second < j->first) {
	  noExamples.push_back(*example);
	  example++;
	} else {
	  if(binary_search(j->second.begin(), j->second.end(), bestRuleID))
	    yesExamples.push_back(*example);
	  else
	    noExamples.push_back(*example);

	  ++example;
	  ++j;
	  while(j == i->second.end()) {
	    i++;
	    if(i==rules_applied.end())
	      break;
	    j = i->second.begin();
	  }
	}
  }

  while (example != examples.end()) {
    noExamples.push_back(*example);
    example++;
  }

  // now partition the rules into the two sides.
  yesChild = new Node(yesExamples);
  noChild = new Node(noExamples);
}


// ======================================================================
// UTILITY FUNCTIONS USED BY BOTH BUILD AND TEST TREE
// ======================================================================

const Node* Node::findClassOfSample(const example_index& example) const {
  if(yesChild == NULL || ruleID == -1)
    return this;

  wordTypeVector& vect = ruleTrace[example.first][example.second];
  if(binary_search(vect.begin(), vect.end(), ruleID))
    return yesChild->findClassOfSample(example);
  else
    return noChild->findClassOfSample(example);
}

string Node::probString(void) const {
  static double eps = atof1(Params::GetParams().valueForParameter("PROBABILITY_SMOOTHING_FACTOR", string("0.0")));

  if (probs.size() == 0) {
    int sz = classCounts.size();
    probs.resize(sz);
    copy(classCounts.begin(), classCounts.end(), probs.begin());
    totalCount = accumulate(classCounts.begin(), classCounts.end(), 0);
    // 	transform(probs.begin(), probs.end(), probs.begin(), bind2nd(divides<float>(), 1.0*totalCount));
    static const Dictionary& dict = Dictionary::GetDictionary();
    int pos=0;
    static char *bad = NULL;
    if (bad == NULL) {
      bad = new char [sz];
      fill(bad, bad+sz, 0);
      bad[dict["FAKE_CLASS"]] = 1;
      bad[dict["ZZZ"]] = 1;
      bad[dict["UNK"]] = 1;
    }

    for(float1D::iterator i=probs.begin() ; i!=probs.end() ; ++i, ++pos)
      if(! bad[pos])
	*i = (*i+eps) / (totalCount+sz*eps);
  }

  static const Dictionary& dict = Dictionary::GetDictionary();
  string probString = "";
  int1D temp_order(Dictionary::num_classes);
  iota(temp_order.begin(), temp_order.end(), 0);
  //   ReverseSorter<float> srtr(probs);
  sort(temp_order.begin(), temp_order.end(), ReverseSorter<float>(probs));
  
  for(int i=0 ; i<Dictionary::num_classes ; i++)
    if(fabs(probs[temp_order[i]])>1e-6) {
      probString += " " + dict[temp_order[i]];
      char probLine[1024];
      sprintf(probLine, " (%f)", probs[temp_order[i]]);
      probString += probLine;
    }
  return probString;
}

void Node::updateCounts() {
  if(yesChild == NULL)
    return;
  else {
    yesChild->updateCounts();
    noChild->updateCounts();
    for(int i=0 ; i<Dictionary::num_classes ; i++)
      classCounts[i] = yesChild->classCounts[i] + noChild->classCounts[i];
  }
}

// ================================================================================= //
// The hypothesis represents the class probability distribution for the surrounding  //
// context (if there is one - the samples are interdependent).                       //
// ================================================================================= //

void Node::computeProbs(const example_index& example, const float2D& hypothesis) {
  if(yesChild == NULL) {
    // The current node is a leaf => compute its prob distribution by
    // the regular method
	
    if(totalCount == 0)
      totalCount = accumulate(classCounts.begin(), classCounts.end(), 0);

    for(int i=0 ; i<Dictionary::num_classes ; i++) {
      probs[i] = classCounts[i]*1.0/totalCount;
    }
  } 
  else {
    // alpha is the probability that the current predicate applies,
    // given the context 
    double alpha = TBLTree::rules[ruleID](corpus[example.first], example.second, hypothesis);
	
    if(fabs(alpha) < 1e-6) {
      noChild->computeProbs(example, hypothesis);
      copy(noChild->probs.begin(), noChild->probs.end(), probs.begin());
    } 
    else 
      if(fabs(alpha-1) < 1e-6) {
	yesChild->computeProbs(example, hypothesis);
	copy(yesChild->probs.begin(), yesChild->probs.end(), probs.begin());
      }
      else {
	// This is a case when the split is done on the 
	// chunk tags => we need to go down both childs
		
	yesChild->computeProbs(example, hypothesis);
	noChild->computeProbs(example, hypothesis);
		
	for(int i=0 ; i<Dictionary::num_classes ; i++)
	  probs[i] = alpha * yesChild->probs[i] + (1-alpha) * noChild->probs[i];
      }
  }
}
