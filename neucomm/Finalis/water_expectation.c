#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <Python.h>
#include "water_expectation.h"

#define pi 3.14159265359

void dealloc_distribution(distribution* x){
    free(x->bounds);
    free(x->values);
}

double l_mu_param_huber(double E_mu){
    return 14.5 + 3.4*E_mu;
}

double water_expectation(distribution* neutrinos, distribution* cross_section, double length_func(double), double distance){
    //define some common values 
    double n_water = 3.34e28;
    double m_mu = 0.10566;
    double threshold = 0;
    
    double average_detections = 0;
    //loop over the energy for the neutrinos:
    for(size_t i = 0; i < neutrinos->size; i++){
        //Get the energy we are currently considering
        double E_v = (neutrinos->bounds[i+1] + neutrinos->bounds[i])/2;
        //Also the energy step
        double dE_v = neutrinos->bounds[i+1] - neutrinos->bounds[i];

        //muon energy 
        double E_mu = E_v - m_mu;

        double l_mu = length_func(E_mu);

        //now need the cross-section
        double sigma = 0;
        for(size_t j = 0; j < cross_section->size; j++){
            //check that the energy we are considering is not too high
            double E_mu  = (cross_section->bounds[j+1]+cross_section->bounds[j])/2;
            if(E_mu > E_v){
                break;
            }
            //Need to get the change in energy
            double dE_mu = cross_section->bounds[j+1]-cross_section->bounds[j];

            sigma += cross_section->values[j]*dE_mu;
        }//cross section loop
        
        average_detections += sigma*n_water*l_mu*neutrinos->values[i]*dE_v*(1-exp(-distance/l_mu))*18;
    }//neutrino loop
    return average_detections;
}

distribution* Huber_flux(double* energy_arr, size_t energy_size, double E_0, double L, double N_mu, double area){
    double m_mu = 0.10566;
    double m_mu_2 = m_mu*m_mu;
    size_t size = energy_size - 1;

    double* neutrinos = (double*)malloc(size*sizeof(double));

    double beta = sqrt(1-(m_mu_2/(E_0*E_0)));
    //for area, we will use either the given area or the area based on theta_0, whichevers smaller
    double theta_0 = m_mu/E_0;
    double r = L*tan(theta_0);
    double temp = pi*r*r;
    if(area > temp){
        area = temp;
    }
    for(size_t i=0; i < size; i++){
        double E_v = (energy_arr[i+1] + energy_arr[i])/2;

        double temp1 = (2*E_0*N_mu)/(m_mu_2*m_mu_2*m_mu_2 * pi * L*L);
        double temp2 = (1-beta)*E_v*E_v;
        double temp3 = 3*m_mu_2 - 4*E_0*E_v*(1-beta);
        double temp4 = temp1*temp2*temp3*area;
        if(temp4 < 0){
            neutrinos[i] = 0;
            continue;
        }
        neutrinos[i] = temp4;
    }
    distribution* flux = (distribution*)distributionPy.tp_alloc(&distributionPy, 0);
    flux->size = size;
    flux->values = neutrinos;
    flux->bounds = energy_arr;
    return flux;
}

distribution* cross_section_huber(double* energy_array, size_t energy_array_size){
    double size = energy_array_size - 1;
    double m_mu = 0.10566;

    double sigma_0 = 1.583e-42;
    double Q = 0.41;
    double Q_bar = 0.08;

    double* cross_section = (double*)malloc(size*sizeof(double));

    for(size_t i = 0; i < size; i++){
        double E_mu = (energy_array[i] + energy_array[i+1])/2;
        cross_section[i] = sigma_0*(Q + Q_bar*(E_mu/(E_mu + m_mu)));
    }

    distribution* cs_dist = (distribution*)distributionPy.tp_alloc(&distributionPy, 0);
    cs_dist->size = size;
    cs_dist->values = cross_section;
    cs_dist->bounds = energy_array;
    return cs_dist;
}

PyObject* distribution_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
    distribution* self;
    self = (distribution*)type->tp_alloc(type, 0);

    self->values = NULL;
    self->bounds = NULL;
    self->size = 0;

    return (PyObject *)self;
}

