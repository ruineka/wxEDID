
#ifndef DEF_TYPES_H
#define DEF_TYPES_H 1

#include <stdint.h>
#include <sys/types.h>


#ifndef __HAVE_U8_T
   #define __HAVE_U8_T
   typedef uint8_t u8_t;
#endif

#ifndef __HAVE_U16_T
   #define __HAVE_U16_T
   typedef uint16_t u16_t;
#endif

#ifndef __HAVE_U32_T
   #define __HAVE_U32_T
   typedef uint32_t u32_t;
#endif

#ifndef __HAVE_U64_T
   #define __HAVE_U64_T
   typedef uint64_t u64_t;
#endif

#ifndef __HAVE_I8_T
   #define __HAVE_I8_T
   typedef int8_t i8_t;
#endif

#ifndef __HAVE_I16_T
   #define __HAVE_I16_T
   typedef int16_t i16_t;
#endif

#ifndef __HAVE_I32_T
   #define __HAVE_I32_T
   typedef int32_t i32_t;
#endif

#ifndef __HAVE_I64_T
   #define __HAVE_I64_T
   typedef int64_t i64_t;
#endif


#endif /* DEF_TYPES_H */
