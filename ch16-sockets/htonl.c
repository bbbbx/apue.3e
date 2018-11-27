#include "apue.h"
#include <arpa/inet.h>

int main(void) {
  uint32_t hostint32= 0xc0a80101;         /* 192.168.1.1 */
  uint32_t netint32 = htonl(hostint32);   /* 1.1.168.192 */
  char     addr[15];
  inet_ntop(AF_INET, &netint32, addr, sizeof(addr));

  printf("hostint32: %#x\nnetint32: %#x\ninet_ntop: %s\n", hostint32, netint32, addr);
}
