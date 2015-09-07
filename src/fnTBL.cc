/*
  The testing program for fnTBL.

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

#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>

#include "hash_wrapper.h"

#include <stdlib.h>
#include <sys/timeb.h>

#include "typedef.h"
#include "TBLTree.h"
#include "Dictionary.h"
#include "Rule.h"
#include "line_splitter.h"
#include "Params.h"
#include "index.h"
#include "trie.h"
#include "PrefixSuffixPredicate.h"
#include "io.h"
#include "Node.h"
#include "timer.h"

typedef trie<char, bool> word_trie;

typedef word_index<unsigned int, unsigned short> word_index_class;
typedef vector<Rule> rule_vector;

bool setState = false;
wordType UNK;
int corpus_size;
int V_flag = 0;

wordType3D corpus;

wordType3DVector ruleTrace;
rule_vector allRules;
bool v_flag;
bool p_flag;
bool soft_probabilities;
bool printRT = false;
bool printErrors = false;
int1D errors, new_errors;
string output_file = "";
ostream* out;

// By default, run the program in line mode, not entire corpus mode
bool non_sequential = false;

float general_error = 0.0;

word_index_class corpusIndex;
word_index_class classifIndex(1);
word_index_class defaultIndex(2);

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

// this function reads in the rules.
void readInRules(char *fileName)
{
  istream *in;
  smart_open(in, fileName);
  char in_line[1024];

  while(in->getline(in_line, 1024, '\n')) {
    if(in_line[0] == '#')
      continue;

    string line(in_line);
    string::size_type pos;
    if((pos = line.find("RULE: ")) != line.npos)
      line.erase(0, pos+6);

    line_splitter splitLine;
    splitLine.split(line);

    allRules.push_back(Rule(splitLine.data()));
  }

  delete in;
}

// This runs a rule on the corpus.
void runOneRule (const Rule &currRule, int ruleID)
{
  int i=currRule.get_least_frequent_feature_position();
  bool unindexable_rule = false;
  
  if(i==-1) {
    unindexable_rule = true;
    i = 0;
  }

  wordType least_frequent = currRule.predicate.tokens[i];
  
  static AtomicPredicate::storage_vector features;
  features.clear();
  PredicateTemplate::Templates[currRule.predicate.template_id][i].get_feature_ids(features);
  
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
    PredicateTemplate::Templates[currRule.predicate.template_id][i].get_sample_differences(offsets);
  
  static int 
    STATE_START = TargetTemplate::STATE_START,
    TRUTH_START = TargetTemplate::TRUTH_START;
  
  int currRuleTtid = currRule.target.tid;
  static wordTypeVector curr_rule_target(TargetTemplate::TRUTH_SIZE);
  const TargetTemplate::pos_vector& curr_rule_positions = TargetTemplate::Templates[currRuleTtid].positions;
  
  fill(curr_rule_target.begin(), curr_rule_target.end(), 0);
  int t=0;
  for(TargetTemplate::pos_vector::const_iterator itt=curr_rule_positions.begin() ;
      itt != curr_rule_positions.end();
      ++itt, ++t) 
    curr_rule_target[*itt] = currRule.target.vals[t];

  static vector<pair<unsigned int, unsigned short> > changedPlaces;
  changedPlaces.clear();
  
  Dictionary& dict = Dictionary::GetDictionary();
  
  word_index_class::iterator endp = thisIndex.end(least_frequent);

  for(word_index_class::iterator it = thisIndex.begin(least_frequent); it != endp ; ++it) {
    unsigned int i = (*it).line_id();
    for(AtomicPredicate::position_vector::iterator offset = offsets.begin() ; offset != offsets.end() ; ++offset) {
      unsigned short int j = (*it).word_id() - *offset;
      if (j>=-PredicateTemplate::MaxBackwardLookup && j<corpus[i].size()-PredicateTemplate::MaxForwardLookup)
	if(currRule.test(corpus[i], j))
	  changedPlaces.push_back(make_pair(i, j));
    }
  }

  static wordType fake_rule_index = Dictionary::GetDictionary()["FAKE_CLASS"];
  for (vector<pair<unsigned,unsigned short> >::iterator thisPosition = changedPlaces.begin();
       thisPosition != changedPlaces.end(); ++thisPosition) {
    int pos=0;
    for(TargetTemplate::pos_vector::const_iterator itt = curr_rule_positions.begin() ;
	itt != curr_rule_positions.end() ; ++itt, ++pos) {
      wordType
	old_classif = corpus[thisPosition->first][thisPosition->second][STATE_START + *itt],
	new_classif = currRule.target.vals[pos],
	true_classif = corpus[thisPosition->first][thisPosition->second][TRUTH_START + *itt];

      if(new_classif != fake_rule_index) { // If the target is FAKE_CLASS => do nothing.	
	if(V_flag >= 3)
	  cerr << "Removing " << dict[old_classif] << " from the classifIndex" << endl;
	classifIndex.erase(old_classif, thisPosition->first, thisPosition->second);
	
	if(printErrors)
	  if(non_sequential)
	    general_error -= 
	      TargetTemplate::value_is_correct(new_classif,true_classif) -
	      TargetTemplate::value_is_correct(old_classif,true_classif);
	  else
	    new_errors[ruleID+1] -=
	      TargetTemplate::value_is_correct(new_classif,true_classif) -
	      TargetTemplate::value_is_correct(old_classif,true_classif);			
	
// 	if(!p_flag)
	corpus[thisPosition->first][thisPosition->second][STATE_START + *itt] = new_classif;
	
	if(V_flag >= 3)
	  cerr << "Adding " << dict[new_classif] << " into the classifIndex" << endl;
	classifIndex.insert(new_classif, thisPosition->first, thisPosition->second);
      }
      
      ruleTrace[thisPosition->first][thisPosition->second].push_back(ruleID);
    }
  }
}

void printRuleTrace(void)
{
  static int feature_set_size = RuleTemplate::name_map.size();
  Dictionary& dict = Dictionary::GetDictionary();
  bool empty_line_are_seps = Params::GetParams()["EMPTY_LINES_ARE_SEPARATORS"] == "1";
  int TRUTH_SIZE = TargetTemplate::TRUTH_SIZE,
    TRUTH_START = TargetTemplate::TRUTH_START,
    STATE_START = TargetTemplate::STATE_START;

  for (int i = 0; i < static_cast<int>(ruleTrace.size()); i++) {
    int maxind = static_cast<int>(corpus[i].size()) - PredicateTemplate::MaxForwardLookup;
    for (int j = -PredicateTemplate::MaxBackwardLookup ; j < maxind ; j++) {
      wordType1D& vect = corpus[i][j];
      for(int k=0 ; k<feature_set_size-2*TRUTH_SIZE ; k++)
	*out << dict[vect[k]] << " ";
      for(int k=STATE_START ; k<STATE_START+TRUTH_SIZE ; k++)
	*out << dict[vect[k]] << " ";
      for(int k=TRUTH_START ; k<TRUTH_START+TRUTH_SIZE ; k++)
	*out << dict[vect[k]] << " ";
      *out << "| ";

      copy(ruleTrace[i][j].begin(), ruleTrace[i][j].end(), ostream_iterator<int>(*out, " "));
      *out << endl;
    }
    if(empty_line_are_seps) // Only if samples are not independent
      *out << endl;
  }
}

void printPosition(unsigned int i, unsigned short j) {
  static int feature_set_size = RuleTemplate::name_map.size();
  Dictionary& dict = Dictionary::GetDictionary();
  int TRUTH_SIZE = TargetTemplate::TRUTH_SIZE,
    TRUTH_START = TargetTemplate::TRUTH_START,
    STATE_START = TargetTemplate::STATE_START;
  
  wordType1D& vect = corpus[i][j];
  for(int k=0 ; k<feature_set_size-2*TRUTH_SIZE ; k++)
    *out << dict[vect[k]] << " ";
  for(int k=STATE_START ; k<STATE_START+TRUTH_SIZE ; k++)
    *out << dict[vect[k]] << " ";
  for(int k=TRUTH_START ; k<TRUTH_START+TRUTH_SIZE-1 ; k++)
    *out << dict[vect[k]] << " ";
  *out << dict[vect[TRUTH_START+TRUTH_SIZE-1]];
}

void computeProbs(TBLTree& t) {
  Node::example_index p;
  bool empty_line_are_seps = Params::GetParams()["EMPTY_LINES_ARE_SEPARATORS"] == "1";

  for(int i=0 ; i<corpus.size() ; i++) {
    int maxind = static_cast<int>(corpus[i].size()) - PredicateTemplate::MaxForwardLookup;
    p.first = i;
    for(int j=-PredicateTemplate::MaxBackwardLookup ; j<maxind ; j++) {
      p.second = j;
      const Node* nd = t.findClassOfSample(p);

      printPosition(i, j);
      *out << " | " << nd->probString() << endl;
    }
    if(empty_line_are_seps) // Only if samples are not independent
      *out << endl;
  }
}

void computeSoftProbs(TBLTree& t) {
  
}

// The program should be called as:
//   fnTBL <sample_file> <rule_file> [-F <param file>] [-v] 
// Arguments:
//  - -F <param file> = uses the specified parameter file instead of the one in $DDINF
//  - -v              = will output various debugging information
//  - -p              = outputs probability with each resulting sample. -t should be defined.
//  - -t <tree file>  = defines the TBL tree file (generated by the learner program - see documentation)
//  - -soft_prob      = uses the "soft probability" model - see documentation

void usage(const char* prog_name) {
  cerr << prog_name << " <examples_file> <rule_list> [-F <params_file>] [-vp] [-t <tree_file>] [-soft_probs] [-printRuleTrace]" << endl
       << endl << "where:" << endl
       << " -F <params_file>    - sets the parameter file (otherwise choses the one specified in the environmental variable $DDINF)" << endl
       << " -v                  - turns on various debugging statements" << endl
       << " -soft_probs         - uses the \"soft\" probability model to assign probabilities - see documentation" << endl
       << " -t <tree_prob>      - specifies the TBL tree file (this parameter is mandatory if probabilistic TBL is used)" << endl
       << " -p                  - assign probabilities to the samples' classification, rather than just the classification" << endl
       << " -printRuleTrace     - for each sample, prints the indices of the rules that applied to it" << endl
       << " -batchSize <n>      - processes samples/sentences in batches of size n (default 100)" << endl
       << " -o <file>           - will output the result in the specified file (default stdout)" << endl
       << " -nonsequential      - will read the entire file in, and then start to process it" << endl
       << endl;
}

int main(int argc, char *argv[]) {
  Dictionary& dict = Dictionary::GetDictionary();
  string tree_file = "";
  string error_file = "";
  int batch_size = 10000;
  bool generate_tree = false;
  timer tm;
  tm.mark();

  if(argc < 3) {
    usage(argv[0]);
    exit(1);	
  }

  for(int i=3 ; i<argc ; i++)
    if(!strcmp("-printRuleTrace",argv[i])) {
      printRT = true;
    } else if(!strcmp("-F", argv[i])) {
      Params::Initialize(argv[++i]);
    } else if(!strcmp("-v", argv[i])) {
      v_flag = true;
    } else if(!strcmp("-p", argv[i])) {
      p_flag = true;
      non_sequential = true;
    } else if(!strcmp("-soft_probs", argv[i])) {
      p_flag = true;
      soft_probabilities = true;
    } else if(!strcmp("-t", argv[i])) {
      tree_file = argv[++i];
    } else if(!strcmp("-printErrors", argv[i])) {
      error_file = argv[++i];
      printErrors = true;
    } else if(!strcmp("-nonsequential", argv[i])) {
      non_sequential = true;
    } else if(!strcmp("-batchSize", argv[i])) {
      batch_size = atoi1(argv[++i]);
    } else if(!strcmp("-o", argv[i])) {
      output_file = argv[++i];
    } else if(!strcmp("-V",argv[i])) {
      V_flag = atoi1(argv[++i]);
    } else if(!strcmp("-generateProbTree", argv[i])) {
      generate_tree = true;
      tree_file = argv[++i];
      non_sequential = true;
    } else {
      cerr << "Unknown flag: " << argv[i] << endl;
      exit(1);
    }

  log_me_in(argc, argv);

  if(p_flag && tree_file == "") {
    cerr << "The TBL tree file is undefined. Please use -t <tree_file> to define it!" << endl;
    usage(argv[0]);
    exit(1);
  }

  if(output_file != "")
    smart_open(out, output_file);
  else
    out = &cout;
  RuleTemplate::Initialize();
  UNK = dict.getIndex(UNK_string);
  
  TBLTree t;

  if(p_flag)
    t.readClasses(tree_file);

  istream* rlsstr;
  smart_open(rlsstr, argv[2]);
  string line;
  getline(*rlsstr, line);
  delete rlsstr;
  line_splitter ls;
  ls.split(line);
  if (ls[0]!="#train_voc_file:") {
    cerr << "The rule file looks corrupted, because there is no mentioning of the training vocabulary." << endl << 
      " Please provide a valid rule file." << endl;
    exit(1);
  }

  string file_name = argv[1];
  bool is_stdin = false;

  if (file_name == "-") {
    is_stdin = true;
    timeb * tp = new timeb;
    ftime(tp);
    srand(tp->millitm);
    // 	int i = rand();
    string tmp_name = "/tmp/temp", new_file;
    ofstream tmpfile;
    do {
      tmpfile.close();
      int i = rand();
      new_file = tmp_name + itoa(i);
      tmpfile.open(new_file.c_str());
    } while (!tmpfile);
    file_name = new_file;

    ofstream of(file_name.c_str());
    while (getline(cin, line)) 
      of << line << endl;
    of.close();
  }

  studyData(file_name.c_str(), ls[1]);
  Rule::Initialize();
  if(non_sequential)
    readInData (const_cast<char*>(file_name.c_str()));

  cerr << "Reading rules" << endl;
  if (p_flag) {
    t.readInTextFormat(tree_file);
    allRules = t.rules;
  }
  else 
    readInRules (argv[2]);

  cerr << "Done reading rules" << endl;

  set<int> filter;
  for(rule_vector::iterator rl = allRules.begin() ; 
      rl!=allRules.end() ;
      ++rl) {
    filter.insert(rl->predicate.tokens[rl->predicate.order[0]]);
  }

  ostream *errstr;
  if(printErrors)
    smart_open(errstr, error_file);

  tm.mark();
  int initial_time = tm.seconds_since_last_mark();
  
  if(non_sequential) {
    generate_index(filter);
	
    ruleTrace.resize(corpus_size);
    for(int i=0 ; i<corpus_size ; i++) 
      for(int j=0 ; j<corpus[i].size() ; j++)
	ruleTrace[i].resize(corpus[i].size());
	
    ticker tk("Rules applied:", 8);
    int ruleID = 0;
	
    if(printErrors) {
      general_error = 0.0;
      for(int i=0 ; i<corpus.size() ; i++) {
	wordType2D & vect = corpus[i];
	int max_pos = vect.size() - PredicateTemplate::MaxForwardLookup;
	for(int j=-PredicateTemplate::MaxBackwardLookup ; j<max_pos ; j++) 
	  for(int k=0 ; k<TargetTemplate::TRUTH_SIZE ; k++)
	    general_error += ! TargetTemplate::value_is_correct(vect[j][TargetTemplate::STATE_START+k],
							      vect[j][TargetTemplate::TRUTH_START+k]);
      }
    }
	
    if(p_flag) {
      for (rule_vector::iterator thisRule = TBLTree::rules.begin(); 
	   thisRule != TBLTree::rules.end(); ++thisRule) {
	runOneRule(*thisRule, ruleID++);
	if(printErrors)
	  *errstr << general_error << endl;
	tk.tick();
      }
      tk.clear();
	  
      if(! soft_probabilities) {
	computeProbs(t);
      } else {
	computeSoftProbs(t);
      }
    } else {
      for (rule_vector::iterator thisRule = allRules.begin(); 
	   thisRule != allRules.end(); ++thisRule) {
	runOneRule(*thisRule, ruleID++);
	if(printErrors)
	  *errstr << general_error << endl;
	tk.tick();
      }
      tk.clear();
	  
      if(generate_tree) {
	TBLTree t;
	t.initialize(allRules);
	t.construct_tree();
	ostream *tree_out_file;
	smart_open(tree_out_file, tree_file);
	*tree_out_file << t;
	delete tree_out_file;
      } else 
	printCorpusState(*out, printRT);
    }
  } 
  else {						// We are processing the sentences 
    // in batches.
    istream* filestr;
    smart_open(filestr, file_name.c_str());

    ticker tk("Processed sentences:", 32);
    corpus.resize(batch_size);
    int feature_set_size = RuleTemplate::name_map.size();
    for(int i=0 ; i<corpus.size() ; i++){
      corpus[i].resize(2000);
      wordType1D p = corpus[i][0] = new wordType [200*feature_set_size];

      for(int j=1 ; j<corpus[i].size() ; j++)
	corpus[i][j] = p + feature_set_size*j;
    }

    if(printErrors) {
      errors.resize(allRules.size()+1);
      new_errors.resize(allRules.size()+1);
    }

    ruleTrace.resize(batch_size);
    int no_lines = 0;
    while (read_lines(*filestr, batch_size)) {
      generate_index(filter);

      fill(new_errors.begin(), new_errors.end(), 0);

      if(printErrors)
	for(int i=0 ; i<corpus.size() ; ++i) {
	  wordType2D & vect = corpus[i];
	  int max_pos = vect.size() - PredicateTemplate::MaxForwardLookup;
	  for(int j=0 ; j<max_pos ; ++j) {
	    for(int k=0 ; k<TargetTemplate::TRUTH_SIZE ; k++)
	      new_errors[0] += TargetTemplate::value_is_correct(vect[j][TargetTemplate::STATE_START+k],
								vect[j][TargetTemplate::TRUTH_START+k]);
	  }
	}
	  
      int ruleID = 0;
      for (rule_vector::iterator thisRule = allRules.begin(); 
	   thisRule != allRules.end(); ++thisRule) {
	if(printErrors)
	  new_errors[ruleID+1] = new_errors[ruleID];
	runOneRule(*thisRule, ruleID++);
      }
      no_lines += batch_size;
      tk.tick(no_lines, true);
      printCorpusState(*out, printRT);
      corpusIndex.clear();
      classifIndex.clear();
      for(int k=0 ; k<corpus.size() ; k++)
	for(int i=0 ; i<corpus[k].size() ; i++) {
	  ruleTrace[k][i].clear();
	}

      for(int i=0 ; i<errors.size() ; i++)
	errors[i] += new_errors[i];
    }
    tk.clear();
    cerr << "Done." << endl;
    delete filestr;

    if(printErrors)
      for(int i=0 ; i<errors.size() ; i++)
	*errstr << errors[i] << endl;
  }

  if(printErrors)
    delete errstr;

  if(output_file != "" && output_file != "-")
    delete out;
  
  tm.mark();
  if(v_flag > 0) {
    cerr << "Time spent during initialization: " << initial_time << " seconds." << endl;
    cerr << "Sentences processed per second: " << 1.0*corpus_size/tm.seconds_since_last_mark() << "." << endl;
  }

  if(is_stdin) {
    if(v_flag)
      cerr << "Removing the temporary file " << file_name << endl;
    execlp("/bin/rm", "rm", file_name.c_str(), (char *) NULL);
    // execvp("/bin/rm", const_cast<char **>(args));
  }
  cerr << "Superdone" << endl;
}
