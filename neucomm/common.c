#include <math.h>
#include <time.h>
#include "common.h"

const double c = 299792458.0;

/*-------------------------------------------*/
/* Defining struct/python class "Extraction" */
/*-------------------------------------------*/
//Deallocator: 
static void Extraction_dealloc(Extraction *self){
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* Extraction_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    Extraction* self;
    self = (Extraction*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static int Extraction_init(Extraction *self, PyObject *args, PyObject *kwds){
    double ppb;
    long bunches;
    double bunch_spacing;
    double rise_time;
    double fall_time;

    if(!PyArg_ParseTuple(args, "dlddd", &ppb, &bunches, &bunch_spacing, &rise_time, &fall_time)){
        return -1;
    }

    self->ppb = ppb;
    self->bunches = bunches;
    self->bunch_spacing = bunch_spacing;
    self->rise_time = rise_time;
    self->fall_time = fall_time;
    return 0;
}

static PyMethodDef Extraction_methods[] = {
    {NULL}
};

PyTypeObject ExtractionPy = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

void ExtractionPy_init(PyTypeObject* self){
    self->tp_name = "neucomm.Extraction";
    self->tp_basicsize = sizeof(Extraction);
    self->tp_itemsize = 0;
    self->tp_dealloc = (destructor)Extraction_dealloc;
    self->tp_flags = Py_TPFLAGS_DEFAULT;
    self->tp_doc =
    "A (somewhat simple) datatype describing 1 proton extraction.\nUsually passed to an accelerator to generate a neutrino flux";
    self->tp_methods = Extraction_methods;
    self->tp_init = (initproc)Extraction_init;
    self->tp_new = Extraction_new;
}


// /*----------------------------------------------*/
// /* Defining the struct/python type NeutrinoFlux */
// /*----------------------------------------------*/

// static void NeutrinoFlux_dealloc(NeutrinoFlux *self)
// {
//     if (!self) return;

//     if (self->energy_distribution) {
//         for (int i = 0; i < self->number_bunches; i++) {
//             Distribution_dealloc(self->energy_distribution[i]);
//         }
//         free(self->energy_distribution);
//     }

//     Py_TYPE(self)->tp_free((PyObject*)self);
// }


// static PyObject* NeutrinoFlux_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
//     NeutrinoFlux* self;
//     self = (NeutrinoFlux*)type->tp_alloc(type, 0);

//     //Need to initialise this shit for some reason
//     self->energy_distribution = NULL;
//     self->number_bunches = 0;
//     self->spacing = 0.0;


//     return (PyObject *)self;
// }


// static PyMethodDef NeutrinoFlux_methods[] = {
//     {NULL}
// };

// PyTypeObject NeutrinoFluxPy = {
//     PyVarObject_HEAD_INIT(NULL, 0)
// };

// void NeutrinoFluxPy_init(PyTypeObject* self){
//     self->tp_name = "neucomm.NeutrinoFlux";
//     self->tp_basicsize = sizeof(NeutrinoFlux);
//     self->tp_itemsize = 0;
//     self->tp_dealloc = (destructor)NeutrinoFlux_dealloc;
//     self->tp_flags = Py_TPFLAGS_DEFAULT;
//     self->tp_doc = 
//     "A datatype describing a neutrino flux.\nShould be passed to a detector model to generate a list of times a neutrino was detected";
//     self->tp_methods = NeutrinoFlux_methods;
//     self->tp_new = NeutrinoFlux_new;
// }



/*---------------------------------*/
/* Poisson distribution calculator */
/*---------------------------------*/
int poisson(double expectation){
    //generate a random "threshold" value
    double threshold = rand()/(double)(RAND_MAX);
    //current number of events we are considering
    int n_events = 0;
    //keep track of the running sum
    double log_sum = 0;
    //Then the probability/ log probability
    double log_p;
    double p;
    //Loop through until the threshold is met
    while(1){
        log_p = ((double)n_events*log(expectation)) - expectation - log_sum;
        p = exp(log_p);
        if(p>threshold)
        {
            return n_events;
        }
        n_events++;
        threshold -= p;
        log_sum += log(n_events);
    }
}


// Distribution* Distribution_alloc(int number_buckets){
//     Distribution* distribution = malloc(sizeof(Distribution));
//     distribution->number_buckets = number_buckets;
//     distribution->buckets_center = (double*)malloc(number_buckets*sizeof(double));
//     distribution->buckets_value = (double*)malloc(number_buckets*sizeof(double));
//     return distribution;
// }

// //dealloc a Distribution
// void Distribution_dealloc(Distribution *self){
//     if (!self) return;
//     free(self->buckets_center);
//     free(self->buckets_value);
//     free(self);
// }