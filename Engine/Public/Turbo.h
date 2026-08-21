#pragma once

#include "CommonTypeDefs.h"
#include <cstdlib>

namespace Turbo
{
   /* NOTE(SS) Replace those two with arenas */
   inline void* DEV_MALLOC(SizeType size)
   {
      void* result = malloc(size);
      memset(result, 0, size);

      return result;
   }

   inline void DEV_FREE(void* ptr)
   {
      free(ptr);
   }

   class Engine;
   extern Engine* gEngine;
}
