/*
 * =====================================================================================
 *
 *       Filename:  isPowerOfTwo.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2019年06月08日 23时29分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sui Jingfeng (), jingfengsui@gmail.com
 *   Organization:  CASIA(2014-2017)
 *
 * =====================================================================================
 */



#include <stdio.h>
#include <stdlib.h>


int isPowerOfTwo(int val)
{
    return (val & -val) == val;
}

int isPowerOfTwo_2(int number)
{
    return ((number & number - 1) == 0);
}

int main()
{
    int i = 0;

    for (i = 0; i < 2049; ++i )
    {
        if(isPowerOfTwo(i))
            printf("%d, ", i);
    }

    for (i = 0; i < 2049; ++i )
    {
        if(isPowerOfTwo_2(i))
            printf("%d, ", i);
    }

    return 0;
}
