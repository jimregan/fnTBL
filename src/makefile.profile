# -*- Makefile -*-

#MAINDIR=/remote/bigram/usr
#MAINDIR=/remote/bigram/usr
#MAINDIR=/usr/tools
#MAINDIR=/people/CS/rflorian/gcc

#ifdef CLSP
#MAINDIR=/people/CS/rflorian/gcc
.MAKEFLAGS: -J 10 -L 2 -k
.EXPORT:
.EXPORT: i686
.SHELL: name=tcsh
#else
.MAKEFLAGS: -J 10 -L 0 -k
.EXPORT:
.EXPORT: server
#MAINDIR=/remote/telugu/usr/local
MAINDIR=/usr
#endif
#MAINDIR=/usr/local2

CC = "ccmalloc ${MAINDIR}/bin/gcc"
CXX = ${MAINDIR}/bin/g++
#CXX = /home/rflorian/insure/bin.linux2/insure
CCC = ${CXX}
LIBPATH = -L${LIBDIR} -L${OBJDIR} -L.
INCPATH = -I${INCDIR} -I${MAINDIR}/include/g++ -I${MAINDIR}/include # -I/usr/include
#-I/remote/bigram/usr/include/g++-3 #-I/remote/bigram/usr/tools/include/g++ -I/remote/bigram/usr/tools/include -I../src -I/remote/bigram/usr/local/include/g++

# math library
LDLIBS = -lm -static $(LDLIBS_ADDITIONAL) #/usr/local/lib/ccmalloc-g++_P.o -lccmalloc -ldl #-ltrie -lg -lc_p #-lstdc++ 

# optimizations for this architecture
ARCHOPTIM = -march=pentiumpro #-D__USE_MALLOC

#for optimizing
#OPTIM = -g -DDEBUG=3 #-D__USE_MALLOC
#LDLIBS_ADDITIONAL=-ldmalloc

#for fast_debugging
#OPTIM = -g -DDEBUG=3

# for seeing warnings and debugging
#OPTIM = -O3 -funroll-all-loops #-malign-double  #-mstack-align-double #-marg-align-double -funroll-all-loops -ffast-math

# for profiling
OPTIM = -g -pg -O2 -a #-fprofile-arcs -ftest-coverage


# this is required for old SunOS
OPTIONS = -DUSE_TYPE1 -DFLOAT=float -DWORD_TYPE="unsigned int" -DPOSITION_TYPE="unsigned short" #-D__USE_MALLOC #-fno-implicit-templates -mcpu=pentiumpro -fguiding-decls #-fexceptions -frtti -D__USE_FIXED_PROTOTYPES__ 

WARNS = -Wall -Wno-sign-compare

CXXFLAGS = -ftemplate-depth-23 -fstrict-aliasing ${OPTIM} ${ARCHOPTIM} ${OPTIONS} ${WARNS} ${LIBPATH} ${INCPATH} #-frepo 
CCFLAGS  = ${CXXFLAGS}
CFLAGS   = $(CCFLAGS)
LD = ${CXX} ${CXXFLAGS}

# 2 lines for sunOS
CCC = ${CXX}

BINDIR = ../bin
INCDIR = ../include
SRCDIR = ../src
LIBDIR = ${MAINDIR}/lib
TESTDIR = ../test
OBJDIR = ../obj

TARGETS = ${BINDIR}/process \
${BINDIR}/ReadVocab
# ${BINDIR}/chartlessparse \
# ${BINDIR}/bbchartlessparse \
# ${BINDIR}/bbtopdownchartparse

TESTTARGETS = ${TESTDIR}/readtest ${TESTDIR}/readtest


all: includes ${TARGETS}

