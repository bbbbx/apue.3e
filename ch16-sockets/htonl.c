#include "apue.h"
#include <arpa/inet.h>

int main(void) {
  uint32_t hostint32= 0x0101a8c0;        /* 1.1.168.192 */
  uint32_t netint32 = htonl(hostint32);   /* 192.168.1.1 */

  printf("hostint32: %#x\nnetint32: %#x\n", hostint32, netint32);
}
