#include <Python.h>
#include "basic_func.h"
#include "common.h"
#include "accelerators.h"
#include "codes/codes.h"
#include "neutrino_flux.h"
#include "detectors.h"
#include "Finalis/water_expectation.h"

static PyMethodDef neucommMethods[] = {
    {"simple_minerva", simple_minerva_py, METH_VARARGS, "A very basic model. Takes in a bunch size, and applies the MINERvA experimental event rate (0.81 per 2.25e13 POT). Returns the number of detected neutrinos"},
    {"numi_no_transverse", numi_no_transverse_py, METH_VARARGS, "numi neutrino distribution, with no transverse momentum (effectively means that all neutrinos pass through the detector)"},
    {"encode_convolution", encode_convolution, METH_VARARGS, "Convolution encoder"},
    {"decode_convolution", decode_convolution, METH_VARARGS, "Convolution decoder"},
    {"cylindrical_water", cylindrical_water_py, METH_VARARGS, "Geometrically simplest detector, a cylinder oriented along the beam axis. This one is a water cherenkov detector. "},
    {"string_to_bits", string_to_bits, METH_VARARGS, "Converts a string into an array of bits (probably best to use regular ASCII for this)"},
    {"bits_to_string", bits_to_string, METH_VARARGS, "Converts a list of bits into ASCII characters"},
    {"water_expectation", water_expectationPy, METH_VARARGS, "Calculates the expected number of detections in a water cherenkov detector given a neutrino flux and a cross-section distribution"},
    {"cross_section_huber", cross_section_huberPy, METH_VARARGS, "Calculates the Huber cross-section distribution for a given set of parameters"},
    {"Huber_flux", Huber_fluxPy, METH_VARARGS, "Calculates the Huber flux distribution for a given set of parameters"},
    {"MINERvA_flux", MINERvA_fluxPy, METH_VARARGS, "The same flux that is predicted to be in the MINERvA detector"},
    {"poisson", poissonPy, METH_VARARGS, "Expose the poisson distribution"},
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
    NeutrinoFluxPy_init(&NeutrinoFluxPy);
    distributionPy_init(&distributionPy);

    if(PyType_Ready(&ExtractionPy) < 0 || PyType_Ready(&NeutrinoFluxPy) < 0 || PyType_Ready(&distributionPy) < 0){
        return NULL;
    }
    PyObject *neucomm = PyModule_Create(&neucommModule);

    Py_INCREF(&ExtractionPy);
    if(PyModule_AddObject(neucomm, "Extraction", (PyObject *)&ExtractionPy) < 0){
        Py_DECREF(&ExtractionPy);
        Py_DECREF(neucomm);
        return NULL;
    }
    Py_INCREF(&NeutrinoFluxPy);
    if(PyModule_AddObject(neucomm, "NeutrinoFlux", (PyObject* )&NeutrinoFluxPy) < 0){
        Py_DECREF(&NeutrinoFluxPy);
        Py_DECREF(neucomm);
        return NULL;
    }
    Py_INCREF(&distributionPy);
    if(PyModule_AddObject(neucomm, "distribution", (PyObject* )&distributionPy) < 0){
        Py_DECREF(&distributionPy);
        Py_DECREF(neucomm);
        return NULL;
    }

    return neucomm;
}

