#include "apue.h"
#include <arpa/inet.h>

int main(void) {
  uint32_t netint32 = 0xc0a80101;        /* 192.168.1.1 */
  uint32_t hostint32 = ntohl(netint32);

  printf("hostint32: %u\nnetint32: %u\n", hostint32, netint32);
}