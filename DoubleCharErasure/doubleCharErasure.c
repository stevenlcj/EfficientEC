//
//  main.c
//  doubleCharErasure
//
//  Created by Lau SZ on 2023/8/1.
//  Copyright © 2023 Shenzhen Technology University. All rights reserved.
//

#include <stdio.h>
#include <math.h>

#include "measureEC.h"
#include "dcerasure.h"

#include <stdlib.h>

#define ARG_NUM 4

void usage(void){
    printf("Usage:\n");
    printf("./program k m blockSizeInMB\n");
}

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc < ARG_NUM) {
        usage();
        return -1;
    }
    dccreate_galois_w8_mult_table();
    
    int k = atoi(argv[1]), m = atoi(argv[2]), blockSizeInMB = atoi(argv[3]);
    printf("k:%d,m:%d,blockSizeInMB:%d\n",k,m,blockSizeInMB);
    measureEC(k, m,blockSizeInMB*1024*1024);
    measureCRS(k, m,blockSizeInMB*1024*1024);
    return 0;
}
