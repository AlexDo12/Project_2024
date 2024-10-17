#ifndef BITONIC_H
#define BITONIC_H

#include <vector>
#include "mpi.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

using std::vector;

#define MASTER 0

void bitonic(vector<int>& random);
void compareExchange(std::vector<int>& local_array, int partner, int rank, int elements_per_process, bool ascending);

#endif