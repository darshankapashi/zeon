#include "Structs.h"

void throwError(ErrorCode what, string why) {
  ZeonException ze;
  ze.what = what;
  ze.why = why;
  throw ze;
}