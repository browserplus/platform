
#include <stdio.h>
#include "header.h"
#include "Bar/Bar.h"

int FooExtern = 42;

void
foo(void)
{
    printf("foo called (bar extern is %d)\n", BarExtern);
    bar();
}
