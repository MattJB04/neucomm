//Implementation of the Reed-Soloman encoder
#include <stdlib.h>
#include "codes.h"

#define GF_POLY = 0x187
#define PPM_SLOTS = 256 //not sure I need this, I think I am mandating 8 bytes at this point (could make more if needed)
#define MSG_LEN = 4
#define PARITY_LEN = 2
#define TOTAL_LEN = MSG_LEN + PARITY_LEN // ditto

/*-------------------------*/
/* Galois Filed Arithmetic */
/*-------------------------*/

u8 gf_add(u8 a,u8 b){
    return a^b;
}

u8 gf_mult(u8 a, u8 b){
    u8 p;
    for(int i = 0; i < 8; i++){
        if((b & 1) != 0){
            p ^= a;
        }

        u8 carry = (a & 0x80);

        a = (a << 1);

        
    }
}