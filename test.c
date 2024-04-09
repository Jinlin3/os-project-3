#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  set_physical_mem();
  translate(3819503009);
  mat_mult(1, 1, 1, 1, 1, 1);
}