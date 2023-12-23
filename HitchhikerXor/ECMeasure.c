//
//  ECMeasure.c
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/6/23.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#include "ECMeasure.h"
#include "hitchhikerxor.h"

#include "openmphitchhikerxor.h"

#include "jerasure.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <math.h>

#define talloc(type, size) (type *) malloc(sizeof(type) * (size));

void generateRandomBlock(char *block, int size){
    int idx;

    for (idx = 0; idx < size; ++idx) {
        *(block + idx) = rand() % 255;
    }
}

void hitchhikerXOROneFailedMeasure(int k, int m, int w, int coupleSize, int size , int threadNumForOMP){
    int *matrix = hichhikerxor_generate_encode_matrix(k, m, w);

    char **data_ptrs = talloc(char *, k);
    char **code_ptrs = talloc(char *, m);
    
    char *dataBlocks = talloc(char , (k+1)*size);
    char *codeBlocks = talloc(char, m*size);
    
    
    char **ompData_ptrs = talloc(char *, k);
    char **ompCode_ptrs = talloc(char *, m);

    char *ompDataBlocks = talloc(char , (k+1)*size);
    char *ompCodeBlocks = talloc(char, m*size);
    
    int idx;
    
    struct timeval startTime, endTime;
    double timeElapsedForHitchhikerXOREncode,timeElapsedForHitchhikerXORDecode,
    timeElapsedForOMPEncode, timeElapsedForOMPDecode,
    timeElapsedForJerasureEncode,  timeElapsedForJerasureDecode;
    
    generateRandomBlock(dataBlocks, k*size);
    omp_setThreadNums(threadNumForOMP);
    
    for (idx = 0; idx < k; ++idx) {
        data_ptrs[idx] = (dataBlocks + idx * size);
        ompData_ptrs[idx] = (ompDataBlocks + idx * size);
    }
    
    for (idx = 0; idx < m; ++idx) {
        code_ptrs[idx] = (codeBlocks + idx * size);
        ompCode_ptrs[idx] = (ompCodeBlocks + idx * size);
    }
    
    gettimeofday(&startTime, NULL);
    hichhikerxor_matrix_encode(k, m, w, matrix, data_ptrs, code_ptrs, coupleSize, size);
    gettimeofday(&endTime, NULL);
    
    timeElapsedForHitchhikerXOREncode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
    ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;
    
    int failedK = rand() % k;
    int *erasures = talloc(int, m+1);
    erasures[0] = failedK;
    erasures[1] = -1;

    memcpy(dataBlocks+k * size, data_ptrs[failedK], size);
    memset(data_ptrs[failedK], 0, size);
    
    //jerasure_matrix_decode(k, m, w, matrix, 0, erasures, data_ptrs, code_ptrs, size);
    
    gettimeofday(&startTime, NULL);
    hichhikerxor_matrix_decode(k, m, w, matrix, erasures, data_ptrs, code_ptrs, coupleSize, size);
    gettimeofday(&endTime, NULL);
    timeElapsedForHitchhikerXORDecode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
    ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;


    int cIdx;
    for (cIdx = 0; cIdx < size; ++cIdx) {
        if (*(data_ptrs[failedK] + cIdx) != *(dataBlocks+k * size + cIdx)) {
            printf("%d \t %d \t idx:%d not equal\n",*(data_ptrs[failedK] + cIdx), *(dataBlocks+k * size + cIdx), cIdx);
            break;
        }
    }
    
    memcpy(ompDataBlocks, dataBlocks, (k+1) * size);
    gettimeofday(&startTime, NULL);
    omp_hichhikerxor_matrix_encode(k, m, w, matrix, ompData_ptrs, ompCode_ptrs,coupleSize, size);
    gettimeofday(&endTime, NULL);
    timeElapsedForOMPEncode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
    ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;
    
    for (idx = 0; idx < m; ++idx) {
        for (cIdx = 0; cIdx < size; ++cIdx) {
            char ompCh= *(ompCode_ptrs[idx]+cIdx);
            char hCh = *(code_ptrs[idx] + cIdx);
            if (ompCh != hCh) {
                printf("ompencode:%d \t %d \t idx:%d not equal\n",ompCh, hCh, cIdx);
                break;

            }
        }
    }
    
    erasures[0] = failedK;
    erasures[1] = -1;
    gettimeofday(&startTime, NULL);
    omp_hichhikerxor_matrix_decode(k, m, w, matrix, erasures, ompData_ptrs, ompCode_ptrs, coupleSize, size);    gettimeofday(&endTime, NULL);
    timeElapsedForOMPDecode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
    ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;

    for (cIdx = 0; cIdx < size; ++cIdx) {
        if (*(ompData_ptrs[failedK] + cIdx) != *(ompDataBlocks+k * size + cIdx)) {
            printf("omp:%d \t %d \t idx:%d not equal\n",*(ompData_ptrs[failedK] + cIdx), *(ompDataBlocks+k * size + cIdx), cIdx);
            break;
        }
    }

        gettimeofday(&startTime, NULL);
        jerasure_matrix_encode(k, m, w, matrix, data_ptrs, code_ptrs, size);
        gettimeofday(&endTime, NULL);
        timeElapsedForJerasureEncode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
        ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;
    
    
        erasures[0] = failedK;
        erasures[1] = -1;
        memcpy(dataBlocks+k * size, data_ptrs[failedK], size);
        memset(data_ptrs[failedK], 0, size);
        gettimeofday(&startTime, NULL);
        jerasure_matrix_decode(k, m, w, matrix, 0, erasures, data_ptrs, code_ptrs, size);
    //    omp_hichhikerxor_matrix_decode(k, m, w, matrix, erasures, data_ptrs, code_ptrs, coupleSize, size);
        gettimeofday(&endTime, NULL);
        timeElapsedForJerasureDecode = ((double)(endTime.tv_sec - startTime.tv_sec)) * 1000.0 +
        ((double)(endTime.tv_usec - startTime.tv_usec))/1000.0;

    
    double hEncodeThroughput,hDecodeThroughput,
            ompEncodeThroughput, ompDecodeThroughput,
            jEncodeThroughput, jDecodeThroughput;
    
    hEncodeThroughput = ((double)(k+m)*size)* 1000.0/timeElapsedForHitchhikerXOREncode/1024.0/1024.0;
    hDecodeThroughput = ((double)(k+1)*size)* 1000.0/timeElapsedForHitchhikerXORDecode/1024.0/1024.0;
    
    ompEncodeThroughput = ((double)(k+m)*size)* 1000.0/timeElapsedForOMPEncode/1024.0/1024.0;
    ompDecodeThroughput = ((double)(k+m)*size)* 1000.0/timeElapsedForOMPDecode/1024.0/1024.0;
    
    jEncodeThroughput = ((double)(k+m)*size)* 1000.0/timeElapsedForJerasureEncode/1024.0/1024.0;
    jDecodeThroughput = ((double)(k+1)*size)* 1000.0/timeElapsedForJerasureDecode/1024.0/1024.0;
    
    printf("hEncode:%fms, hDecode:%fms, ompEncode:%fms,ompDecode:%fms, jEncode:%fms, jDecode:%f\n",
           timeElapsedForHitchhikerXOREncode, timeElapsedForHitchhikerXORDecode, timeElapsedForOMPEncode, timeElapsedForOMPDecode,timeElapsedForJerasureEncode, timeElapsedForJerasureDecode);
    
    printf("hEncode:%fMB/s,hDecode:%fMB/s,ompEncode:%fMB/s,ompDecode:%fMB/s jEncode:%fMB/s, jDecode:%fMB/s\n",
           hEncodeThroughput, hDecodeThroughput,ompEncodeThroughput, ompDecodeThroughput, jEncodeThroughput, jDecodeThroughput);

}

