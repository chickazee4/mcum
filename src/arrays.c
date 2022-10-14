#include "mcum.h"
#include <stdint.h>

uint64_t *
includes_snowflake(uint64_t list[], uint64_t toFind, int c){
    for(int i = 0; i < c; i++){
        if(list[i] == toFind){
            return &list[i];
        }
    }
    return NULL;
}

uint64_t *
includes_snowflake_ptr(uint64_t **list, uint64_t toFind, int c){
    for(int i = 0; i < c; i++){
        if(*list[i] == toFind){
            return &(*list[i]);
        }
    }
    return NULL;
}

uint64_t **
append_snowflake(uint64_t **list, uint64_t add, int count){
    *list = realloc(count + sizeof(uint64_t));
    list[count] = add;
    return list;
}

uint64_t **
remove_snowflake(uint64_t **list, uint64_t del, int *count){
    for(int i = 0; i < *count; i++){
        if (*list[i] == del){
            if(i < *count - 1){
                *count--;
                *list[i] = *list[*count];
                *list[*count] = 0;
                break;
            } else {
                *list[i] = 0;
                break;
            }
        }
    }
    return list;
}