/*
 * common.hpp
 * - common stuff shared by all files
 */

#ifndef __common_hpp__
#define __common_hpp__

#include <stdint.h>
#include <stdio.h>

/*
 * Convert IP (in network order) to string
 */
inline char* ip2a(uint32_t ip, char* addr) {
  sprintf(addr, "%d.%d.%d.%d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff,
          (ip >> 24) & 0xff);
  return addr;
}

#endif
