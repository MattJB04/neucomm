#ifndef WATER_EXPECTATION_H
#define WATER_EXPECTATION_H
#include <Python.h>
typedef struct distribution{
    PyObject_HEAD;
    size_t size;
    double* values;
    double* bounds;
}distribution;

extern PyTypeObject distributionPy;
void distributionPy_init(PyTypeObject* self);

PyObject* water_expectationPy(PyObject *self, PyObject *args);
PyObject* cross_section_huberPy(PyObject *self, PyObject *args);
PyObject* Huber_fluxPy(PyObject *self, PyObject *args);
PyObject* MINERvA_fluxPy(PyObject *self, PyObject *args);
#endif