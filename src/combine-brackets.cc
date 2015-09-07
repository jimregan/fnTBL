#include <vector>
#include <iostream>

#include "line_splitter.h"
#include "timer.h"
#include "Dictionary.h"
#include "common.h"

int open_bracket = 1;
int close_bracket = 2;
int no_bracket = 3;

int O_class = 0;
int I_class = 1;
int B_class = 2;
int E_class = 3;

using namespace std;


class sentence {
  typedef vector<string> string_vector;
  typedef vector<int> int_vector;
  typedef vector<double> double_vector;
  typedef pair<int,int> int_int_pair;

  int line_id;
  vector<double_vector> oprob;
  vector<double_vector> cprob;
  static double_vector length_probabilities;

public:
  static double no_bracket_initial_weight;

protected:
  static Dictionary classes;

  vector<string_vector> values;
public:
  sentence(int n=0): line_id(0), oprob(n), cprob(n) {}
  virtual ~sentence() {}
  
  double score(int i, int j) const {
    return oprob[i][open_bracket] * cprob[j][close_bracket] 
      * strictly_no_bracket_score(i+1,j-1) * (i==j ? 1 :  cprob[i][no_bracket] * oprob[j][no_bracket])
      * len_prob(j-i+1);
  }

  double len_prob(int len) const {
    if(length_probabilities.size()>0) 
      if(len < length_probabilities.size())
	return length_probabilities[len];
      else
	return length_probabilities.back();
    else
      return 1;
  }

  double no_bracket_score(int i, int j) const {
    // Computes the score associated with the event of not having a bracket
    // start after i (included) and end before j (included) - it's possible
    // to have brackets ending in i and/or starting in j
    double prob = 1;
//     if (i<j)
    prob = oprob[i][no_bracket] * cprob[j][no_bracket];
    for(int k=i+1 ; k<j ; k++)
      prob *= oprob[k][no_bracket] * cprob[k][no_bracket];

    return prob;
  }

  double strictly_no_bracket_score(int i, int j) const {
    // Computes the score associated with the event of not having a bracket
    // ending or starting in the interval i..j (including not end in i or begin in j)
    double prob = 1;
    for(int k=i ; k<=j ; k++)
      prob *= oprob[k][no_bracket] * cprob[k][no_bracket];
    return prob;
  }

  void set_shortest_chunk_sequence() {
    static int_vector shortest_sequence;
    static string out_classes[] = {"O","I","B","E"};

    open_bracket = classes.getIndex("[");
    close_bracket = classes.getIndex("]");
    no_bracket = classes.getIndex(".");
    shortest_sequence.resize(values.size());

    fill(shortest_sequence.begin(), shortest_sequence.end(), O_class);

    int last_open = -1;
    for(int i=0 ; i<values.size() ; i++) {
      if(oprob[i][open_bracket] >= no_bracket_initial_weight)
	last_open = i;
      
      if(cprob[i][close_bracket] >= no_bracket_initial_weight) {
	if(last_open != -1) {
	  shortest_sequence[last_open] = B_class;
	  if(last_open!=i)
	    shortest_sequence[i] = E_class;
	  for(int k = last_open+1 ; k<i ; k++)
	    shortest_sequence[k] = I_class;
	}
	last_open = -1;
      }
    }

    for(int i=0 ; i<values.size() ; i++)
      values[i][2] = out_classes[shortest_sequence[i]];
  }

  void set_best_sequence() {
    static int_vector best_sequence;
    static string out_classes[] = {"O","I","B","E"};
    static int_int_pair no_bracket_pair = make_pair(-1,-1);
    static map<int_int_pair,int_int_pair > indices;
    indices.clear();

    open_bracket = classes.getIndex("[");
    close_bracket = classes.getIndex("]");
    no_bracket = classes.getIndex(".");

    best_sequence.resize(values.size());
    fill(best_sequence.begin(), best_sequence.end(), O_class);

    double matrix[1000][1000]; // no point in resizing this matrix, if we want the algo to finish today
    int n = values.size();
    int i,j;
    for(i=1 ; i<n ; i++)
      matrix[i][i-1]=1;

    for(int len=1 ; len<=n ; ++len) {
      for(int first=0 ; first <= n-len ; first++) {
	int last = first+len-1;
	double max = strictly_no_bracket_score(first, last);
	pair<int, int> first_last = make_pair(first,last);
	indices[first_last] = no_bracket_pair;
	for(i=first ; i<=last ; i++) 
	  for(j=i ; j<=last ; j++) {
	    int_int_pair i_j = make_pair(i,j);
	    if((i>first || j<last) && indices[i_j] == no_bracket_pair)
	      continue;
	    double val = (i==0 ? 1 : matrix[first][i-1]) * score(i,j) * matrix[j+1][last];
	    if(val > max) {
	      max = val;
	      indices[first_last] = i_j;
	    }
	  }
	matrix[first][last] = max;
      }
    }

    // Now, recover the best sequence
    int first = 0;
    int last = n-1;
    static stack<int_int_pair> s;
    s.push(make_pair(first,last));

    while (s.size()>0) {
      int_int_pair current_t = s.top();
      s.pop();

      int_int_pair t = indices[current_t];
      if(t.first==-1 || current_t.first > current_t.second) // no bracket here
	continue; 
      if(best_sequence[t.first] != O_class)
	cerr << "Error at line " << line_id << ", indices (" << t.first << "," << t.second << ")" << endl;
      best_sequence[t.first] = B_class;

      if(t.first != t.second) {
	if(best_sequence[t.second] != O_class)
	  cerr << "Error at line " << line_id << ", indices (" << t.first << "," << t.second << ")" << endl;
	best_sequence[t.second] = E_class;
      }

      for(int k=t.first+1 ; k<t.second ; k++)
	best_sequence[k] = I_class;

      if(t.first > current_t.first && t.first>0)
	s.push(make_pair(current_t.first, t.first-1));
      if(t.second < current_t.second && t.second<n)
	s.push(make_pair(t.second+1, current_t.second));
    }
    
    for(i=0 ; i<n ; i++)
      values[i][2] = out_classes[best_sequence[i]];
  }

//   istream& operator << (istream& istr, sentence& s) {
    
//   }
  void clear() {
    values.clear();
    oprob.clear();
    cprob.clear();
  }

