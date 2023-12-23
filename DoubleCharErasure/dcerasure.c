//
//  dcerasure.c
//  DoubleCharErasure
//
//  Created by Lau SZ on 2023/8/1.
//  Copyright © 2023 Shenzhen Technology University. All rights reserved.
//

#include "dcerasure.h"
#include "galois.h"
#include "jerasure.h"
#include <string.h>

//#include "GF8Mul.h"

#define talloc(type, num) ((type *)malloc(sizeof(type) * num))
#define STRIDE_SIZE 8

static unsigned short *galois_w8_mult_table = NULL;
static int galois_w8_mult_table_oneregin_size = (1<< 16);
static int galois_w8_mult_table_size = (1<< 16) * (1<<8);

#define MAX_UNSIGNED_SHORT (0b1111111111111111)


void dccreate_galois_w8_mult_table(void){
    galois_w8_mult_table = (unsigned short *)malloc(sizeof(unsigned short) * galois_w8_mult_table_size);

    int idx, maxCharSize = (1<<8) - 1, innerIdx;
    
    //Don't need to handle 0, so start from 1
    for (idx = 1; idx <= maxCharSize; ++idx) {
        
        unsigned short *ptrOffset = galois_w8_mult_table + idx * galois_w8_mult_table_oneregin_size;
        for (innerIdx = 0; innerIdx < galois_w8_mult_table_oneregin_size; ++innerIdx) {
            *(ptrOffset + innerIdx) = (unsigned short)innerIdx;
        }
        
        galois_w08_region_multiply((char *)ptrOffset,
                                   idx,
                                   galois_w8_mult_table_oneregin_size * 2,//The size should be bytes, not the size of short type
                                   NULL,
                                   0);
    }
}

