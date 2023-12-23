//
//  openmphitchhikerxor.c
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/7/11.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#include "openmphitchhikerxor.h"
#include "jerasure.h"
#include "galois.h"

//#include <omp.h>
#include <string.h>

static int ompThreadNum = 1; // Default is one

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

void omp_setThreadNums(int threadNum){
    if (threadNum <= 0) {
        return;
    }
    ompThreadNum = threadNum;
//    omp_set_num_threads(ompThreadNum);
}

void omp_thread_dotprod(int k, int w, int *matrix_row,
                        int *src_ids, int dest_id,
                        char **data_ptrs, char **coding_ptrs, int offset, int size){
    int init;
    char *dptr, *sptr;
    int i;
    
    init = 0;
    
    dptr = (dest_id < k) ? data_ptrs[dest_id] : coding_ptrs[dest_id-k] + offset;
    
    /* First copy or xor any data that does not need to be multiplied by a factor */
    
    for (i = 0; i < k; i++) {
        if (matrix_row[i] == 1) {
            if (src_ids == NULL) {
                sptr = data_ptrs[i] + offset;
            } else if (src_ids[i] < k) {
                sptr = data_ptrs[src_ids[i]] + offset;
            } else {
                sptr = coding_ptrs[src_ids[i]-k] + offset;
            }
            if (init == 0) {
                memcpy(dptr, sptr, size);
                init = 1;
            } else {
                galois_region_xor(sptr, dptr, dptr, size);
            }
        }
    }
    
    /* Now do the data that needs to be multiplied by a factor */
    
    for (i = 0; i < k; i++) {
        if (matrix_row[i] != 0 && matrix_row[i] != 1) {
            if (src_ids == NULL) {
                sptr = data_ptrs[i] + offset;
            } else if (src_ids[i] < k) {
                sptr = data_ptrs[src_ids[i]] + offset;
            } else {
                sptr = coding_ptrs[src_ids[i]-k] + offset;
            }
            switch (w) {
                case 8:  galois_w08_region_multiply(sptr, matrix_row[i], size, dptr, init); break;
                case 16: galois_w16_region_multiply(sptr, matrix_row[i], size, dptr, init); break;
                case 32: galois_w32_region_multiply(sptr, matrix_row[i], size, dptr, init); break;
            }
            init = 1;
        }
    }
}

void omp_thread_encode(int k, int m, int w, int *matrix,
                       int threadIdx, int threadJobSize,
                       char **data_ptrs, char **coding_ptrs){
    int i;
    int offset = threadIdx * threadJobSize;
    
//    jerasure_matrix_encode(k, m, w, matrix, data_ptrs, coding_ptrs, threadJobSize);
    for (i = 0; i < m; i++) {
        omp_thread_dotprod(k, w, matrix+(i*k), NULL, k+i, data_ptrs, coding_ptrs, offset, threadJobSize);
    }
}


void omp_hichhikerxor_matrix_encode(int k, int m, int w, int *matrix,
                                    char **data_ptrs, char **coding_ptrs, int coupleSize, int totalSize){
    int idx;
    int threadJobSize = totalSize / ompThreadNum;
    
    int mIdx;
    int startIdx, endIdx;
    int xorLen = k/(m-1);
    
    int kIdx;
    
    char *codePtr;
    char *dataPtr;

#pragma omp parallel for
    for (idx = 0; idx < ompThreadNum; ++idx) {
        omp_thread_encode(k, m, w, matrix, idx, threadJobSize,
                          data_ptrs, coding_ptrs);
    }
    
    threadJobSize = coupleSize / ompThreadNum;
    
    for (mIdx=1 ; mIdx < m; ++mIdx) {
        startIdx = (mIdx -1) * xorLen;
        if ((mIdx + 1 ) == m ) {
            endIdx = k - 1;
        }else{
            endIdx = startIdx + xorLen - 1;
        }
        
        codePtr = coding_ptrs[mIdx] + coupleSize;
        
        for (kIdx = startIdx; kIdx <= endIdx; ++kIdx) {
            dataPtr = data_ptrs[kIdx];
#pragma omp parallel for
            for (idx = 0; idx < ompThreadNum; ++idx) {
                int offset = idx * threadJobSize;
                galois_region_xor(dataPtr+ offset, codePtr + offset, codePtr + offset, threadJobSize);
            }
        }
    }
}

