# -*- Makefile -*-

# This is the Makefile for fnTBL 1.0 distribution.

# Uncomment and change the following line if you have multiple
# c++ distributions installed.

#CC=/usr/tools/bin/gcc

# This is the main g++ instalation directory - modify if necessary
#ifdef .PMAKE
MAINDIR != which $(CC) | sed 's%/bin/gcc%%; s%/bin/cc%%' 
#else
MAINDIR = $(shell which $(CC) | sed 's%/bin/cc%%; s%/bin/gcc%%')
#endif

CXX = ${MAINDIR}/bin/g++
CCC = ${CXX}

LIBPATH = -L${LIBDIR} -L${OBJDIR} -L.
INCPATH = -I${INCDIR} #-I${MAINDIR}/include/g++ -I${MAINDIR}/include

.EXPORT:
.EXPORT: server

TBL_TRAIN_SOURCES = ${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/timer.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/Predicate.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL-train.o 

TBL_SOURCES = ${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/Predicate.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL.o

CXXFLAGS = $(CXXOPT) $(CXXDEBUG) $(CXXWARNINGS) $(CXXSTUFF)

# math library
LDLIBS = -lm $(LDLIBS_ADDITIONAL) #-ltrie -lg -lc_p #-lstdc++ 

# optimizations for this architecture
ARCHOPTIM = #-D__USE_MALLOC

#See if we're using pmake
#ifdef .PMAKE
OPTIM_FLAG != perl ../exec/get_optim_flag.prl $(CCC)
#else
OPTIM_FLAG = $(shell perl ../exec/get_optim_flag.prl $(CCC))
#endif

# Uncomment the following line if the executable seems to crash
#OPTIM_FLAG = -O 

#for optimizing
#OPTIM = $(OPTIM_FLAG) -funroll-all-loops 
OPTIM = $(OPTIM_FLAG) -funroll-all-loops

# Trick to decide which version of STL you have installed - do not modify, please.

#See if we're using pmake
#ifdef .PMAKE
TYPE_TO_USE != perl ../exec/find_type.prl $(CCC)
#else
TYPE_TO_USE = $(shell perl ../exec/find_type.prl $(CCC))
#endif

OPTIONS = -D$(TYPE_TO_USE) -DFLOAT=float -DWORD_TYPE="unsigned int" -DPOSITION_TYPE="unsigned char"

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

default: all

all: clean fnTBL fnTBL-train adapt_perl_scripts

adapt_perl_scripts:
	perl ../exec/alter_perl_dir.pl `which perl` ../exec/
	perl ../exec/adapt.prl
	chmod a+x ../exec/*.prl
	-rm ../exec/*.old

clean:
	-${RM} *.o ../lib/*.a *~ ${OBJDIR}/*.o ${BINDIR}/*

sterile: clean
	-${RM} *.rpo
	-${RM} ${TARGETS}
	-${RM} ${TESTTARGETS}

distclean: sterile
	-${RM} makefile.optim

remake: clean depend
	${MAKE}

# figure out the dependencies

includes:
	cd ../include; make


# Here we construct a library containing all files obtained w/ genclass
${LIBDIR}/libtext.a: ${GNUOBJ}
	ar rcv $@ ${GNUOBJ}
	ranlib $@

# Our main targets

fnTBL: ${OBJDIR}/Predicate.o ${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/fnTBL ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/Predicate.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL.o $(LDLIBS)

fnTBL-train:	${INCDIR}/AtomicPredicate.h ${INCDIR}/Constraint.h ${INCDIR}/ContainsStringPredicate.h ${INCDIR}/CooccurrencePredicate.h ${INCDIR}/Dictionary.h ${INCDIR}/FeatureSequencePredicate.h ${INCDIR}/FeatureSetPredicate.h ${INCDIR}/Node.h ${INCDIR}/Params.h ${INCDIR}/Predicate.h ${INCDIR}/PrefixSuffixAddPredicate.h ${INCDIR}/PrefixSuffixIdentityPredicate.h ${INCDIR}/PrefixSuffixPredicate.h ${INCDIR}/PrefixSuffixRemovePredicate.h ${INCDIR}/Rule.h ${INCDIR}/SingleFeaturePredicate.h ${INCDIR}/SubwordPartPredicate.h ${INCDIR}/TBLTree.h ${INCDIR}/Target.h ${INCDIR}/common.h ${INCDIR}/index.h ${INCDIR}/indexed_map.h ${INCDIR}/io.h ${INCDIR}/line_splitter.h ${INCDIR}/sized_memory_pool.h ${INCDIR}/svector.h ${INCDIR}/timer.h ${INCDIR}/trie.h ${INCDIR}/typedef.h ${SRCDIR}/Constraint.cc ${SRCDIR}/ContainsStringPredicate.cc ${SRCDIR}/CooccurrencePredicate.cc ${SRCDIR}/Dictionary.cc ${SRCDIR}/Node.cc ${SRCDIR}/Params.cc ${SRCDIR}/Predicate.cc ${SRCDIR}/PrefixSuffixAddPredicate.cc ${SRCDIR}/Rule.cc ${SRCDIR}/SubwordPartPredicate.cc ${SRCDIR}/TBLTree.cc ${SRCDIR}/Target.cc ${SRCDIR}/common.cc ${SRCDIR}/index.cc ${SRCDIR}/io.cc ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/Predicate.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL-train.o
	 $(CCC) $(CCFLAGS) -o ${BINDIR}/fnTBL-train ${OBJDIR}/Constraint.o ${OBJDIR}/ContainsStringPredicate.o ${OBJDIR}/CooccurrencePredicate.o ${OBJDIR}/Dictionary.o ${OBJDIR}/Node.o ${OBJDIR}/Params.o ${OBJDIR}/Predicate.o ${OBJDIR}/PrefixSuffixAddPredicate.o ${OBJDIR}/Rule.o ${OBJDIR}/SubwordPartPredicate.o ${OBJDIR}/TBLTree.o ${OBJDIR}/Target.o ${OBJDIR}/common.o ${OBJDIR}/index.o ${OBJDIR}/io.o ${OBJDIR}/fnTBL-train.o $(LDLIBS)


printme:
	echo $(TYPE_TO_USE)

# Automatically generated dependencies
${OBJDIR}/Constraint.o: ../src/Constraint.cc ../include/Constraint.h \
 ../include/typedef.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/common.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Target.h ../include/Dictionary.h \
 ../include/indexed_map.h ../include/Params.h \
 ../include/line_splitter.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/mmemory ../include/Rule.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Constraint.o ${SRCDIR}/Constraint.cc
${OBJDIR}/ContainsStringPredicate.o: ../src/ContainsStringPredicate.cc \
 ../include/ContainsStringPredicate.h ../include/typedef.h \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/ContainsStringPredicate.o ${SRCDIR}/ContainsStringPredicate.cc
${OBJDIR}/CooccurrencePredicate.o: ../src/CooccurrencePredicate.cc \
 ../include/CooccurrencePredicate.h ../include/AtomicPredicate.h \
 ../include/typedef.h ../include/indexed_map.h ../include/common.h \
 ../include/Params.h ../include/line_splitter.h ../include/Predicate.h \
 ../include/Dictionary.h ../include/trie.h ../include/m_pair.h \
 ../include/my_bit_vector.h ../include/linear_map.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/CooccurrencePredicate.o ${SRCDIR}/CooccurrencePredicate.cc
${OBJDIR}/Dictionary.o: ../src/Dictionary.cc ../include/Dictionary.h \
 ../include/typedef.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Dictionary.o ${SRCDIR}/Dictionary.cc
${OBJDIR}/GetOpt.o: ../src/GetOpt.cc
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/GetOpt.o ${SRCDIR}/GetOpt.cc
${OBJDIR}/MemoryAllocator.o: ../src/MemoryAllocator.cc
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/MemoryAllocator.o ${SRCDIR}/MemoryAllocator.cc
${OBJDIR}/Node.o: ../src/Node.cc ../include/TBLTree.h ../include/Node.h \
 ../include/Rule.h ../include/typedef.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Node.o ${SRCDIR}/Node.cc
${OBJDIR}/Params.o: ../src/Params.cc ../include/Params.h ../include/common.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Params.o ${SRCDIR}/Params.cc
${OBJDIR}/Predicate.o: ../src/Predicate.cc ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/svector.h \
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
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Predicate.o ${SRCDIR}/Predicate.cc
${OBJDIR}/PrefixSuffixAddPredicate.o: ../src/PrefixSuffixAddPredicate.cc \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h ../include/AtomicPredicate.h \
 ../include/typedef.h ../include/Dictionary.h ../include/common.h \
 ../include/indexed_map.h ../include/trie.h ../include/m_pair.h \
 ../include/my_bit_vector.h ../include/linear_map.h \
 ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/PrefixSuffixAddPredicate.o ${SRCDIR}/PrefixSuffixAddPredicate.cc
${OBJDIR}/Rule.o: ../src/Rule.cc ../include/Rule.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Rule.o ${SRCDIR}/Rule.cc
${OBJDIR}/SubwordPartPredicate.o: ../src/SubwordPartPredicate.cc \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/SubwordPartPredicate.o ${SRCDIR}/SubwordPartPredicate.cc
${OBJDIR}/TBLTree.o: ../src/TBLTree.cc ../include/TBLTree.h ../include/Node.h \
 ../include/Rule.h ../include/typedef.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/TBLTree.o ${SRCDIR}/TBLTree.cc
${OBJDIR}/Target.o: ../src/Target.cc ../include/Target.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/common.h \
 ../include/Dictionary.h ../include/typedef.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Params.h \
 ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Target.o ${SRCDIR}/Target.cc
${OBJDIR}/Vocabulary.o: ../src/Vocabulary.cc ../include/Vocabulary.h \
 ../include/common.h ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/Vocabulary.o ${SRCDIR}/Vocabulary.cc
${OBJDIR}/buildTree.o: ../src/buildTree.cc ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/line_splitter.h \
 ../include/smart_open.h ../include/io.h ../include/Node.h \
 ../include/Rule.h ../include/Predicate.h ../include/AtomicPredicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory ../include/Constraint.h ../include/Target.h \
 ../include/Params.h ../include/PrefixSuffixPredicate.h \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/buildTree.o ${SRCDIR}/buildTree.cc
${OBJDIR}/bvector_test.o: ../src/bvector_test.cc ../include/my_bit_vector.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/bvector_test.o ${SRCDIR}/bvector_test.cc
${OBJDIR}/common.o: ../src/common.cc ../include/common.h ../include/Params.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/common.o ${SRCDIR}/common.cc
${OBJDIR}/fnTBL-train.o: ../src/fnTBL-train.cc ../include/typedef.h \
 ../include/TBLTree.h ../include/Node.h ../include/Rule.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h \
 ../include/io.h ../include/timer.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h \
 ../include/ContainsStringPredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/fnTBL-train.o ${SRCDIR}/fnTBL-train.cc
${OBJDIR}/fnTBL.o: ../src/fnTBL.cc ../include/typedef.h ../include/TBLTree.h \
 ../include/Node.h ../include/Rule.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h ../include/io.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/fnTBL.o ${SRCDIR}/fnTBL.cc
${OBJDIR}/index.o: ../src/index.cc ../include/index.h ../include/memory.h \
 ../include/typedef.h ../include/common.h ../include/indexed_map.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/index.o ${SRCDIR}/index.cc
${OBJDIR}/io.o: ../src/io.cc ../include/io.h ../include/typedef.h \
 ../include/line_splitter.h ../include/common.h ../include/index.h \
 ../include/memory.h ../include/indexed_map.h ../include/Params.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h \
 ../include/SubwordPartPredicate.h ../include/SingleFeaturePredicate.h \
 ../include/AtomicPredicate.h ../include/Dictionary.h \
 ../include/Predicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory ../include/Rule.h \
 ../include/Constraint.h ../include/Target.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h \
 ../include/ContainsStringPredicate.h \
 ../include/CooccurrencePredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/io.o ${SRCDIR}/io.cc
${OBJDIR}/learner.o: ../src/learner.cc ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Rule.h \
 ../include/Predicate.h ../include/AtomicPredicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory ../include/Constraint.h ../include/Target.h \
 ../include/Params.h ../include/line_splitter.h ../include/index.h \
 ../include/memory.h ../include/Node.h ../include/TBLTree.h \
 ../include/io.h ../include/timer.h \
 ../include/PrefixSuffixAddPredicate.h \
 ../include/PrefixSuffixPredicate.h ../include/SubwordPartPredicate.h \
 ../include/SingleFeaturePredicate.h \
 ../include/ContainsStringPredicate.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/learner.o ${SRCDIR}/learner.cc
${OBJDIR}/learner1.o: ../src/learner1.cc ../include/typedef.h \
 ../include/ruleTemplates.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Rule.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/learner1.o ${SRCDIR}/learner1.cc
${OBJDIR}/lin_map_test.o: ../src/lin_map_test.cc ../include/linear_map.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/lin_map_test.o ${SRCDIR}/lin_map_test.cc
${OBJDIR}/rule_hash_test.o: ../src/rule_hash_test.cc ../include/Rule.h \
 ../include/typedef.h ../include/Dictionary.h ../include/common.h \
 ../include/indexed_map.h ../include/trie.h ../include/m_pair.h \
 ../include/my_bit_vector.h ../include/linear_map.h \
 ../include/Predicate.h ../include/AtomicPredicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory ../include/Constraint.h ../include/Target.h \
 ../include/Params.h ../include/line_splitter.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/rule_hash_test.o ${SRCDIR}/rule_hash_test.cc
${OBJDIR}/set_test.o: ../src/set_test.cc
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/set_test.o ${SRCDIR}/set_test.cc
${OBJDIR}/simple-learner.o: ../src/simple-learner.cc ../include/typedef.h \
 ../include/ruleTemplates.h ../include/Dictionary.h \
 ../include/common.h ../include/indexed_map.h ../include/trie.h \
 ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Rule.h ../include/Predicate.h \
 ../include/AtomicPredicate.h ../include/svector.h \
 ../include/sized_memory_pool.h ../include/mmemory \
 ../include/Constraint.h ../include/Target.h ../include/Params.h \
 ../include/line_splitter.h ../include/index.h ../include/memory.h \
 ../include/timer.h ../include/io.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/simple-learner.o ${SRCDIR}/simple-learner.cc
${OBJDIR}/simple-tester.o: ../src/simple-tester.cc ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Rule.h \
 ../include/Predicate.h ../include/AtomicPredicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory ../include/Constraint.h ../include/Target.h \
 ../include/Params.h ../include/line_splitter.h ../include/index.h \
 ../include/memory.h ../include/io.h ../include/timer.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/simple-tester.o ${SRCDIR}/simple-tester.cc
${OBJDIR}/test.o: ../src/test.cc ../include/timer.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/test.o ${SRCDIR}/test.cc
${OBJDIR}/test1.o: ../src/test1.cc
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/test1.o ${SRCDIR}/test1.cc
${OBJDIR}/testTree.o: ../src/testTree.cc ../include/typedef.h \
 ../include/Dictionary.h ../include/common.h ../include/indexed_map.h \
 ../include/trie.h ../include/m_pair.h ../include/my_bit_vector.h \
 ../include/linear_map.h ../include/Node.h \
 ../include/Rule.h ../include/Predicate.h ../include/AtomicPredicate.h \
 ../include/svector.h ../include/sized_memory_pool.h \
 ../include/mmemory ../include/Constraint.h ../include/Target.h \
 ../include/Params.h ../include/line_splitter.h \
 ../include/smart_open.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/testTree.o ${SRCDIR}/testTree.cc
${OBJDIR}/test_index_map.o: ../src/test_index_map.cc ../include/index.h \
 ../include/memory.h ../include/typedef.h ../include/common.h \
 ../include/indexed_map.h ../include/dmalloc.h
		$(CCC) -c $(CCFLAGS) -o ${OBJDIR}/test_index_map.o ${SRCDIR}/test_index_map.cc
