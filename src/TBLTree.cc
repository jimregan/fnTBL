/*
  Implements the probabilistic tree needed for the computations of
  the probabilistic behavior of TBL.

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

#include "TBLTree.h"
#include <queue>
#include <stack>
#include <string>
#include "common.h"
#include "line_splitter.h"

vector<Rule> TBLTree::rules;
TBLTree::rule_map_type TBLTree::rule_index;
extern wordType3D corpus;



TBLTree::TBLTree(const string& file) {
  readInTextFormat(file);
}

void TBLTree::readInTextFormat(const string& file) {
  istream* ts;
  int1D v1, v2;
  Node::addSimpleTemplates(v1, v2);
  smart_open(ts, file);
  *ts >> *this;
  delete ts;
}

void TBLTree::initialize(const vector<Rule>& tbl_rules) {
  rules = tbl_rules;
  
  for(int i=0 ; i<rules.size() ; ++i)
	rule_index[rules[i]] = i;
}

void TBLTree::gather_leaves() {
  frontier1.clear();
  stack<Node*> s;
  s.push(root);

  while (!s.empty()) {
	PNode p = s.top();
	s.pop();
	if(! p->yesChild)
	  frontier1.push_back(p);
	else {
	  s.push(p->noChild);
	  s.push(p->yesChild);
	}
  }
}

// ============================================================================
// The tree construction has 2 steps:
// 1. Separate the training examples based on the TBL rules
// 2. Further split the samples using simple TBL rules, that are equivalent to 
//    decision tree rules - SingleFeaturePredicate rules.
// ============================================================================

void TBLTree::construct_tree() {
  // First, assign all the samples to the root of the tree.
  for(unsigned int i=0 ; i<corpus.size() ; i++) {
	int num_words = corpus[i].size()-PredicateTemplate::MaxForwardLookup;
	for(unsigned short j=static_cast<unsigned short>(-PredicateTemplate::MaxBackwardLookup) ; j<num_words ; j++)
	  root->examples.push_back(make_pair(i, j));
  }

  // Then split them using the rules that apply to them
  root->split(0);
  gather_leaves();
  
  for(frontier_iterator nd=frontier_begin() ; nd!=frontier_end() ; ++nd) 
	(*nd)->growDT();
}

ostream& operator << (ostream& ostr, const TBLTree& tree) {
  ostr << "Number_of_rules " << tree.rules.size() << endl;
  ostr << "Classes:";
  Dictionary& dict = Dictionary::GetDictionary();
  for(int i=0 ; i<dict.num_classes ; i++)
	ostr << " " << dict[i] ;
  ostr << endl;

  for(int i=0 ; i<tree.rules.size() ; i++)
	ostr << tree.rules[i].printMe() << endl;

  Node* pNode;
  stack<Node*> q;
  q.push(tree.root);

  while (!q.empty()) {
	pNode = q.top();
	q.pop();
	ostr << *pNode;
	if (pNode->yesChild) {
	  ostr << " " << 2 << endl;
	  q.push(pNode->yesChild);
	  q.push(pNode->noChild);
	} else {
	  ostr << " " << 0 << endl;
	}
  }

  return ostr;
}

istream& operator >> (istream& istr, TBLTree& tree) {
  int no_rules;
  string str;
  string line;
  line_splitter ls;
  getline(istr, line);
  ls.split (line);
  no_rules = atoi1(ls[1]);
  tree.rules.resize(no_rules);

  Dictionary& dict = Dictionary::GetDictionary();
  getline(istr, line);
  ls.split(line);

  for(int i=1 ; i<ls.size() ; i++) {
	dict.insert(ls[i]);
// 	cerr << "Word: " << ls[i] << " index: " << dict[ls[i]] << endl;
  }

//   cerr << "Fixing the dictionary: " << dict.num_classes << endl;
  dict.FixClasses();
//   cerr << "Fixing the dictionary: " << dict.num_classes << endl;
  for(int i=0 ; i<no_rules ; i++) {
	getline(istr, line);
	ls.split(line);
	tree.rules[i] = Rule(ls.data());
  }

//   tree.root = new Node;
  stack<Node*> q;
//   delete tree.root;
  tree.root = new Node;
  q.push(tree.root);
  while (! q.empty()) {
	Node * pnode = q.top();
	int no_children;
	q.pop();

	if(!(istr >> *pnode >> no_children) ) {
	  cerr << "There is an error with the tree file - please check it and try again" << endl;
	  cerr << "Last tree read: " << *pnode << endl;
	  exit(11);
	}	

	if (no_children == 2) {
	  pnode->yesChild = new Node;
	  pnode->noChild = new Node;
	  q.push(pnode->yesChild);
	  q.push(pnode->noChild);
	} else
	  pnode->yesChild = pnode->noChild = 0;
  }

  return istr;
}

void TBLTree::readClasses(const string& file) {
  istream* istr;
  smart_open(istr, file);
  
  string line;
  line_splitter ls;
  getline(*istr, line);

  Dictionary& dict = Dictionary::GetDictionary();
  getline(*istr, line);
  ls.split(line);

  for(int i=1 ; i<ls.size() ; i++)
	dict.insert(ls[i]);

  dict.FixClasses();

  if (file != "-")
	delete istr;

  PredicateTemplate::PredicateTemplate_vector& templates = PredicateTemplate::Templates;
//   static unsigned short fake_rule_index = Dictionary::GetDictionary()["FAKE_CLASS"];
  static bool initialized = false;

  if(! initialized) {
	for(int i=0 ; i<PredicateTemplate::name_map.size()-1 ; ++i) {
	  static vector<string> v(1);
	  v[0] = PredicateTemplate::name_map[i];
	  PredicateTemplate pt(v);
	  templates.push_back(pt);
	  PredicateTemplate::TemplateNames.push_back(PredicateTemplate::name_map[i]+"_0");
	}
	initialized = true;
  }

}
