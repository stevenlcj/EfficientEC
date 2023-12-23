//
//  main.c
//  HitchhikerXor
//
//  Created by Lau SZ on 2022/6/16.
//  Copyright © 2022 Shenzhen Technology University. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "ECMeasure.h"
#define talloc(type, num) (type *) malloc (sizeof(type)*(num))

#define ARG_SIZE 6

void usage(){
    printf("the program needs five args: k,m,w,sizeInKB threadNumForOMP\n");
    exit(0);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc < ARG_SIZE) {
        usage();
    }
    int k,m,w,coupleSize, totalSize, threadNumForOMP;
    
    k = atoi(argv[1]);
    m = atoi(argv[2]);
    w = atoi(argv[3]);
    totalSize = atoi(argv[4]) * 1024;
    coupleSize = totalSize /2;
    threadNumForOMP = atoi(argv[5]);
    
    hitchhikerXOROneFailedMeasure(k, m, w, coupleSize, totalSize,threadNumForOMP);
    
    return 0;
}

