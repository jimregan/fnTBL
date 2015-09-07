// -*- C++ -*-
/*
  Definitions for all the programs.
  This file is part of the fnTBL distribution.

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

#ifndef __TYPEDEF_H
#define __TYPEDEF_H
#define __STL_NO_NAMESPACES
#include <string>
#include <set>
#include <vector>

typedef int intType;
typedef int scoreType;   // the type of score for good/bad

// typedef unsigned int wordType;

// The type for the data
typedef WORD_TYPE wordType;

// The type for feature indices
typedef POSITION_TYPE featureIndexType;

typedef wordType* wordType1D;
typedef char relativePosType;

typedef std::vector<wordType> wordTypeVector;
typedef std::vector<wordType1D > wordType2D;
typedef std::vector<wordTypeVector> wordType2DVector;
typedef std::vector<wordType2D > wordType3D;
typedef std::vector<wordType2DVector> wordType3DVector;


// typedef unsigned short featureIndexType;
typedef std::vector<featureIndexType> featureIndexType1D;
typedef std::vector<featureIndexType1D > featureIndexType2D;

typedef std::vector<std::string> string1D;
typedef std::set<std::string> string_set;
typedef std::set<wordType> wordType_set;

typedef std::vector<int> int1D;
typedef std::vector<int1D> int2D;
typedef std::vector<int2D> int3D;

typedef std::vector<float> float1D;
typedef std::vector<float1D> float2D;

typedef std::vector<unsigned short int> shortint1D;

typedef short int * shortintPtr;

static const std::string UNK_string = "UNKNOWN";

static const float EPSILON = 1e-5;  // anything within this is the same

static const int THRESHOLDNUMRULES = 500;

static const int DEFAULTCOST = 1;

static const int WORD = 0;
static const int POS = 1;

#endif