  void read(istream& open_str, istream& close_str) {
    line_id++;
    clear();
    static string line, line1;

    while(getline(open_str, line)) {
      getline(close_str, line1);
      if(line == "")
	break;
      read_values(line);
      read_values(line1, false);
    }
  }

private:
  
  void read_values(const string& line, bool read_open = true) {
    int p = line.find("|");
    static line_splitter ls;
    if (p==line.npos) {
      cerr << "The string " << line << " does not contain the prob. separator '|'!" << endl;
      exit(121);
    }
    ls.split(line.substr(0, p-1));
    static string_vector word;
    word.resize(ls.size());
    copy(ls.begin(), ls.end(), word.begin());

    if (read_open)
      values.push_back(word);
//     else
//       if(word != values.back()) {
// 	cerr << "The open and close files do not match !!" << endl << "Open: " << values.back() << endl
// 	     << "Close: " << word << endl;
// 	exit(122);
//       }

    ls.split(line.substr(p+1));
    static double_vector word_prob;
    word_prob.clear();
    double eps = 1e-5;
    double sum = 0;
    for (line_splitter::iterator j=ls.begin() ; j!=ls.end() ; j++) {
      int cls = classes.insert(*j);
      word_prob.resize(classes.size());
      ++j;
      static line_splitter bs("()");
      bs.split(*j);
      sum += (word_prob[cls] = atof1(bs[0])+eps);
    }

    transform(word_prob.begin(), word_prob.end(), word_prob.begin(),
	      bind2nd(divides<double>(), sum));

    if(read_open) 
      oprob.push_back(word_prob);
    else
      cprob.push_back(word_prob);
  }
  
public:

  friend ostream& operator << (ostream& ostr, sentence& s) {
    for (vector<string_vector>::iterator i=s.values.begin() ; i!=s.values.end() ; ++i) {
      for(string_vector::iterator j = i->begin() ; j!=i->end() ; ++j) 
	ostr << *j << " ";
      ostr << "\n";
    }
    return ostr << "\n";
  }

  static void read_lengths(const string& length_data_file) {
    ifstream h(length_data_file.c_str());
    line_splitter ls;
    string line;
    while (getline(h, line)) {
      ls.split(line);
      int n=atoi1(ls[0]);
      double v = atof1(ls[1]);
      length_probabilities.resize(n+1);
      length_probabilities[n] = v;
    }
    h.close();
  }
};

Dictionary sentence::classes;
sentence::double_vector sentence::length_probabilities;
double sentence::no_bracket_initial_weight = 0.5;

void usage(char * program_name) {
  cerr << "Usage: " << endl
       << " " << program_name << " [options] <open_bracket_file> <close_bracket_file>" << endl
       << " -shortest         = will identify the shortest NPs instead of the best sequence (default)" << endl
       << " -len_data <file>  = reads information about the length of base NPs from the specified file" << endl
       << endl;
}

int main (int argc, char* argv[]) {

  if(argc < 3) {
    usage(argv[0]);
    exit (2);
  }

  bool shortest_solution = false;
  int first_arg = 1;
  string length_data_file = "";

  for(int i=1 ; i<argc ; i++) {
    if(!strcmp("-shortest", argv[i])) {
      shortest_solution = true;
      first_arg ++;
    }
    else if(!strcmp("-len_data",argv[i])) {
      length_data_file = argv[++i];
      first_arg += 2;
    }
    else if(!strcmp("-no_bracket_weight", argv[i])) {
      sentence::no_bracket_initial_weight = atof1(argv[++i]);
      cerr << "No bracket weight: " << sentence::no_bracket_initial_weight << endl;
      first_arg += 2;
    }
  }

  if (length_data_file != "")
    sentence::read_lengths(length_data_file);

  ifstream f(argv[first_arg]), g(argv[first_arg+1]);
  sentence s;
  timer tm;
  int lineid=0;
  tm.mark();
  while (! f.eof()) {
    s.read(f,g);
    if(shortest_solution)
      s.set_shortest_chunk_sequence();
    else
      s.set_best_sequence();
    cout << s;
    if (++lineid%10 == 0) {
      tm.mark();
      cerr << "\r" << "                                           " << "\r"
	   << "Sentences per second: " << lineid/(tm.seconds_since_beginning()+1) 
	   << " (" << lineid << " sentences processed)";
    }
  }
  cerr << "\r" << "                                           " << "\r";
}


