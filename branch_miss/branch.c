#include <stdio.h>

int result = 0;
#define WORK(x, y) \
int work_##x() \
{ \
    if (y == 0) \
        return; \
    result++; \
    int i=0; \
    int j=0; \
    if(!i) \
        work_##y(); \
    else \
        j--; \
    return result; \
} \

// WORK(1,0)
// WORK(2,1)
// WORK(3,2)
WORK(4,3)


int work_1()
{
    if (0 == 0)
        return;
}


int work_2()
{
    if (1 == 0)
        return;
    result++;
    int i=0;
    int j=0;
    if(!i)
        work_1();
    else
        j--;
    return result;
}


int work_3()
{
    if (2 == 0)
        return;
    result++;
    int i=0;
    int j=0;
    if(!i)
        work_2();
    else
        j--;
    return result;
}


int main()
{
    int i=0;
    for(i=0;i<50000;i++)
        work_4();
    return 0;
}

