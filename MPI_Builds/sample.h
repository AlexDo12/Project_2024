#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>
using std::vector;

#define MASTER 0

void sample(vector<int>& data); // Declaration of the mergesort function
void test_sample(vector<int>& sorted, vector<int>& reverse, vector<int>& scrambled, vector<int>& random); // test function

#endif