#include <stdio.h>
#include <time.h>
#include <unistd.h>


#define    TENMULTI    10
#define    OCTMULTI    8
#define    SIXMULTI    6
#define    FOURMULTI   4
#define    TWOMULTI    2

int main(int argc, char **argv)
{
    clock_t start = clock();
    sleep(1);
    clock_t end = clock();

    switch (sizeof(void*)){
     case 4: printf("\nSystem: 32bit\n\n");
      break;
     case 8: printf("\nSystem: 64bit\n\n");
      break;
    }

    printf("sizeof(clock_t)=%d, CLOCKS_PER_SEC=%d\n",
           sizeof(clock_t), CLOCKS_PER_SEC);
    printf("sizeof(size_t)=%d, sizeof(int)=%d\n\n",
           sizeof(size_t), sizeof(int));

    printf("clock()\n");
    printf("C Native:     %d\n", start);
    printf("C Multiplier: %d\t (x%d)\n", start*TWOMULTI, TWOMULTI);
    printf("C Multiplier: %d\t (x%d)\n", start*FOURMULTI, FOURMULTI);
    printf("C Multiplier: %d\t (x%d)\n", start*SIXMULTI, SIXMULTI);
    printf("C Multiplier: %d\t (x%d)\n", start*OCTMULTI, OCTMULTI);
    printf("C Multiplier: %d\t (x%d)\n\n", start*TENMULTI, TENMULTI);


    printf("One sec sleep: %d - %d = %d\n\n", end, start, end-start);
}

/*
 * cc -o clock_debug clock_debug.c
 *
 * game/singe/lua.h
 *
 * There is a weird bug in 64bit land, where Lua os.clock() is off
 *    by a factor of 6 - we kludge here on 64bit systems......
 *
 * #define LUA_64BIT_CLOCK_SKEW    6
 *
 * C function clock() differences:
 *
 * System: 32bit
 *
 * sizeof(clock_t)=4, CLOCKS_PER_SEC=1000000
 * sizeof(size_t)=4, sizeof(int)=4
 *
 * clock()
 * C Native:     4090
 * C Multiplier: 8180       (x2)
 * C Multiplier: 16360      (x4)
 * C Multiplier: 24540      (x6)
 * C Multiplier: 32720      (x8)
 * C Multiplier: 40900      (x10)
 *
 * One sec sleep: 4164 - 4090 = 74
 *
 * System: 64bit
 *
 * sizeof(clock_t)=8, CLOCKS_PER_SEC=1000000
 * sizeof(size_t)=8, sizeof(int)=4
 *
 * clock()
 * C Native:     375
 * C Multiplier: 750        (x2)
 * C Multiplier: 1500       (x4)
 * C Multiplier: 2250       (x6)
 * C Multiplier: 3000       (x8)
 * C Multiplier: 3750       (x10)
 *
 * One sec sleep: 387 - 375 = 12
 *
 */
