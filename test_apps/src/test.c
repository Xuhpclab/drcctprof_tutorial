/* 
 *  Copyright (c) 2022 Xuhpclab. All rights reserved.
 *  Licensed under the MIT License.
 *  See LICENSE file for more information.
 */

#include <stdio.h>
#include <stdlib.h>

int* array = NULL;
int global = 0;

#define ARRAY_SIZE 10000

void array_create() {
    array = (int*)malloc(ARRAY_SIZE * sizeof(int));
}

void array_init() {
    for(int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i;
    }
}

void array_use() {
    for(int i = 0; i < ARRAY_SIZE; i ++) {
        global += array[i];
    }
}

void moo() {
    for(int i =0 ; i < 100; i++) {
        array_use();
    }
}

void foo() {
    array_use();
    moo();
}


int main(){
    array_create();
    array_init();
    foo();
    return 0; 
}