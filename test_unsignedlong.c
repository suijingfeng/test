#include <stdio.h>
#include <stdlib.h>


int main (int argc, char * argv[])
{
    unsigned int dummy = 0x20534444;

    if(dummy == 0x20534444UL)
    {
        printf("Great: dummy (32bit) = 0x20534444UL (64bit)");
    }
    else
    {
        printf("Great: dummy (32bit) != %ld (64bit)", 0x20534444UL);
    }

}