void dcgalois_w08_region_multiply(char *region,       /* Region to multiply */
                                  int multby,       /* Number to multiply by */
                                  int nbytes,       /* Number of bytes in region */
                                  char *result,
                                  int init){
    
    if(galois_w8_mult_table == NULL){
        dccreate_galois_w8_mult_table();
    }
    
//    unsigned long tempResult;
    unsigned long tmpResult[STRIDE_SIZE];
    unsigned short *sPtr = (unsigned short *)&tmpResult;
    unsigned short *reginShortPtr = (unsigned short *)region;
    unsigned short *tableStartPtr = galois_w8_mult_table + multby * galois_w8_mult_table_oneregin_size;
    int idx, shortIdx, size = nbytes / sizeof(long);
    
    if (init == 0) {
        unsigned short *resultShortPtr = (unsigned short *)result;

        for (idx = 0, shortIdx= 0; idx < size; idx= idx + 2, shortIdx= shortIdx + 8) {
            *(resultShortPtr+shortIdx) = *(tableStartPtr + *(reginShortPtr + shortIdx));
            *(resultShortPtr+shortIdx+ 1) = *(tableStartPtr + *(reginShortPtr + shortIdx + 1));
            *(resultShortPtr+shortIdx+ 2) = *(tableStartPtr + *(reginShortPtr + shortIdx + 2));
            *(resultShortPtr+shortIdx+ 3) = *(tableStartPtr + *(reginShortPtr + shortIdx + 3));
            *(resultShortPtr+shortIdx +4 ) = *(tableStartPtr + *(reginShortPtr + shortIdx +4 ));
            *(resultShortPtr+shortIdx+ 5) = *(tableStartPtr + *(reginShortPtr + shortIdx + 5));
            *(resultShortPtr+shortIdx+ 6) = *(tableStartPtr + *(reginShortPtr + shortIdx + 6));
            *(resultShortPtr+shortIdx+ 7) = *(tableStartPtr + *(reginShortPtr + shortIdx + 7));
        }
    }else{
        unsigned long *resultLongPtr = (unsigned long *)result;
        for (idx = 0, shortIdx = 0; idx < size; idx = idx + STRIDE_SIZE , shortIdx = shortIdx + 4*STRIDE_SIZE) {
            sPtr[0] = *(tableStartPtr + *(reginShortPtr + shortIdx + 0));
            sPtr[1] = *(tableStartPtr + *(reginShortPtr + shortIdx + 1));
            sPtr[2] = *(tableStartPtr + *(reginShortPtr + shortIdx + 2));
            sPtr[3] = *(tableStartPtr + *(reginShortPtr + shortIdx + 3));
            
            sPtr[4] = *(tableStartPtr + *(reginShortPtr + shortIdx + 4));
            sPtr[5] = *(tableStartPtr + *(reginShortPtr + shortIdx + 5));
            sPtr[6] = *(tableStartPtr + *(reginShortPtr + shortIdx + 6));
            sPtr[7] = *(tableStartPtr + *(reginShortPtr + shortIdx + 7));
            
            sPtr[8] = *(tableStartPtr + *(reginShortPtr + shortIdx + 8));
            sPtr[9] = *(tableStartPtr + *(reginShortPtr + shortIdx + 9));
            sPtr[10] = *(tableStartPtr + *(reginShortPtr + shortIdx + 10));
            sPtr[11] = *(tableStartPtr + *(reginShortPtr + shortIdx + 11));
            
            sPtr[12] = *(tableStartPtr + *(reginShortPtr + shortIdx + 12));
            sPtr[13] = *(tableStartPtr + *(reginShortPtr + shortIdx + 13));
            sPtr[14] = *(tableStartPtr + *(reginShortPtr + shortIdx + 14));
            sPtr[15] = *(tableStartPtr + *(reginShortPtr + shortIdx + 15));
            
            sPtr[16] = *(tableStartPtr + *(reginShortPtr + shortIdx + 16));
            sPtr[17] = *(tableStartPtr + *(reginShortPtr + shortIdx + 17));
            sPtr[18] = *(tableStartPtr + *(reginShortPtr + shortIdx + 18));
            sPtr[19] = *(tableStartPtr + *(reginShortPtr + shortIdx + 19));
            
            sPtr[20] = *(tableStartPtr + *(reginShortPtr + shortIdx + 20));
            sPtr[21] = *(tableStartPtr + *(reginShortPtr + shortIdx + 21));
            sPtr[22] = *(tableStartPtr + *(reginShortPtr + shortIdx + 22));
            sPtr[23] = *(tableStartPtr + *(reginShortPtr + shortIdx + 23));
            
            sPtr[24] = *(tableStartPtr + *(reginShortPtr + shortIdx + 24));
            sPtr[25] = *(tableStartPtr + *(reginShortPtr + shortIdx + 25));
            sPtr[26] = *(tableStartPtr + *(reginShortPtr + shortIdx + 26));
            sPtr[27] = *(tableStartPtr + *(reginShortPtr + shortIdx + 27));
            
            sPtr[28] = *(tableStartPtr + *(reginShortPtr + shortIdx + 28));
            sPtr[29] = *(tableStartPtr + *(reginShortPtr + shortIdx + 29));
            sPtr[30] = *(tableStartPtr + *(reginShortPtr + shortIdx + 30));
            sPtr[31] = *(tableStartPtr + *(reginShortPtr + shortIdx + 31));

            
//            sPtr[32] = *(tableStartPtr + *(reginShortPtr + shortIdx + 32));
//            sPtr[33] = *(tableStartPtr + *(reginShortPtr + shortIdx + 33));
//            sPtr[34] = *(tableStartPtr + *(reginShortPtr + shortIdx + 34));
//            sPtr[35] = *(tableStartPtr + *(reginShortPtr + shortIdx + 35));
//
//            sPtr[36] = *(tableStartPtr + *(reginShortPtr + shortIdx + 36));
//            sPtr[37] = *(tableStartPtr + *(reginShortPtr + shortIdx + 37));
//            sPtr[38] = *(tableStartPtr + *(reginShortPtr + shortIdx + 38));
//            sPtr[39] = *(tableStartPtr + *(reginShortPtr + shortIdx + 39));
//
//            sPtr[40] = *(tableStartPtr + *(reginShortPtr + shortIdx + 40));
//            sPtr[41] = *(tableStartPtr + *(reginShortPtr + shortIdx + 41));
//            sPtr[42] = *(tableStartPtr + *(reginShortPtr + shortIdx + 42));
//            sPtr[43] = *(tableStartPtr + *(reginShortPtr + shortIdx + 43));
//
//            sPtr[44] = *(tableStartPtr + *(reginShortPtr + shortIdx + 44));
//            sPtr[45] = *(tableStartPtr + *(reginShortPtr + shortIdx + 45));
//            sPtr[46] = *(tableStartPtr + *(reginShortPtr + shortIdx + 46));
//            sPtr[47] = *(tableStartPtr + *(reginShortPtr + shortIdx + 47));
//
//            sPtr[48] = *(tableStartPtr + *(reginShortPtr + shortIdx + 48));
//            sPtr[49] = *(tableStartPtr + *(reginShortPtr + shortIdx + 49));
//            sPtr[50] = *(tableStartPtr + *(reginShortPtr + shortIdx + 50));
//            sPtr[51] = *(tableStartPtr + *(reginShortPtr + shortIdx + 51));
//
//            sPtr[52] = *(tableStartPtr + *(reginShortPtr + shortIdx + 52));
//            sPtr[53] = *(tableStartPtr + *(reginShortPtr + shortIdx + 53));
//            sPtr[54] = *(tableStartPtr + *(reginShortPtr + shortIdx + 54));
//            sPtr[55] = *(tableStartPtr + *(reginShortPtr + shortIdx + 55));
//
//            sPtr[56] = *(tableStartPtr + *(reginShortPtr + shortIdx + 56));
//            sPtr[57] = *(tableStartPtr + *(reginShortPtr + shortIdx + 57));
//            sPtr[58] = *(tableStartPtr + *(reginShortPtr + shortIdx + 58));
//            sPtr[59] = *(tableStartPtr + *(reginShortPtr + shortIdx + 59));
//
//            sPtr[60] = *(tableStartPtr + *(reginShortPtr + shortIdx + 60));
//            sPtr[61] = *(tableStartPtr + *(reginShortPtr + shortIdx + 61));
//            sPtr[62] = *(tableStartPtr + *(reginShortPtr + shortIdx + 62));
//            sPtr[63] = *(tableStartPtr + *(reginShortPtr + shortIdx + 63));

            
//            #pragma unroll
//            for (sIdx = 0; sIdx < STRIDE_SIZE; ++sIdx) {
//                resultLongPtr[sIdx] = resultLongPtr[sIdx] ^ tmpResult[sIdx];
//            }
            resultLongPtr[0] = resultLongPtr[0] ^ tmpResult[0];
            resultLongPtr[1] = resultLongPtr[1] ^ tmpResult[1];
            resultLongPtr[2] = resultLongPtr[2] ^ tmpResult[2];
            resultLongPtr[3] = resultLongPtr[3] ^ tmpResult[3];
            resultLongPtr[4] = resultLongPtr[4] ^ tmpResult[4];
            resultLongPtr[5] = resultLongPtr[5] ^ tmpResult[5];
            resultLongPtr[6] = resultLongPtr[6] ^ tmpResult[6];
            resultLongPtr[7] = resultLongPtr[7] ^ tmpResult[7];
            
            
            resultLongPtr = resultLongPtr + STRIDE_SIZE;
        }
    }
}

