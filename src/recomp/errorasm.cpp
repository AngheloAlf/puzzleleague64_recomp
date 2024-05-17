#include "../ultramodern/ultramodern.hpp"
#include "recomp.h"

extern "C" void __osError_recomp(uint8_t* rdram, recomp_context* ctx) {
    s16 code = ctx->r4;
    s16 numArgs = ctx->r5;

    fprintf(stderr, "<__osError> code: %i, numArgs: %i \n", code, numArgs);
}
