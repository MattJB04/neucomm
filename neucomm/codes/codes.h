#include <stdlib.h>
#include <stdint.h>
#include <Python.h>

#ifndef CODES_H
#define CODES_H

typedef uint8_t u8;
#define INF __UINT8_MAX__


extern u8 shift_in(u8 x, u8 bit);
extern u8 parity(u8 x);
extern int hamming(u8 a, u8 b);

u8 *py_bin_to_byte(PyObject* list, size_t list_size, size_t* bytes_size);
PyObject* byte_to_py_bin(u8* bytes, size_t bytes_size, size_t* list_size);

PyObject* string_to_bits(PyObject* self, PyObject* args);
PyObject* bits_to_string(PyObject* self, PyObject* args);

#include "convoluted.h"
#endif


