#include "utils.h"

void print_buffer(uint8_t* buf, ssize_t Size)
{
  ssize_t i = 0;
  while (i < Size) {
    printf("%02x ", buf[i]);
    i++;
    if (!(i%16)) {
      printf(" | ");
      for (int j=16; j>0; j--)
        if ((buf[i-j] >= 0x20) && (buf[i-j] < 0x7E))
          printf("%c", buf[i-j]);
        else
          printf(".");
      printf("\n");
    }
  }
  
  if (i%16) {
    for (int j=0; j<=15; j++) {
      if ((i+j)%16)
        printf("   ");
      else
        break;
    }
    printf(" | ");
    
    for (int j=(i%16); j>0; j--) {
      if ((buf[i-j] >= 0x20) && (buf[i-j] < 0x7E))
        printf("%c", buf[i-j]);
      else
        printf(".");
    }
  }
  printf("\n");
}
