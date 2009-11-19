#include <stdio.h>

#include "Foo/Foo.h"
#include "Bar/Bar.h"

int
main(int argc, char ** argv)
{
  printf("FooExtern: %d\n", FooExtern);
  printf("BarExtern: %d\n", BarExtern);
  foo();
  bar();
  return 0;
}
