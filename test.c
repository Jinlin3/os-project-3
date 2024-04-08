#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  set_physical_mem();
  mat_mult(1, 1, 1, 1, 1, 1);
}