//
//  measureEC.c
//  DoubleCharErasure
//
//  Created by Lau SZ on 2023/8/1.
//  Copyright © 2023 Shenzhen Technology University. All rights reserved.
//

#include "measureEC.h"

#include "dcerasure.h"

#include "jerasure.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#define talloc(type, num) ((type *)malloc(sizeof(type) * num))
#define LONG_SIZE sizeof(long)
#define PACKET_MULTI 128
#define PACKET_SIZE (PACKET_MULTI * LONG_SIZE)

void generateRandValues(char *ptr, int size){
    int idx;
    printf("rand value:%d\n",rand()%256);
    for (idx = 0; idx < size ; ++idx) {
        *(ptr + idx) = rand() % 256;
    }
}

int inErasures(int *erasures, int rIdx, int num){
    int idx;
    for (idx = 0; idx < num; ++idx) {
        if (*(erasures + idx) == rIdx) {
            return 1;
        }
    }
    
    return 0;
}

void generateErasures(int *erasures, int m){
    int idx;
    for (idx = 0; idx < m; ++idx) {
        int rIdx;
        do{
            rIdx = rand()%m;
        }while(inErasures(erasures, rIdx, idx));
        erasures[idx] = rIdx;
    }
    erasures[m] = -1;
}

void printMat(int *matrix, int k, int m){
    int rIdx, cIdx;
    for (rIdx = 0; rIdx < m; ++rIdx) {
        for (cIdx = 0; cIdx < k; ++cIdx) {
            printf("%d ",*(matrix + rIdx*k + cIdx));
        }
        printf("\n");

    }
}

void measureEC(int k, int m, int sizePerBlock){
    int idx, w = 8;
    int *matrix = create_matrix_w8(k, m);
//    printMat(matrix, k, m);
    int *erasures = talloc(int, m+1);
    
    char *data, *code, *validationCode, *decodeData, *decodeCode;
    char **dataPtrs, **codePtrs, **validataionCodePtrs, **decodeDataPtrs, **decodeCodePtrs;
    
    struct timeval startTime, endTime;
    
    data = talloc(char, sizePerBlock * k);
    code = talloc(char, sizePerBlock * m);
    decodeData = talloc(char, sizePerBlock * k);
    decodeCode = talloc(char, sizePerBlock * m);

    validationCode = talloc(char, sizePerBlock * m);
    
    dataPtrs = talloc(char *, k);
    codePtrs = talloc(char *, m);
    
    decodeDataPtrs = talloc(char *, k);
    decodeCodePtrs = talloc(char *, m);

    validataionCodePtrs = talloc(char *, m);
    
    for (idx = 0; idx < k; ++idx) {
        dataPtrs[idx] = (data + idx * sizePerBlock);
        decodeDataPtrs[idx] = (decodeData + idx * sizePerBlock);
    }
    
    for (idx = 0; idx < m; ++idx) {
        codePtrs[idx] = (code + idx * sizePerBlock);
        decodeCodePtrs[idx] = (decodeCode + idx * sizePerBlock);
        validataionCodePtrs[idx] = (validationCode + idx * sizePerBlock);
    }
    
    generateRandValues(data, sizePerBlock * k);
    memset(code, 0, sizePerBlock * m);
    memset(validationCode, 0, sizePerBlock * m);
        
    gettimeofday(&startTime, NULL);
    dcerasure_matrix_encode(k, m, w, matrix, dataPtrs, codePtrs, sizePerBlock);
    gettimeofday(&endTime, NULL);
    
    double timeElapsedInMS = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;
    
    gettimeofday(&startTime, NULL);
    jerasure_matrix_encode(k, m, w, matrix, dataPtrs, validataionCodePtrs, sizePerBlock);
    gettimeofday(&endTime, NULL);
    double jerasureTimeElapsedInMS = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;
    printf("Time elapsed:%fms jerasure Time elapsed:%fms\n", timeElapsedInMS, jerasureTimeElapsedInMS);
    double dcEncodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/timeElapsedInMS;
    double jerasureEncodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/jerasureTimeElapsedInMS;
    printf("dcEncodeThroughput:%fMB/s,jerasureEncodeThroughput:%fMB/s\n",dcEncodeThroughput,jerasureEncodeThroughput);

    
    memcpy(decodeData, data, k * sizePerBlock);
    memcpy(decodeCode, code, m * sizePerBlock);
    generateErasures(erasures, m);
    
    gettimeofday(&startTime, NULL);
    dcerasure_matrix_decode(k, m, w, matrix, 0, erasures, decodeDataPtrs, decodeCodePtrs, sizePerBlock);
    gettimeofday(&endTime, NULL);
    double dcDecodeTime = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;

    gettimeofday(&startTime, NULL);
    jerasure_matrix_decode(k, m, w, matrix, 0, erasures, dataPtrs, codePtrs, sizePerBlock);
    gettimeofday(&endTime, NULL);
    
    double jerasureDecodeTime = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;
    
    double dcDecodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/dcDecodeTime;
    double jerasureDecodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/jerasureDecodeTime;

    printf("dcDecodeTime:%fms, jerasureDecodeTime:%fms\n",dcDecodeTime, jerasureDecodeTime);
    printf("dcDecodeThroughput:%fMB/s,jerasureDecodeThroughput:%fMB/s\n",dcDecodeThroughput,jerasureDecodeThroughput);
    
    
    //correctness validation
    for (idx = 0; idx < m; ++idx) {
        if (erasures[idx] < k) {
            int startIdx = erasures[idx] * sizePerBlock;
            int endIdx = startIdx + sizePerBlock;
            for (; startIdx < endIdx; ++startIdx) {
                if (*(data + startIdx) != *(decodeData + startIdx)) {
                    printf("idx:%d not equal at:%d",idx, startIdx);
                    break;
                }
            }
        }else{
            int startIdx = (erasures[idx] - k)* sizePerBlock;
            int endIdx = startIdx + sizePerBlock;
            for (; startIdx < endIdx; ++startIdx) {
                if (*(code + startIdx) != *(code + startIdx)) {
                    printf("idx:%d not equal at:%d",idx, startIdx);
                    break;
                }
            }

        }
    }
    
    printf("idx value:%d\n",idx);
}


