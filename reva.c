
#include "rva.h"

int
main(int argc, const char **argv) {

  return rvaMain(argc, argv,
                 "reva", "Brevais Lattice Hacking",
                 rvaCmdList, stderr);
}
