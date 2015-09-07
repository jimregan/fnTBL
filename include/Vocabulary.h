// This is a -*- C++ -*- file

#ifndef _Template_Vocabulary_h
#define _Template_Vocabulary_h

#include <vector>
#include <hash_map.h>
#include "common.h"

class Vocabulary 
{
public:
  typedef hash_map<string, int> string_map;

  typedef vector<string *> string_vector;
  typedef string_map::iterator iterator;
  typedef string_map::const_iterator const_iterator;
protected:
  string_map wordMap;
  string_vector words;

  int size, first;
  mutable int wasUnknown;
  int unknownIndex;
  string spelling_of_unknown;
  
public:

  Vocabulary(int s = 0, int f=0);

  Vocabulary(const string filename);

  Vocabulary(Vocabulary& v):
	wordMap(v.wordMap), words(v.words), size(v.size),
	first(v.first), wasUnknown(0), unknownIndex(v.unknownIndex), 
	spelling_of_unknown(v.spelling_of_unknown)
	{
	}

  virtual ~Vocabulary () 
    {
    }

  void Add(const string& word);

  void Add(const char * wrd) {
	string s(wrd);
	Add(s);
  }

  const string& operator[](int i) const
    {
#ifdef DEBUG
      if(i < first || i >= size+first) 
	cerr << "Error at line 37 in Vocabulary.h";
#endif
      return * words[i-first];
    }

  int operator[](const char *str) const {
	static string s;
	s = str;
	return (*this)[s];
  }

  int operator[](const string& word) const {
	const_iterator i=wordMap.find(word);
	if(i == wordMap.end()) {
	  wasUnknown = 1;
	  return unknownIndex;
	}
	else {
	  wasUnknown = 0;
	  return (*i).second;
	}
  }

  int WasUnknown() const {
    return wasUnknown;
  }

  int Size() const {
    return size;
  }
  
  int FirstIndex() const {
    return first;
  }

  int LastIndex() const {
    return first+size-1;
  }

  const string& UnknownSpelling() const {
	return spelling_of_unknown;
  }

  int UnknownIndex() const{
	return unknownIndex;
  }

  void SaveToFile(const string fileName);

  static Vocabulary * vocab;
  static void Initialize(string s="");
  static void CreateNewVocabulary() {
	vocab = new Vocabulary();
  }

  static const Vocabulary& GetVocabulary() {
	if(vocab == NULL)
	  Initialize();
	return *vocab;
  }
  
  void static Clear() {
	delete vocab;
  }

  friend ostream& operator << (ostream&, const Vocabulary&);
};

#endif