void distribution_dealloc(distribution *self){
    if(!self) return;

    if(self->values){
        free(self->values);
    }

    if(self->bounds){
        free(self->bounds);
    }

    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMethodDef distribution_methods[] = {
    {NULL}
};

void distributionPy_init(PyTypeObject* self){
    self->tp_name = "neucomm.distribution";
    self->tp_basicsize = sizeof(distribution);
    self->tp_itemsize = 0;
    self->tp_dealloc = (destructor)distribution_dealloc;
    self->tp_flags = Py_TPFLAGS_DEFAULT;
    self->tp_doc =
    "A datatype that describes a distribution";
    self->tp_methods = distribution_methods;
    self->tp_new = distribution_new;
}

PyTypeObject distributionPy = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

PyObject* water_expectationPy(PyObject *self, PyObject *args){
    distribution* neutrinos;
    distribution* cross_section;
    int length_option;
    double distance;

    if(!PyArg_ParseTuple(args, "O!O!id", &distributionPy, &neutrinos, &distributionPy, &cross_section, &length_option, &distance)){
        return NULL;
    }

    double (*l_mu)(double);
    switch(length_option){
        default:
            l_mu = l_mu_param_huber;
    }
    double expectation = water_expectation(neutrinos, cross_section, l_mu, distance);
    return PyFloat_FromDouble(expectation);
}

PyObject* cross_section_huberPy(PyObject *self, PyObject *args){
    PyObject* energy_list;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &energy_list)){
        return NULL;
    }

    size_t array_size = PyList_GET_SIZE(energy_list);
    double* array = (double*)malloc(array_size*sizeof(double));
    for(size_t i = 0; i < array_size; i++){
        array[i] = PyFloat_AsDouble(PyList_GetItem(energy_list, i));
    }

    distribution* result;
    result = cross_section_huber(array, array_size);
    return (PyObject*)result;
}

PyObject* Huber_fluxPy(PyObject *self, PyObject *args){
    PyObject* energy_list;
    double E_0, L, N_mu, area;
    if(!PyArg_ParseTuple(args, "O!dddd", &PyList_Type, &energy_list, &E_0, &L, &N_mu, &area)){
        return NULL;
    }

    size_t array_size = PyList_GET_SIZE(energy_list);
    double* array = (double*)malloc(array_size*sizeof(double));
    for(size_t i = 0; i < array_size; i++){
        array[i] = PyFloat_AsDouble(PyList_GetItem(energy_list, i));
    }

    distribution* result;
    result = Huber_flux(array, array_size, E_0, L, N_mu, area);
    return (PyObject*)result;
}

PyObject* MINERvA_fluxPy(PyObject *self, PyObject *args){
    double a = 2.9745/(1e6);// paper gives data as v/m^2/GeV/10^6POT for MINERvA. I want the raw number of neutrinos per POT, so I multiply by cross-section area of MINERvA and scale by POT
    double POT = 2.23e13;
    if(!PyArg_ParseTuple(args, "|d", &POT)){
        return NULL;
    }
    a = a*POT;
    //I think I will have to do this manually (there are more data points, but I'll stop at 10 for now)
    //data from paper "Neutrino Flux Predictions for the NuMI Beam"
    double bucket_bounds[] = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5, 13, 13.5, 14, 14.5, 15};
    double buckets_value[] = {a*7.0, a*15.0, a*35.0, a*51.0, a*67.0, a*86.0, a*79.0, a*49.0, a*21.0, a*12.0, a*8.0, a*6.0, a*5.0, a*4.5, a*4.0, a*3.75, a*3.5, a*3.2, a*3.1, a*3.0, a*2.9, a*2.8, a*2.7, a*2.6, a*2.5, a*2.4, a*2.3, a*2.2, a*2.1, a*2.0};
    int number_buckets = 30;

    double* bounds = (double*)malloc((number_buckets+1)*sizeof(double));
    double* values = (double*)malloc((number_buckets)*sizeof(double));

    for(size_t i = 0; i < number_buckets; i++){
        bounds[i] = bucket_bounds[i];
        values[i] = buckets_value[i];
    }
    bounds[number_buckets] = bucket_bounds[number_buckets];

    distribution* flux = (distribution*)distributionPy.tp_alloc(&distributionPy, 0);
    flux->size = number_buckets;
    flux->values = values;
    flux->bounds = bounds;
    return (PyObject*)flux;
}