#include "neutrino_flux.h"
#include <stdlib.h>
#include <Python.h>

static void NeutrinoFlux_dealloc(NeutrinoFlux *self){
    if(!self) return;

    if (self->args){
        free(self->args);
    }

    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* NeutrinoFlux_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    NeutrinoFlux* self;
    self = (NeutrinoFlux*)type->tp_alloc(type, 0);

    self->distribution = NULL;
    self->ppb = 0.0;
    self->bunches = 0;
    self->rise_time = 0;
    self->fall_time = 0;
    self->args = NULL;

    return (PyObject *)self;
}

static PyMethodDef NeutrinoFlux_methods[] = {
    {NULL}
};

PyTypeObject NeutrinoFluxPy = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

void NeutrinoFluxPy_init(PyTypeObject* self){
    self->tp_name = "neucomm.NeutrinoFlux";
    self->tp_basicsize = sizeof(NeutrinoFlux);
    self->tp_itemsize = 0;
    self->tp_dealloc = (destructor)NeutrinoFlux_dealloc;
    self->tp_flags = Py_TPFLAGS_DEFAULT;
    self->tp_doc =
    "A datatype describing a neutrino flux.\nShould be passed to a detector model to generate a list of times a neutrino was detected";
    self->tp_methods = NeutrinoFlux_methods;
    self->tp_new = NeutrinoFlux_new;
}