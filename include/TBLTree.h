// -*- C++ -*-
/*
  Implements the probabilistic tree needed for computing the probabilities
  associated with the rules.

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

#ifndef __TBLTree_h__
#define __TBLTree_h__

#include "Node.h"
#include "Rule.h"
#include <vector>
#include "hash_wrapper.h"

class TBLTree {
public:
  typedef Node* PNode;
  typedef TBLTree self;
  typedef std::vector<PNode> pnode_vector;
  typedef pnode_vector::iterator frontier_iterator;
  typedef pnode_vector::const_iterator frontier_const_iterator;
  typedef HASH_NAMESPACE::hash_map<Rule,int> rule_map_type;
  typedef Node::example_index example_index;
  
  TBLTree() {
    root = new Node;
  }

  TBLTree(const string& file);

  void readInTextFormat(const string& file);
  void readClasses(const string& file);

  frontier_iterator frontier_begin() {
    return frontier1.begin();
  }

  frontier_iterator frontier_end() {
    return frontier1.end();
  }

  frontier_const_iterator frontier_begin() const {
    return frontier1.begin();
  }

  frontier_const_iterator frontier_end() const {
    return frontier1.end();
  }

  void move_to_next_level() {
    frontier1.swap(frontier2);
    frontier2.clear();
  }

  void push_node_in_frontier(Node* p) {
    frontier2.push_back(p);
  }
  
  const Node* findClassOfSample(const example_index& example) const { 
    return root->findClassOfSample(example);
  }

  void updateCounts() {
    root->updateCounts();
  }

  void computeProbs(const example_index& example, const float2D& hypothesis) {
    root->computeProbs(example, hypothesis);
  }

  void initialize(const vector<Rule>& rules);

  const float1D& probs() const {
    return root->probs;
  }

  void construct_tree();
  void gather_leaves();

  friend ostream& operator << (ostream& ostr, const self&);
  friend istream& operator >> (istream& istr, self&);

  static int GetRuleIndex(const Rule& rule) {
    rule_map_type::iterator i = rule_index.find(rule);
    int index;
    if(i==rule_index.end()) {
      index = rule_index[rule] = rules.size();
      rules.push_back(rule);
    } else 
      index = i->second;

    return index;
  }

protected:
  Node * root;
  pnode_vector frontier1, frontier2;

public:
  static vector<Rule> rules;
  static rule_map_type rule_index;
};

#endif
