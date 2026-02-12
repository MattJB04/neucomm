#include <Python.h>
#ifndef COMMON_H
#define COMMON_H
//Struct for a simple extraction. Might add more details later. For now it just has protons per bunch, rise time and fall time
typedef struct Extraction{
    PyObject_HEAD
    double ppb; //protons per bunch
    long bunches; //Number of bunches
    double bunch_spacing; //Bunch spacing
    double rise_time; //rise time for the kicker magnets
    double fall_time; //fall time for the kicker magnets (might be unnecassary?)
} Extraction;
//Struct for a simple extraction. Might add more details later. For now it just has protons per bunch, rise time and fall time. Python version.
extern PyTypeObject ExtractionPy;
//Needs an initialliser for reasons
void ExtractionPy_init(PyTypeObject* self);


//poisson distribution calculator
int poisson(double expectation);

extern const double c; //speed of light in m/s
extern const double muon_mass; //muon rest mass in GeV
extern const double muon_average_lifetime; //muon average lifetime in seconds (halflife/ln(2))

// //A type for a discrete 1D distribution
// typedef struct Distribution{
//     double* buckets_center; //Center of the buckets for the distribution
//     double* buckets_value; //The value of the buckets
//     int number_buckets; //The number of buckets
// } Distribution;
// //And python versions
// extern PyTypeObject NeutrinoFluxPy;
// void NeutrinoFluxPy_init(PyTypeObject* self);
// //allocate memmory for distribution
// Distribution* Distribution_alloc(int number_buckets);
// //deallocate memory for distribution
// void Distribution_dealloc(Distribution *self);

// //Struct for a neutrino beam. Has pointers to functions that generate the relevant distribution.
// typedef struct NeutrinoFlux{
//     PyObject_HEAD;
//     Distribution** energy_distribution; //energy distribution
//     long number_bunches; //The number of bunches in one extraction
//     double spacing; //Bunch spacing of the extractions
// } NeutrinoFlux;
#endif