int omp_hichhikerxor_matrix_IOefficient_decoding(int k, int m, int w, int failedKIdx, int *dm_ids, int *matrix, int *decoding_vector, char **data_ptrs, char **coding_ptrs, int coupleSize){
    
    int idx;
    //int mIdx = failedKIdx/(m-1) + 1;find the parity contains the xor of failedKIdx
    int xorLen = k/(m-1);
    int mIdx = failedKIdx/xorLen+1;
    int startIdx, endIdx;
    
    char * dataPtr, *codingPtr;
    
    dataPtr = data_ptrs[failedKIdx] + coupleSize; // restore b[failedKIdx] first
    
    int init = 0;
    
    int threadIdx, threadJobSize;
    
    threadJobSize = coupleSize / ompThreadNum;
    
    
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
                
#pragma omp parallel for
                for (threadIdx = 0; threadIdx < ompThreadNum; ++threadIdx) {
                    int offset = threadIdx * threadJobSize;
                    galois_region_xor(dataPtr + offset, codingPtr + offset, dataPtr + offset, threadJobSize);
                }
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

#pragma omp parallel for
for (threadIdx = 0; threadIdx < ompThreadNum; ++threadIdx) {
    int offset = threadIdx * threadJobSize;
            switch (w) {
                case 8:  galois_w08_region_multiply(codingPtr + offset, *(decoding_vector + idx), threadJobSize, dataPtr + offset, init); break;
                case 16: galois_w16_region_multiply(codingPtr + offset, *(decoding_vector + idx), threadJobSize, dataPtr + offset, init); break;
                case 32: galois_w32_region_multiply(codingPtr + offset, *(decoding_vector + idx), threadJobSize, dataPtr + offset, init); break;
            }
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
#pragma omp parallel for
        for (threadIdx = 0; threadIdx < ompThreadNum; ++threadIdx) {
            int offset = threadIdx * threadJobSize;
            galois_region_xor(dataPtr + offset, codingPtr + offset, dataPtr + offset, threadJobSize);
        }
    }
    
    printf("\n");
    
    int *vector = matrix + mIdx*k;
    init = 1;  //ensure that the value of init is 1
    for (idx = 0; idx < k; ++idx) {
        codingPtr = data_ptrs[idx] + coupleSize;
        if (*(vector + idx) == 1) {
//            galois_region_xor(dataPtr, codingPtr, dataPtr, coupleSize);
#pragma omp parallel for
            for (threadIdx = 0; threadIdx < ompThreadNum; ++threadIdx) {
                int offset = threadIdx * threadJobSize;
                galois_region_xor(dataPtr + offset, codingPtr + offset, dataPtr + offset, threadJobSize);
            }

        }else if (*(vector + idx) !=0){
#pragma omp parallel for
            for (threadIdx = 0; threadIdx < ompThreadNum; ++threadIdx) {
                int offset = threadIdx * threadJobSize;
                switch (w) {
                    case 8:  galois_w08_region_multiply(codingPtr + offset, *(vector + idx), threadJobSize, dataPtr + offset, init); break;
                    case 16: galois_w16_region_multiply(codingPtr + offset, *(vector + idx), threadJobSize, dataPtr + offset, init); break;
                    case 32: galois_w32_region_multiply(codingPtr + offset, *(vector + idx), threadJobSize, dataPtr + offset, init); break;
                }
            }

//            switch (w) {
//                case 8:  galois_w08_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
//                case 16: galois_w16_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
//                case 32: galois_w32_region_multiply(codingPtr, *(vector + idx), coupleSize, dataPtr, init); break;
//            }
        }
    }
    
    return 0;
}

int omp_hichhikerxor_matrix_decode(int k, int m, int w,
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
        omp_hichhikerxor_matrix_IOefficient_decoding(k, m, w, erasures[0], dm_ids, matrix, decoding_matrix + k * erasures[0], data_ptrs, coding_ptrs, coupleSize);
    }else{
        
    }
    
    return 0;
}

