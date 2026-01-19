#include <Python.h>
#include "basic_func.h"
#include "common.h"

static PyMethodDef neucommMethods[] = {
    {"simple_minerva", simple_minerva_py, METH_VARARGS, "A very basic model. Takes in a bunch size, and applies the MINERvA experimental event rate (0.81 per 2.25e13 POT). Returns the number of detected neutrinos"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef neucommModule = {
    PyModuleDef_HEAD_INIT,
    "_core",
    "Python libraries for a neutrino communication project",
    -1,
    neucommMethods
};

PyMODINIT_FUNC PyInit__core(void) {
    //set seed:
    srand(time(NULL));

    ExtractionPy_init(&ExtractionPy);
    if(PyType_Ready(&ExtractionPy) < 0){
        return NULL;
    }

    PyObject *neucomm = PyModule_Create(&neucommModule);

    Py_INCREF(&ExtractionPy);
    if(PyModule_AddObject(neucomm, "Extraction", (PyObject *)&ExtractionPy) < 0){
        Py_DECREF(&ExtractionPy);
        Py_DECREF(neucomm);
        return NULL;
    }

    return neucomm;
}