clean:
	-${RM} *_P.o ../lib/*.a *~ ${OBJDIR}/*_P.o

sterile: clean
	-${RM} *.rpo
	-${RM} ${TARGETS}
	-${RM} ${TESTTARGETS}

distclean: sterile
	-${RM} makefile.profile

remake: clean depend
	${MAKE}

# figure out the dependencies

depend:
	grep -v '^makefile.profile' Make.depend.profile > makefile.profile
	echo "" >> makefile.profile
	echo "# Automatically generated dependencies" >> makefile.profile
	echo "" >> makefile.profile
	${CXX} ${INCPATH} -MM ${SRCDIR}/*.cc | \/usr\/tools\/bin/add_makeline _P -N   >> makefile.profile


tests: includes ${TESTTARGETS}

includes:
	cd ../include; make


# Here we construct a library containing all files obtained w/ genclass
${LIBDIR}/libtext.a: ${GNUOBJ}
	ar rcv $@ ${GNUOBJ}
	ranlib $@

# Our main targets

trie-test:	${INCDIR}/debug.h ${INCDIR}/line_splitter.h ${INCDIR}/linear_map.h ${INCDIR}/m_pair.h ${INCDIR}/my_bit_vector.h ${INCDIR}/trie.h ${OBJDIR}/trie-test_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/trie-test_P ${OBJDIR}/trie-test_P.o $(LDLIBS)

test1:	 ${OBJDIR}/test1_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/test1_P ${OBJDIR}/test1_P.o $(LDLIBS)


test:	${INCDIR}/reversible_array.h ${INCDIR}/timer.h ${OBJDIR}/test_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/test_P ${OBJDIR}/test_P.o $(LDLIBS)

test_index_map:	${INCDIR}/index.h ${INCDIR}/indexed_map.h ${OBJDIR}/test_index_map_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/test_index_map_P ${OBJDIR}/test_index_map_P.o $(LDLIBS)

rule_hash_test:	${INCDIR}/AtomicPredicate.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/common.h ${INCDIR}/indexed_map.h ${INCDIR}/line_splitter.h ${INCDIR}/svector.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/common.cc ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/common_P.o ${OBJDIR}/rule_hash_test_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/rule_hash_test_P ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/common_P.o ${OBJDIR}/rule_hash_test_P.o $(LDLIBS)

set_test:	 ${OBJDIR}/set_test_P.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/set_test_P ${OBJDIR}/set_test_P.o $(LDLIBS)

fnTBL:	${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint_P.o ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/CooccurrencePredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Node_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/TBLTree_P.o ${OBJDIR}/Target_P.o ${OBJDIR}/common_P.o ${OBJDIR}/index_P.o ${OBJDIR}/io_P.o ${OBJDIR}/fnTBL_P.o
	$(CXX) $(CCFLAGS) -o ${BINDIR}/fnTBL_P ${OBJDIR}/Constraint_P.o ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/CooccurrencePredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Node_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/TBLTree_P.o ${OBJDIR}/Target_P.o ${OBJDIR}/common_P.o ${OBJDIR}/index_P.o ${OBJDIR}/io_P.o ${OBJDIR}/fnTBL_P.o $(LDLIBS)

fnTBL-train:	${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/timer.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint_P.o ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/CooccurrencePredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Node_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/TBLTree_P.o ${OBJDIR}/Target_P.o ${OBJDIR}/common_P.o ${OBJDIR}/index_P.o ${OBJDIR}/io_P.o ${OBJDIR}/fnTBL-train_P.o
	 $(CXX) $(CCFLAGS) -o ${BINDIR}/fnTBL-train_P ${OBJDIR}/Constraint_P.o ${OBJDIR}/ContainsStringPredicate_P.o ${OBJDIR}/CooccurrencePredicate_P.o ${OBJDIR}/Dictionary_P.o ${OBJDIR}/Node_P.o ${OBJDIR}/Params_P.o ${OBJDIR}/Predicate_P.o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${OBJDIR}/Rule_P.o ${OBJDIR}/SubwordPartPredicate_P.o ${OBJDIR}/TBLTree_P.o ${OBJDIR}/Target_P.o ${OBJDIR}/common_P.o ${OBJDIR}/index_P.o ${OBJDIR}/io_P.o ${OBJDIR}/fnTBL-train_P.o $(LDLIBS)

progs: fnTBL fnTBL-train

# Automatically generated dependencies

${OBJDIR}/Constraint_P.o: ../src/Constraint.cc ../include/Constraint.h \
 ../include/typedef.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/debug.h ../include/common.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Target.h ../include/Dictionary.h \
 ../include/indexed_map.h ../include/Params.h \
 ../include/line_splitter.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/mmemory ../include/Rule.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Constraint_P.o ${SRCDIR}/Constraint.cc
${OBJDIR}/ContainsStringPredicate_P.o: ../src/ContainsStringPredicate.cc \
 ../include/ContainsStringPredicate.h ../include/typedef.h \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h ../include/debug.h \
 ../include/linear_map.h ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/ContainsStringPredicate_P.o ${SRCDIR}/ContainsStringPredicate.cc
${OBJDIR}/CooccurrencePredicate_P.o: ../src/CooccurrencePredicate.cc \
 ../include/CooccurrencePredicate.h ../include/AtomicPredicate.h \
 ../include/typedef.h ../include/indexed_map.h ../include/common.h \
 ../include/Params.h ../include/line_splitter.h ../include/Predicate.h \
 ../include/Dictionary.h ../include/trie.h ../include/m_pair.h \
 ../include/my_bit_vector.h ../include/debug.h ../include/linear_map.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/CooccurrencePredicate_P.o ${SRCDIR}/CooccurrencePredicate.cc
${OBJDIR}/Dictionary_P.o: ../src/Dictionary.cc ../include/Dictionary.h \
 ../include/typedef.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Dictionary_P.o ${SRCDIR}/Dictionary.cc
${OBJDIR}/GetOpt_P.o: ../src/GetOpt.cc
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/GetOpt_P.o ${SRCDIR}/GetOpt.cc
${OBJDIR}/Node_P.o: ../src/Node.cc ../include/TBLTree.h ../include/Node.h \
 ../include/Rule.h ../include/typedef.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h ../include/debug.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Node_P.o ${SRCDIR}/Node.cc
${OBJDIR}/Params_P.o: ../src/Params.cc ../include/Params.h ../include/common.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Params_P.o ${SRCDIR}/Params.cc
${OBJDIR}/Predicate_P.o: ../src/Predicate.cc ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Params.h \
 ../include/line_splitter.h ../include/SingleFeaturePredicate.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/PrefixSuffixRemovePredicate.h \
 ../include/PrefixSuffixIdentityPredicate.h \
 ../include/ContainsStringPredicate.h \
 ../include/FeatureSequencePredicate.h ../include/Rule.h \
 ../include/Constraint.h ../include/Target.h \
 ../include/CooccurrencePredicate.h ../include/FeatureSetPredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Predicate_P.o ${SRCDIR}/Predicate.cc
${OBJDIR}/PrefixSuffixAddPredicate_P.o: ../src/PrefixSuffixAddPredicate.cc \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h ../include/AtomicPredicate.h \
 ../include/typedef.h ../include/Dictionary.h ../include/common.h \
 ../include/indexed_map.h ../include/trie.h ../include/m_pair.h \
 ../include/my_bit_vector.h ../include/debug.h ../include/linear_map.h \
 ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/PrefixSuffixAddPredicate_P.o ${SRCDIR}/PrefixSuffixAddPredicate.cc
${OBJDIR}/Rule_P.o: ../src/Rule.cc ../include/Rule.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Rule_P.o ${SRCDIR}/Rule.cc
${OBJDIR}/SubwordPartPredicate_P.o: ../src/SubwordPartPredicate.cc \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h ../include/Predicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/SubwordPartPredicate_P.o ${SRCDIR}/SubwordPartPredicate.cc
${OBJDIR}/TBLTree_P.o: ../src/TBLTree.cc ../include/TBLTree.h ../include/Node.h \
 ../include/Rule.h ../include/typedef.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h ../include/debug.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/TBLTree_P.o ${SRCDIR}/TBLTree.cc
${OBJDIR}/Target_P.o: ../src/Target.cc ../include/Target.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/debug.h ../include/common.h \
 ../include/Dictionary.h ../include/typedef.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Target_P.o ${SRCDIR}/Target.cc
${OBJDIR}/bvector_test_P.o: ../src/bvector_test.cc ../include/my_bit_vector.h \
 ../include/debug.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/bvector_test_P.o ${SRCDIR}/bvector_test.cc
${OBJDIR}/common_P.o: ../src/common.cc ../include/common.h ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/common_P.o ${SRCDIR}/common.cc
${OBJDIR}/fnTBL-train_P.o: ../src/fnTBL-train.cc ../include/typedef.h \
 ../include/TBLTree.h ../include/Node.h ../include/Rule.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h \
 ../include/io.h ../include/timer.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h \
 ../include/ContainsStringPredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/fnTBL-train_P.o ${SRCDIR}/fnTBL-train.cc
${OBJDIR}/fnTBL_P.o: ../src/fnTBL.cc ../include/typedef.h ../include/TBLTree.h \
 ../include/Node.h ../include/Rule.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h ../include/debug.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h ../include/io.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/fnTBL_P.o ${SRCDIR}/fnTBL.cc
${OBJDIR}/index_P.o: ../src/index.cc ../include/index.h ../include/memory.h \
 ../include/typedef.h ../include/common.h ../include/indexed_map.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/index_P.o ${SRCDIR}/index.cc
${OBJDIR}/io_P.o: ../src/io.cc ../include/io.h ../include/typedef.h \
 ../include/line_splitter.h ../include/common.h ../include/index.h \
 ../include/memory.h ../include/indexed_map.h ../include/Params.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/debug.h ../include/linear_map.h \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/Dictionary.h \
 ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Rule.h \
 ../include/Constraint.h ../include/Target.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h \
 ../include/ContainsStringPredicate.h \
 ../include/CooccurrencePredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/io_P.o ${SRCDIR}/io.cc
