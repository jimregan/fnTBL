/*
  Defines the implementation for the class Rule.

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

#include "Rule.h"
#include <numeric>
#include "Params.h"
#include "line_splitter.h"

rule_list_type rule_hash::fake_list(0);
ConstraintSet Rule::Constraints;

HASH_NAMESPACE::hash_map<string, string> RuleTemplate::variables;
HASH_NAMESPACE::hash_map<string, string> RuleTemplate::rvariables;
string1D RuleTemplate::TemplateNames;
RuleTemplate::RuleTemplate_vector RuleTemplate::Templates;
Dictionary RuleTemplate::name_map;
int2D RuleTemplate::pt_list;

extern rule_hash allRules;

using namespace std;
using std::operator!=;

// initializes a rule from the corpus.
Rule::Rule(int p_tid, int t_tid, const wordType1D& p_tok, const wordType1D& t_tok): predicate(p_tid, p_tok), target(t_tid, t_tok) {
  bad = good = (scoreType)0.0; 

  hashIndex = hashVal();
  ON_DEBUG(rule_name = printMe());
}

Rule::Rule(int p_tid, int t_tid, const wordTypeVector& p_tok, const wordTypeVector& t_tok): predicate(p_tid, p_tok), target(t_tid, t_tok)
{
  bad = good = (scoreType)0.0; 

  hashIndex = hashVal();
  ON_DEBUG(rule_name = printMe());
}
// initializes a rule.
// This constructor is used when we're reading in a rule in from a file.
Rule::Rule(string1D &rule_components): target(0)
{
  static string arrow="=>";
  string1D::iterator pos = find(rule_components.begin(), rule_components.end(), arrow);
  if(pos==rule_components.end()) {
    cerr << "The rule \"" ;
    copy(rule_components.begin(), rule_components.end(), ostream_iterator<string>(cerr, " "));
    cerr << "\" is corrupt (does not contain the string =>). Please replace and restart." << endl;
    exit(1323);
  }

  string1D pred_part(pos-rule_components.begin()), truth_part(rule_components.end()-pos-1);
  copy(rule_components.begin(), pos, pred_part.begin());
  copy(pos+1, rule_components.end(), truth_part.begin());
  predicate.create_from_words(pred_part);
  target.create_from_words(truth_part);

  // first get out the target state
  hashIndex = hashVal();
  ON_DEBUG(rule_name = printMe());
}

// Test a rule on a position. Returns true iff the target state is not 
// the correct one, the predicate matches and the transformation does not 
// break any constraints.
bool Rule::test(const wordType2D &corpus, int word) const {
  return target.affects(corpus[word]) && predicate.test(corpus, word) && Constraints(corpus[word], target);
}

double Rule::test(const wordType2D& corpus, int word, const float2D& context_prob) const {
  return predicate.test(corpus, word, context_prob);
}

// Gets the corresponding token from the corpus.
string Rule::printMe() const {
  return predicate.printMe() + " => " + target.printMe();
}

// Calculates the index into a hash of rules.
int Rule::hashVal(void) const {
  // Have the hash value depend on the targetState as well.
  return target.hashVal(predicate.hashIndex);
}

void Rule::Initialize() {
  string constraints_file = Params::GetParams()["CONSTRAINTS_FILE"];
  if(constraints_file != "")
    Constraints.read(constraints_file);
}

void RuleTemplate::Initialize() {
  // Initialize the rule templates
  
  if (PredicateTemplate::name_map.size() != 0 && TargetTemplate::name_map.size() !=0 )
    return;

  const Params& p = Params::GetParams();
  string file = p.valueForParameter("FILE_TEMPLATE");
  istream *f;
  smart_open(f, file);
  string line;
  getline(*f, line);
  delete f;

  line_splitter ls;
  ls.split(line);

  string invalid_chars = "=_:-";
  string::size_type pos;
  
  static string arrow = "=>";
  const string1D& data = ls.data();
  string1D::const_iterator it = find(data.begin(), data.end(), arrow);
  if(it==data.end()) {
    cerr << "The FILE_TEMPLATE file is incorrect - the string \"=>\" is missing." << endl
	 << "The correct pattern is: <name_attr1> <name_attr2> .. <name_attr_k> => <name_truth1> .. <name_truth_p>" << endl
	 << "The assumption is that the guesses are the last p attributes.." << endl
	 << "Please correct the file and restart" << endl;
    exit(1233);
  }

  if(data.size() > (1ll << 8*sizeof(featureIndexType)) ) {
    cerr << "Oups - you're in trouble here. The \"featureIndexType\" type, "<< endl
	 << "which holds the features, is not adequate for your features" << endl
	 << "(it can hold only " << (1ll << 8*sizeof(featureIndexType)) << " features, while you have defined "
	 << data.size() << " features)." << endl
	 << "  You will have to recompile the program. Edit the Makefile file, " << endl 
	 << "modifying the parameter POSITION_TYPE to the next" << endl
	 << "possible type (i.e. char -> short int, short int -> int, etc)." << endl
	 << endl
	 << "Just to be safe, remember to do a \"make clean\" before recompilation." << endl;
    cerr << "Aborting the execution." << endl << endl;
    exit(1235);
  }

  int num_guesses = data.end()-it-1,
    imply_sign_pos = it-data.begin();
  
  TargetTemplate::STATE_START = imply_sign_pos - num_guesses;
  TargetTemplate::TRUTH_START = imply_sign_pos;
  TargetTemplate::TRUTH_SIZE = num_guesses;

  // First process the predicate part  

  for(line_splitter::iterator i=ls.begin() ; i!=it ; ++i) {
    for(string::iterator ch=invalid_chars.begin() ; ch!=invalid_chars.end() ; ++ch)
      if ((pos=i->find(*ch)) != i->npos) {
	cerr << "The feature " << *i << " has an restricted sign (" << *ch << ") in it; it is not allowed. Please rename the feature." << endl;
	exit(2);
      }
	
    PredicateTemplate::name_map.insert(*i);
    RuleTemplate::name_map.insert(*i);
  }
  
  for(line_splitter::const_iterator i=it+1 ; i!=ls.end() ; ++i) {
    for(string::iterator ch=invalid_chars.begin() ; ch!=invalid_chars.end() ; ++ch)
      if ((pos=i->find(*ch)) != i->npos) {
	cerr << "The feature " << *i << " has an restricted sign (" << *ch << ") in it; it is not allowed. Please rename the feature." << endl;
	exit(2);
      }
	
    TargetTemplate::name_map.insert(*(i-TargetTemplate::TRUTH_SIZE-1));
    RuleTemplate::name_map.insert(*i);
  }

  file = p.valueForParameter("RULE_TEMPLATES");
  smart_open(f, file);
  
  while (getline(*f, line)) {
    if(line[0] == '#')
      continue;

    ls.split(line);
    if(ls.size()==0)
      continue;
    if(ls.size()==3 && ls[1] == "=") {
      variables[ls[0]] = ls[2];
      rvariables[ls[2]] = "$"+ls[0];
      continue;
    }
	
    string1D& dt = ls.data();
    imply_sign_pos = find(dt.begin(), dt.end(), arrow) - dt.begin();
    if(imply_sign_pos == dt.size()) {
      cerr << "The rule template \"" << line << "\" does not contain =>, -> is corrupt :). Please correct." << endl;
      exit(122);
    }
	
    static vector<string> words(ls.size());
    words.resize(imply_sign_pos);
    // Add the predicate template
    for(int i=0 ; i<imply_sign_pos ; i++) {
      if(ls[i][0] == '$') {
	words[i] = variables[ls[i].substr(1)];
	if(words[i] == "") {
	  cerr << "The variable " << ls[i] << " is not defined in the file " << file << ". Please correct the problem." << endl;
	  exit(145);
	}
      }
      else
	words[i] = ls[i];	  
    }

    static string tmp;
    tmp = words[0];
    for(int i=1 ; i<words.size() ; i++) {
      tmp += " ";
      tmp += words[i];
    }
    string1D::iterator ni;
    int ptid;
    if((ni = find(PredicateTemplate::TemplateNames.begin(), PredicateTemplate::TemplateNames.end(), tmp)) == PredicateTemplate::TemplateNames.end()) {
      ptid = PredicateTemplate::TemplateNames.size();
      PredicateTemplate::Templates.push_back(PredicateTemplate (words));
      PredicateTemplate::TemplateNames.push_back(tmp);
    } else 
      ptid = ni - PredicateTemplate::TemplateNames.begin();

    words.clear();
    words.resize(ls.size()-imply_sign_pos-1);
    // And now for the truths
    for(int i=imply_sign_pos+1, j=0 ; i<ls.size() ; i++, j++) {
      if(ls[i][0] == '$') {
	words[j] = variables[ls[i].substr(1)];
	if(words[j] == "") {
	  cerr << "The variable " << ls[i] << " is not defined in the file " << file << ". Please correct the problem." << endl;
	  exit(145);
	}
      }
      else
	words[j] = ls[i];	  
    }
	
    static string tmp1;
    tmp1 = words[0];
    for(int i=1 ; i<words.size() ; i++) {
      tmp1 += " ";
      tmp1 += words[i];
    }

    int ttid;
    if((ni = find(TargetTemplate::TemplateNames.begin(), TargetTemplate::TemplateNames.end(), tmp1)) == TargetTemplate::TemplateNames.end()) {
      ttid = TargetTemplate::Templates.size();
      TargetTemplate::Templates.push_back(TargetTemplate (words));
      TargetTemplate::TemplateNames.push_back(tmp1);
    } else
      ttid = ni - TargetTemplate::TemplateNames.begin();

    tmp += " ";
    tmp += tmp1;
    RuleTemplate::AddTemplate(ptid, ttid, tmp);
  }

  for(int i=0 ; i<PredicateTemplate::Templates.size() ; i++) {
    PredicateTemplate::Templates[i].set_dependencies();
  }
  delete f;
}

// Generate all rules whose predicate is true on the given "sentence" (corpus), and correct
// at least one token from the truth.
// If generateBasedOnPredicate is true, then it will generate all rules whose predicate is
// true, regardless whether they correct tokens or not.
void RuleTemplate::instantiate(const wordType2D& corpus, int sample_ind, 
			       int pred_tid, rule_set& instances, 
			       bool generateBasedOnPredicate, 
			       bool forced_generation) {
  static wordType2DVector pred_insts;
  static wordType2DVector target_insts;

  ON_DEBUG(assert(pt_list.size()>pred_tid && pt_list[pred_tid].size()>0));

  pred_insts.clear();
  target_insts.clear();
  PredicateTemplate::Templates[pred_tid].instantiate(corpus, sample_ind, pred_insts);
  if(pred_insts.size() == 0)
    return;

  if(generateBasedOnPredicate) {
    for(int i=0 ; i<pred_insts.size() ; i++) {
      Predicate p(pred_tid, pred_insts[i]);
      rule_hash_const_iterator rlend = allRules.pend(p);
      for(rule_hash_const_iterator rl = allRules.pbegin(p) ; rl!=rlend ; ++rl)
	if((*rl).constraint_test(corpus[sample_ind]))
	  instances.insert(*rl);
    }
  }
  else {
    const int1D lst = pt_list[pred_tid];
    for(int1D::const_iterator ttid=lst.begin() ; ttid!=lst.end() ; ++ttid) {
      // If the current target template has all the fields already correct
      // then the rule will not get its goods increased for the current feature vector.
      if(!forced_generation && TargetTemplate::Templates[*ttid].bads(corpus[sample_ind])==0)
	continue;

      target_insts.clear();
      TargetTemplate::Templates[*ttid].instantiate(corpus[sample_ind], target_insts);

      for (int k=0 ; k<target_insts.size() ; k++) {
	if(Rule::Constraints.test(corpus[sample_ind], Target(*ttid, target_insts[k])))
	  for(int i=0 ; i<pred_insts.size() ; i++)
	    instances.insert(Rule(pred_tid, *ttid, pred_insts[i], target_insts[k]));
      }
    }
  }
}
