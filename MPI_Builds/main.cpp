#include <iostream>
#include <cstdlib>          // For rand()
#include <vector>

#include "mergesort.h"
#include "radix.h"
#include "bitonic.h"
#include "sample.h"

using std::cout, std::cerr, std::endl;
using std::vector;

// generates arry chunck
// args:
// total array length length
// number of processes
// what process is calling
// array order
//   1: sorted
//   2: reverse sorted
//   3: 1% scrambled
//   4: random
// returns:
// pointer to array start
vector<int> GenerateArrayChunck(int arrayLength, int numProcesses, int offset, int order) {
  int chunckLength = arrayLength/numProcesses;

  vector<int> chunck(chunckLength);
  
  switch(order) {
    // sorted array 
    case 1:
      for(int i = 0; i < chunckLength; i ++) {
        chunck.at(i) = i*(offset + 1);
      }
    break;

    // reverse sorted
    case 2:
      //TODO: implement
    break;

    // 1% scrambled
    case 3:
      for(int i = 0; i < chunckLength; i ++) {
        chunck.at(i) = i*(offset + 1);
        if (rand() % 100 == 0) {
          chunck.at(i) = rand();
        }
      }
    break;
    
    // random 
    case 4:
      for(int i = 0; i < chunckLength; i ++) {
        chunck.at(i) = rand();
      }
    break;
    
    // "error handling"
    default:
      cerr << "you fucked up telling array generator what to do" << endl;
  }

  return chunck;
}


void vectorTester() {
  for (int i = 0; i < 4; i++) {
    vector<int> currarr = GenerateArrayChunck(32, 4, i, 1);
    cout << "[";
    for (int j = 0; j < currarr.size(); j++) {  // Use size() instead of length()
      cout << currarr.at(j) << " ";  // Use j as the index here
    }
    cout << "]" << endl;
  }
}


int main() {
  std::cout << "Running main" << std::endl;
    
  mergesort();
  bitonic();
  radix();
  sample();

  //TODO: test array checker
    
  return 0;
}
