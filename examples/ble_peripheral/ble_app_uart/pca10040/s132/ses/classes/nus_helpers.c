#include "nus_helpers.h"


#ifdef PRINT_VECTORS
static void print_addr(uint8_t* addr){
     for(uint8_t i = 0; i < 6; i++){
          printf("%02x ", addr[i]);
     }
}

static void print_vec_str(vec_string_t * strings) {
  size_t i; const char * string;
  vec_foreach(strings, string, i) {printf("Names[%zu] = \"%s\"\n", i, string);}
}

static void print_vec_byte(vec_byte_t * bytes) {
  size_t i; uint8_t byte;
  printf("Bytes[%zu] = ", i);
  vec_foreach(bytes, byte, i) {printf("%02x ", byte);}
}

static void print_vec_bytes(vec_bytes_t * bytes) {
  size_t i; size_t j;
  uint8_t byte;
  vec_byte_t byte2;
  vec_foreach(bytes, byte2, i) {
    printf("Bytes[%zu] = ", i);
     vec_foreach(&byte2, byte, j) {printf("%02x ", byte);}
  }
     printf("\r\n");
}
#endif
