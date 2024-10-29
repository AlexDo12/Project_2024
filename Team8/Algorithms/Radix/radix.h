#ifndef RADIX_H
#define RADIX_H

int getBits(int num, int shift, int r);
void radix(std::vector<int>& local_data, int total_elements, int r, int max_value, MPI_Comm comm);
void testRadix(vector<int>& data);

#endif