#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  set_physical_mem();
  void* virtual_address1 = t_malloc(1000);
  unsigned int vp = (unsigned int)virtual_address1;
  int result = t_free(vp, 1);
}