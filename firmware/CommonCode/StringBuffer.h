//
// StringBuffer 
//
// StringBuffer is used to hold the bytes of a string
// 
//
#ifndef STRINGBUFFER_LOADED
#define STRINGBUFFER_LOADED

#include "booltype.h"
#include "inttypes.h"
#include <string.h>

#define StringBuffer_max_length 20
typedef struct {
   uint8_t length;
   char bytes[StringBuffer_max_length];
   } StringBuffer;

// initializes the buffer to an empty string
inline void StringBuffer_init (
   StringBuffer *buffer)
   {
   buffer->length = 0;
   // set all bytes to zero so the string will always be null-terminated
   memset(buffer->bytes, 0, StringBuffer_max_length);
   }

inline void StringBuffer_append_byte (
   const char byte,
   StringBuffer *buffer)
   {
   buffer->bytes[buffer->length] = byte;
   ++buffer->length;
   }

inline uint8_t StringBuffer_length (
   const StringBuffer *buffer)
   {
   return buffer->length;
   }

inline const char* StringBuffer_bytes (
   const StringBuffer *buffer)
   {
   return buffer->bytes;
   }

#endif   /* UARTSTRINGBUFFER_LOADED */
