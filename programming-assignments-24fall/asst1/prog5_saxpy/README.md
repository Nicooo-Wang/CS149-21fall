## Program 5: BLAS `saxpy`

1. Compile and run `saxpy`. The program will report the performance of ISPC (without tasks) and ISPC (with tasks) implementations of saxpy. What speedup from using ISPC with tasks do you observe? Explain the performance of this program. Do you think it can be substantially improved? (For example, could you rewrite the code to achieve near linear speedup? Yes or No? Please justify your answer.)

   Answer : I only achive 1.44x speedup from use of tasks. Since this program requires many memory accesses, so it is limited by the bandwidth of memory. I think it can not be substantially improved.

2. **Extra Credit:** (1 point) Note that the total memory bandwidth consumed computation in `main.cpp` is `TOTAL_BYTES = 4 * N * sizeof(float);`. Even though `saxpy` loads one element from X, one element from Y, and writes one element to `result` the multiplier by 4 is correct. Why is this the case? (Hint, think about how CPU caches work.)

   Answer : When the program writes to one element of `result`, it will first fetch the cache line which contains this element into the cache. This requires one memory operation. Then when this cache line is not needed, it will be flashed out of the cache, this requires another memory operation. 
