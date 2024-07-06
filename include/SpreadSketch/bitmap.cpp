#include "bitmap.h"


int countzerobits(bitmap bmp, int from, int to){
  int z=0;
  for(int j=from;j<to;j++){
    if(!getbit(j,bmp)){
      z++;
    }
  }
  return z;
}
