# CSCE 435 Group project

## 0. Group number: 8
we will be communicating using an iMessage group chat. This has been created already and all members have responded

## 1. Group members:
1. Alex Do
2. Alex Byrd
3. Jose Rojo
4. Matthew Livesay

## 2. Project topic (e.g., parallel sorting algorithms)
Examining the performance of parallelized sorting algorithms

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort: Alex Do
- Sample Sort: Alex Byrd
- Merge Sort: Jose Rojo
- Radix Sort: Matthew Livesay

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

#### Radix Sort

```
// master process
generate array

for each chunck of digits (likely single digit in base 10 but gains may be had processing 2-3 digits at a time)

    split array and send to workers

    receive histograms from workers
    combine histograms from all workers (might be a reduce like call to acomplish this)

    calculate prefix sum (iterates over histogram array which will be short)

    build output array 
    copy output array to origional array

confirm array is sorted


// worker process
for each chunk of digits
    receive array chunk
    calculate histogram of the chunck 
    send histogram to master process
```
parameters will need to be tweaked for radix sort. increasing the chunck size would reduce the number of times the master thread has to iterate over the entire array, at the cost of increased memory usage and inter-process communication. There also might be a way to add some parallelism to the building and copying of the output array 

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types  
    - input array lengths of 128, 1024, 8192, and 131072
    - all inputs will be arrays of integers to allow comparitson with radix sort
- Strong scaling (same problem size, increase number of processors/nodes)
    - strong scaling will be evaluated by running problem size of ..., ..., and ... on 2, 4, 8, 16, 32, and 64 processes 
- Weak scaling (increase problem size, increase number of processors)
    - 128 on 2 processes
    - 8192 on 16 processes
    - 131072 on 32 processes 
    - 1048576 on 64 processes