void measureCRS(int k, int m, int sizePerBlock){
    int *matrix, *bitmatrix;

    int i,j, w = 8,idx;
    matrix = talloc(int, m*k);
    for (i = 0; i < m; i++) {
        
        for (j = 0; j < k; j++) {
            matrix[i*k+j] = galois_single_divide(1, i ^ (m + j), w);
        }
    }
    bitmatrix = jerasure_matrix_to_bitmatrix(k, m, w, matrix);
    
    int *erasures = talloc(int, m+1);
    memset(erasures, 0, sizeof(int)*(m+1));

    char *data, *code, *validationCode, *decodeData, *decodeCode;
    char **dataPtrs, **codePtrs, **validataionCodePtrs, **decodeDataPtrs, **decodeCodePtrs;
    
    struct timeval startTime, endTime;
    
    data = talloc(char, sizePerBlock * k);
    code = talloc(char, sizePerBlock * m);
    decodeData = talloc(char, sizePerBlock * k);
    decodeCode = talloc(char, sizePerBlock * m);
    
    validationCode = talloc(char, sizePerBlock * m);
    
    dataPtrs = talloc(char *, k);
    codePtrs = talloc(char *, m);
    
    decodeDataPtrs = talloc(char *, k);
    decodeCodePtrs = talloc(char *, m);
    
    validataionCodePtrs = talloc(char *, m);
    
    for (idx = 0; idx < k; ++idx) {
        dataPtrs[idx] = (data + idx * sizePerBlock);
        decodeDataPtrs[idx] = (decodeData + idx * sizePerBlock);
    }
    
    for (idx = 0; idx < m; ++idx) {
        codePtrs[idx] = (code + idx * sizePerBlock);
        decodeCodePtrs[idx] = (decodeCode + idx * sizePerBlock);
        validataionCodePtrs[idx] = (validationCode + idx * sizePerBlock);
    }
    
    generateRandValues(data, sizePerBlock * k);
    memset(code, 0, sizePerBlock * m);
    memset(validationCode, 0, sizePerBlock * m);
    
    gettimeofday(&startTime, NULL);
    jerasure_bitmatrix_encode(k, m, w, bitmatrix, dataPtrs, codePtrs, sizePerBlock, PACKET_SIZE);
    gettimeofday(&endTime, NULL);
    
    double crsEncodeTimeElapsedInMS = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;
    
    generateErasures(erasures, m);
    gettimeofday(&startTime, NULL);
    jerasure_bitmatrix_decode(k, m, w, bitmatrix, 0, erasures, dataPtrs, codePtrs, sizePerBlock, PACKET_SIZE);
    gettimeofday(&endTime, NULL);
    double crsDecodeTimeElapsedInMS = (double)(endTime.tv_sec - startTime.tv_sec)* 1000.0 + (double)(endTime.tv_usec - startTime.tv_usec)/1000.0;

    double crsEncodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/crsEncodeTimeElapsedInMS;
    double crsDecodeThroughput = (double)(sizePerBlock * (k+m))*1000.0/1024.0/1024.0/crsDecodeTimeElapsedInMS;

    printf("crsEncodeTimeElapsedInMS:%fms, crsDecodeTimeElapsedInMS:%fms\n",crsEncodeTimeElapsedInMS, crsDecodeTimeElapsedInMS);
    printf("crsEncodeThroughput:%fMB/s,crsDecodeThroughput:%fMB/s\n",crsEncodeThroughput,crsDecodeThroughput);
}