void dcerasure_matrix_encode_dotprod(int k, int *vector, char **data_ptrs, char *coding_ptr, int size){
    int init = 0, idx;
    
    for (idx = 0; idx < k; ++idx) {
        if (*(vector + idx) == 0){
            continue;
        }
        
        
        if (*(vector + idx) == 1) {
            if (init == 0) {
                memcpy(coding_ptr ,data_ptrs[idx], size);
            }else{
                galois_region_xor(data_ptrs[idx], coding_ptr,coding_ptr,size);
            }
        }else{
            dcgalois_w08_region_multiply(data_ptrs[idx],
                                         *(vector + idx),
                                         size,       
                                         coding_ptr,
                                         init);
        }
        
        init = 1;

    }
}

int *create_matrix_w8(int k, int m){
    int *matrix = talloc(int, m*k);
    int w= 8;
    for (int i = 0; i < m; i++) {
        
        for (int j = 0; j < k; j++) {
            matrix[i*k+j] = galois_single_divide(1, i ^ (m + j), w);
        }
    }
    
    return matrix;
}

void dcerasure_matrix_encode(int k, int m, int w, int *matrix,
                             char **data_ptrs, char **coding_ptrs, int size){
    int idx;
    for (idx = 0; idx < m; ++idx) {
        dcerasure_matrix_encode_dotprod(k, matrix+(idx*k), data_ptrs, coding_ptrs[idx], size);
    }
}

