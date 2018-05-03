# Project Files
1. 1000.en - File containing 1000 source sentences
2. 1000.de - File containing 1000 translated sentences   
3. Serial-BLEU.cpp - Serial code
4. Distr-BLEU.cpp - MPI implementation 
5. Parallel-BLEU.cpp - OpenMP implementation

# Boost library
Our implementation requires boost library. Please download using https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz

# Serial Implementation
## compile
g++ -std=c++11 -I <path_to_boost> Serial-BLEU.cpp -o Serial-BLEU
## run
./Serial-BLEU 4 1000

# OpenMP Implementation
## compile
g++ -I <path_to_boost> -std=c++11 -fopenmp Parallel-BLEU.cpp -o Parallel-BLEU
## run 
mpirun.mpich2 -n 4 -hosts master,node001,node002,node003 ./Parallel-BLEU 4 1000

# MPI Implementation
## compile
mpicxx -I <path_to_boost> -std=c++11 Distr-BLEU.cpp -o Distr-BLEU
## run
./Distr-BLEU 4 1000

If you would like to test on a different dataset make sure that the source and target files have same name with .en suffix  for source file and .de for target file.
