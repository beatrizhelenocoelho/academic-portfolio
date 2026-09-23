/* Include files */

#include "LKATestBenchExample_cgxe.h"
#include "m_dG2RDoTkg0Y58HEEmUsuH.h"

unsigned int cgxe_LKATestBenchExample_method_dispatcher(SimStruct* S, int_T
  method, void* data)
{
  if (ssGetChecksum0(S) == 158540411 &&
      ssGetChecksum1(S) == 3019984762 &&
      ssGetChecksum2(S) == 2642220818 &&
      ssGetChecksum3(S) == 3577770328) {
    method_dispatcher_dG2RDoTkg0Y58HEEmUsuH(S, method, data);
    return 1;
  }

  return 0;
}