void dcerasure_matrix_decode_dotprod(int k, int w, int *matrix_row,
                             int *src_ids, int dest_id,
                             char **data_ptrs, char **coding_ptrs, int size)
{
    int init;
    char *dptr, *sptr;
    int i;
    
    if (w != 1 && w != 8 && w != 16 && w != 32) {
        fprintf(stderr, "ERROR: jerasure_matrix_dotprod() called and w is not 1, 8, 16 or 32\n");
        exit(1);
    }
    
    init = 0;
    
    dptr = (dest_id < k) ? data_ptrs[dest_id] : coding_ptrs[dest_id-k];
    
    /* First copy or xor any data that does not need to be multiplied by a factor */
    
    for (i = 0; i < k; i++) {
        if (matrix_row[i] == 1) {
            if (src_ids == NULL) {
                sptr = data_ptrs[i];
            } else if (src_ids[i] < k) {
                sptr = data_ptrs[src_ids[i]];
            } else {
                sptr = coding_ptrs[src_ids[i]-k];
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
                sptr = data_ptrs[i];
            } else if (src_ids[i] < k) {
                sptr = data_ptrs[src_ids[i]];
            } else {
                sptr = coding_ptrs[src_ids[i]-k];
            }
            
            dcgalois_w08_region_multiply(sptr, matrix_row[i], size, dptr, init);
            init = 1;
        }
    }
}

int dcerasure_matrix_decode(int k, int m, int w,
                            int *matrix, int row_k_ones, int *erasures,
                            char **data_ptrs, char **coding_ptrs, int size){
    int i, edd, lastdrive;
    int *tmpids;
    int *erased, *decoding_matrix, *dm_ids;
    
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
    
    /* You only need to create the decoding matrix in the following cases:
     
     1. edd > 0 and row_k_ones is false.
     2. edd > 0 and row_k_ones is true and coding device 0 has been erased.
     3. edd > 1
     
     We're going to use lastdrive to denote when to stop decoding data.
     At this point in the code, it is equal to the last erased data device.
     However, if we can't use the parity row to decode it (i.e. row_k_ones=0
     or erased[k] = 1, we're going to set it to k so that the decoding
     pass will decode all data.
     */
    
    if (!row_k_ones || erased[k]) lastdrive = k;
    
    dm_ids = NULL;
    decoding_matrix = NULL;
    
    if (edd > 1 || (edd > 0 && (!row_k_ones || erased[k]))) {
        dm_ids = talloc(int, k);
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
    
    /* Decode the data drives.
     If row_k_ones is true and coding device 0 is intact, then only decode edd-1 drives.
     This is done by stopping at lastdrive.
     We test whether edd > 0 so that we can exit the loop early if we're done.
     */
    
    for (i = 0; edd > 0 && i < lastdrive; i++) {
        if (erased[i]) {
            dcerasure_matrix_decode_dotprod(k, w, decoding_matrix+(i*k), dm_ids, i, data_ptrs, coding_ptrs, size);
            edd--;
        }
    }
    
    /* Then if necessary, decode drive lastdrive */
    
    if (edd > 0) {
        tmpids = talloc(int, k);
        for (i = 0; i < k; i++) {
            tmpids[i] = (i < lastdrive) ? i : i+1;
        }
        dcerasure_matrix_decode_dotprod(k, w, matrix, tmpids, lastdrive, data_ptrs, coding_ptrs, size);
        free(tmpids);
    }
    
    /* Finally, re-encode any erased coding devices */
    
    for (i = 0; i < m; i++) {
        if (erased[k+i]) {
            jerasure_matrix_dotprod(k, w, matrix+(i*k), NULL, i+k, data_ptrs, coding_ptrs, size);
        }
    }
    
    free(erased);
    if (dm_ids != NULL) free(dm_ids);
    if (decoding_matrix != NULL) free(decoding_matrix);
    
    return 0;
}
