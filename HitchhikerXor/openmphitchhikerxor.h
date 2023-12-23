//
//  openmphitchhikerxor.h
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/7/11.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#ifndef openmphitchhikerxor_h
#define openmphitchhikerxor_h

#include <stdio.h>

void omp_setThreadNums(int threadNum);

void omp_hichhikerxor_matrix_encode(int k, int m, int w, int *matrix,
                                char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize);

int omp_hichhikerxor_matrix_decode(int k, int m, int w,
                                   int *matrix, int *erasures,
                                   char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize);

#endif /* openmphitchhikerxor_h */
