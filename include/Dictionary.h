// -*- c++ -*-
/*
  Creates the interface for a vocabulary class - indexes words in and out.

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

#ifndef __DICTIONARY_H
#define __DICTIONARY_H

#include <vector>
#include "hash_wrapper.h"
#include <string>

#include "typedef.h"
#include "common.h"
#include "indexed_map.h"
#include "trie.h"

class Dictionary {
public:
  typedef indexed_map<std::string, wordType> word_index_type;
  typedef word_index_type::iterator iterator;
  typedef word_index_type::const_iterator const_iterator;
  typedef trie<char, bool> word_trie;
  
  Dictionary(void): unknown_index(-1), spelling_of_unknown("UNK") {
  }

  ~Dictionary() {}

  const std::string& getString(wordType index) const;

  wordType reverse_access(const std::string& word) const {
    return getIndex(word);
  }

  const std::string& direct_access(wordType index) {
    return getString(index);
  }
  
  wordType operator [] (const std::string& word) {
    return getIndex(word);
  }

  wordType operator [] (const std::string& word) const {
    return getIndex(word);
  }

  const std::string& operator [] (wordType index) const {
    return getString(index);
  }

  wordType getIndex(const std::string& word);
  wordType getIndex(const std::string& word) const;

  wordType increaseCount(const std::string& word, unsigned int count = 1);
  wordType increaseCount(int index, unsigned int count = 1);

  const_iterator find(const std::string& word) const {
    return word_index.find(word);
  }

  iterator find(const std::string& word) {
    return word_index.find(word);
  }

  bool wasUnknown() const {
    return was_unknown;
  }

  int unknownIndex() const {
    return unknown_index;
  }
  
  const std::string& unknownSpelling() const {
    return spelling_of_unknown;
  }

  int getCounts(wordType thisString);

  const int1D& getCounts() {
    return word_counts;
  }

  int size() const {
    return word_index.size();
  }

  static Dictionary& GetDictionary() {
    static Dictionary dict;

    return dict;
  }

  static int num_classes;

  static void FixClasses() {
    Dictionary& dict = GetDictionary();
    if(num_classes == 0)
      num_classes = dict.size()+1;
  }

  static bool is_classfication(wordType word) {
    return word < num_classes;
  }

  wordType insert(const std::string& word) {
    wordType ind = word_index.insert(word);
    if(ind >= word_counts.size()) {
      word_counts.resize(ind+1);
      word_counts[ind] = 0;
    }
    return ind;
  }

  iterator begin() {
    return word_index.begin();
  }

  const_iterator begin() const {
    return word_index.begin();
  }

  iterator end() {
    return word_index.end();
  }

  const_iterator end() const {
    return word_index.end();
  }

  void insert_in_direct_trie(const std::string & w) {
    static std::vector<char> k;
    k.resize(w.size());
    copy(w.begin(), w.end(), k.begin());
    direct_trie[k] = true;
  }

  void insert_in_reverse_trie(const std::string& w) {
    static std::vector<char> k;
    k.resize(w.size());
    copy(w.rbegin(), w.rend(), k.begin());
    reverse_trie[k] = true;
  }

  word_trie& get_direct_trie() {
    return direct_trie;
  }

  word_trie& get_reverse_trie() {
    return reverse_trie;
  }

  void writeToFile(const std::string& file) const ;
  void readFromFile(const std::string& file);

  void set_start() {
    _real_word_start_index = size();
  }

  void set_end() {
    _real_word_end_index = size();
  }
  
  wordType real_word_start_index() const {
    return _real_word_start_index;
  }

  wordType real_word_end_index() const {
    return _real_word_end_index;
  }

  void destroy() {
    int1D tmp1;
    word_counts.swap(tmp1);
    word_index.destroy();
    direct_trie.destroy();
    reverse_trie.destroy();
  }

private:
  int1D word_counts;
  word_index_type word_index;
  mutable bool was_unknown;
  int unknown_index;
  std::string spelling_of_unknown;
  word_trie direct_trie, reverse_trie;
  wordType _real_word_start_index, _real_word_end_index;
};

#endif
