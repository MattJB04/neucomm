#ifndef CODES_H
#define CODES_H

#include <stdlib.h>
#include <stdint.h>
#include <Python.h>

typedef uint8_t u8;
#define INF __UINT8_MAX__

typedef uint16_t u16;

extern u8 shift_in(u8 x, u8 bit);
extern u8 parity(u8 x);
extern int hamming(u8 a, u8 b);

u8 *py_bin_to_byte(PyObject* list, size_t list_size, size_t* bytes_size);
PyObject* byte_to_py_bin(u8* bytes, size_t bytes_size, size_t* list_size);

PyObject* string_to_bits(PyObject* self, PyObject* args);
PyObject* bits_to_string(PyObject* self, PyObject* args);
PyObject* string_to_bytes(PyObject* self, PyObject* args);
PyObject* bytes_to_string(PyObject* self, PyObject* args);

typedef struct { u8 v; u8 valid; } option_8;
typedef struct {u16 v; u16 valid;} option_16;

option_8 *py_byte_to_option_8(PyObject* list, size_t list_size);
u8 *py_byte_to_u8(PyObject* list, size_t list_size);
PyObject* u8_to_py_byte(u8* bytes, size_t bytes_size);
PyObject* option_8_to_py_byte(option_8* bytes, size_t bytes_size);

option_16 *py_byte_to_option_16(PyObject* list, size_t list_size);
u16 *py_byte_to_u16(PyObject* list, size_t list_size);
PyObject* u16_to_py_byte(u16* bytes, size_t bytes_size);
PyObject* option_16_to_py_byte(option_16* bytes, size_t bytes_size);

#include "convoluted.h"
#include "rs.h"
#endif


