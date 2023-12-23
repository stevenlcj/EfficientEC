//
//  hichhikerxor.c
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/6/16.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#include "hitchhikerxor.h"

#include "jerasure.h"
#include "galois.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

int *hichhikerxor_generate_encode_matrix(int k, int m, int w){
    int *matrix = talloc(int, m*k);
    int i,j;

    if (matrix == NULL) {
        printf("unable to allocate memory for matrix\n");
        return NULL;
    }
    
    for (i = 0; i < m; i++) {
        for (j = 0; j < k; j++) {
            matrix[i*k+j] = galois_single_divide(1, i ^ (m + j), w);
        }
    }
    
    return matrix;
}


void hichhikerxor_matrix_encode(int k, int m, int w, int *matrix,
                                char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize){
    int mIdx;
    int startIdx, endIdx;
    int xorLen = k/(m-1);

    int kIdx;
    
    char *codePtr;
    char *dataPtr;

    jerasure_matrix_encode(k, m, w, matrix, data_ptrs, coding_ptrs, totalSize);
    
    for (mIdx=1 ; mIdx < m; ++mIdx) {
        startIdx = (mIdx -1) * xorLen;
        if ((mIdx + 1 ) == m ) {
            endIdx = k - 1;
        }else{
            endIdx = startIdx + xorLen - 1;
        }

//        printf("startIdx:%d, endIdx:%d\n", startIdx, endIdx);

        codePtr = coding_ptrs[mIdx] + coupleSize;
        
        for (kIdx = startIdx; kIdx <= endIdx; ++kIdx) {
//            printf("mIdx:%d, kIdx:%d\n", mIdx, kIdx);

            dataPtr = data_ptrs[kIdx];
            galois_region_xor(dataPtr, codePtr, codePtr, coupleSize);
        }
    }
}

//Assume totalSize = 2 * coupleSize
int hichhikerxor_matrix_decode(int k, int m, int w,
                           int *matrix, int *erasures,
                           char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize){
    int *dm_ids = talloc(int, k);

    int i, edd, lastdrive;
    int *erased, *decoding_matrix;
    
    if (w != 8 && w != 16 && w != 32) return -1;
    
    erased = jerasure_erasures_to_erased(k, m, erasures);
    if (erased == NULL) return -1;
    
    /* Find the number of data drives failed */
    
    lastdrive = k;
    
    edd = 0;
    for (i = 0; i < k; i++) {
        if (erased[i]) {
            edd++;
            lastdrive = i;
        }
    }
    
    
    if ( erased[k]) lastdrive = k;
    
    decoding_matrix = NULL;
    
    if (edd > 0 ) {
        if (dm_ids == NULL) {
            free(erased);
            return -1;
        }
        
        decoding_matrix = talloc(int, k*k);
        if (decoding_matrix == NULL) {
            free(erased);
            free(dm_ids);
            return -1;
        }
        
        if (jerasure_make_decoding_matrix(k, m, w, matrix, erased, decoding_matrix, dm_ids) < 0) {
            free(erased);
            free(dm_ids);
            free(decoding_matrix);
            return -1;
        }
    }
    
    if (decoding_matrix == NULL) {
        printf("unable to create decoding matrix");
        return -1;
    }
    
    //Only one missing data block and no missing coding block
    if (dm_ids[k-1] == k) {
        //The situation where only one data block missing
        hichhikerxor_matrix_IOefficient_decoding(k, m, w, erasures[0], dm_ids, matrix, decoding_matrix + k * erasures[0], data_ptrs, coding_ptrs, coupleSize);
    }else{
        
    }

    return 0;
}


//
int hichhikerxor_matrix_IOefficient_decoding(int k, int m, int w, int failedKIdx, int *dm_ids, int *matrix, int *decoding_vector, char **data_ptrs, char **coding_ptrs, int coupleSize){
    
    int idx;
    //int mIdx = failedKIdx/(m-1) + 1;find the parity contains the xor of failedKIdx
    int xorLen = k/(m-1);
    int mIdx = failedKIdx/xorLen+1;
    int startIdx, endIdx;

    char * dataPtr, *codingPtr;
    
    dataPtr = data_ptrs[failedKIdx] + coupleSize; // restore b[failedKIdx] first
    
    int init = 0;
    
    for (idx = 0 ; idx < k; ++idx) {
        if (*(decoding_vector+idx)  == 1) {
            if (dm_ids[idx] < k) {
                codingPtr = data_ptrs[dm_ids[idx]] + coupleSize;
            }else{
                codingPtr = coding_ptrs[dm_ids[idx] - k] + coupleSize;
            }
            
            if (init == 0) {
                memcpy(dataPtr, codingPtr, coupleSize);
                init = 1;
            }else{
                galois_region_xor(dataPtr, codingPtr, dataPtr, coupleSize);
            }
        }
    }
    
    for (idx = 0; idx < k; ++idx) {
        if (*(decoding_vector + idx) !=0 &&  *(decoding_vector + idx) !=1) {
            if (dm_ids[idx] < k) {
                codingPtr = data_ptrs[dm_ids[idx]] + coupleSize;
            }else{
                codingPtr = coding_ptrs[dm_ids[idx] - k] + coupleSize;
            }
            switch (w) {
                case 8:  galois_w08_region_multiply(codingPtr, *(decoding_vector + idx), coupleSize, dataPtr, init); break;
                case 16: galois_w16_region_multiply(codingPtr, *(decoding_vector + idx), coupleSize, dataPtr, init); break;
                case 32: galois_w32_region_multiply(codingPtr, *(decoding_vector + idx), coupleSize, dataPtr, init); break;
            }
            init = 1;
        }
    }
    
    dataPtr = data_ptrs[failedKIdx];
    
    //the case of the last data block missing
    if (mIdx >= m) {
        mIdx = m - 1;
    }

    startIdx = (mIdx -1) * xorLen;
    if ((mIdx + 1 ) == m ) {
        endIdx = k - 1;
    }else{
        endIdx = startIdx + xorLen - 1;
    }
    
    codingPtr = coding_ptrs[mIdx]+coupleSize;
    
    printf("mIdx:%d",mIdx);
    memcpy(dataPtr, codingPtr, coupleSize);
    for (idx = startIdx; idx <= endIdx; ++idx) {
        if (idx == failedKIdx) {
            continue;
        }
        printf(" idx:%d", idx);
        codingPtr = data_ptrs[idx];
        galois_region_xor(dataPtr, codingPtr, dataPtr, coupleSize);
    }
    
    printf("\n");
    
    int *vector = matrix + mIdx*k;
    init = 1;  //ensure that the value of init is 1
    for (idx = 0; idx < k; ++idx) {
        codingPtr = data_ptrs[idx] + coupleSize;
        if (*(vector + idx) == 1) {
            galois_region_xor(dataPtr, codingPtr, dataPtr, coupleSize);
        }else if (*(vector + idx) !=0){
            switch (w) {
                case 8:  galois_w08_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
                case 16: galois_w16_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
                case 32: galois_w32_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
            }
        }
    }

    return 0;
}
