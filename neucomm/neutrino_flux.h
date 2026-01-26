#ifndef NEUTRINO_FLUX_H
#define NEUTRINO_FLUX_H

#include <Python.h>

typedef struct NeutrinoMomentum{
    double p; //longitudinal (?) momentum (on-axis)
    double p_t; //Transverse momentum
    double number_neutrinos; //number of neutrinos
} NeutrinoMomentum;

typedef struct NeutrinoFlux{
    PyObject_HEAD;
    NeutrinoMomentum (*distribution)(double protons, double* args);
    double ppb;
    long bunches;
    double bunch_spacing;
    double rise_time;
    double fall_time;
    double* args;
} NeutrinoFlux;

extern PyTypeObject NeutrinoFluxPy;

void NeutrinoFluxPy_init(PyTypeObject* self);
#endif