//
//  hichhikerxor.h
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/6/16.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#ifndef hichhikerxor_h
#define hichhikerxor_h

#include <stdio.h>

int *hichhikerxor_generate_encode_matrix(int k, int m, int w);

void hichhikerxor_matrix_encode(int k, int m, int w, int *matrix,
                            char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize);

int hichhikerxor_matrix_decode(int k, int m, int w,
                           int *matrix, int *erasures,
                           char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize);


//restore b[failedKIdx] by using other blocks and first coding blocks
// restore a[failedKIdx] by using data blocks of b, (failedKIdx)/(m-1) + 1 coding blocks of b and the revelant a blocks with startIdx and endIdx

int hichhikerxor_matrix_IOefficient_decoding(int k, int m, int w, int failedKIdx, int *dm_ids, int *matrix, int *decoding_vector, char **data_ptrs, char **coding_ptrs, int coupleSize);


#endif /* hichhikerxor_h */
