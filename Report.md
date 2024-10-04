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
//master thread
creates array to be sorted
initializes MPI with p processes

for each chunck of bits (ie bits 0-7, 8-15, ...) 
    split array into p chunks
    send each part of array to the coresponding process

    receives offsets and values from worker threads 
    assembles the array

confirms returned array is sorted


//worker thread 
for each chunck of bits (ie bits 0-7, 8-15, ...)
    receive array
    computes histogram
    sends histogram data to processes that need it

    receives required histogram data 
    computes offset for each value (uses formula in AMD parellel radix paper)
    sends offsets and values to master process

```

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
