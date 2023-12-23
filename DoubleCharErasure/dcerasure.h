//
//  dcerasure.h
//  DoubleCharErasure
//
//  Created by Lau SZ on 2023/8/1.
//  Copyright © 2023 Shenzhen Technology University. All rights reserved.
//

#ifndef dcerasure_h
#define dcerasure_h

#include <stdio.h>

void dccreate_galois_w8_mult_table(void);

void dcerasure_matrix_encode(int k, int m, int w, int *matrix,
                            char **data_ptrs, char **coding_ptrs, int size);

int dcerasure_matrix_decode(int k, int m, int w,
                           int *matrix, int row_k_ones, int *erasures,
                           char **data_ptrs, char **coding_ptrs, int size);

int *create_matrix_w8(int k, int m);

#endif /* dcerasure_h */
