#ifndef RS_HEADER
#define RS_HEADER

#include "codes.h"

PyObject* encode_rs_8(PyObject *self, PyObject *args);
PyObject* decode_rs_erasures_8(PyObject *self, PyObject *args);
PyObject* encode_rs_16(PyObject *self, PyObject *args);
PyObject* decode_rs_erasures_16(PyObject *self, PyObject *args);
extern void gf_init_8(void);
extern void gf_init_16(void);
#endif