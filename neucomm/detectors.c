#include <Python.h>
#include <stdio.h>
#include "detectors.h"
#include "neutrino_flux.h"
#include "common.h"

int compar(const void* a, const void* b){
    return (*(int*)a - *(int*)b);
}

//Geometrically simplest detector, a cylinder oriented along the beam axis. This one is a water cherenkov detector. 
double* cylindrical_water(NeutrinoFlux* flux, long samples, double distance, double radius, double length, long* array_size){
    //Take the cross section for a charged current interaction for a neutrino, from the paper "From eV to Eev: Neutrino Cross-Sections Across Energy Scales"
    //Very roughly, the paper includes a graph that I am intergrating by eye so kinda meh. No one will read this anyway. Units are m2/GeV
    double cross_section = 0.8e-42;
    //Water cherenkov has relatively high threshold, so I need to check that a neutrino hits that threshold. This is the energy of needed to create a muon, and have it move 
    //faster than the local speed of light
    double energy_threshold = 0.2654;
    //To calculate the probability of interaction, I need the number density of water
    double water_number_density = 33.3679e27;
    long number_of_detections = 0;
    double* detect_array =(double*)malloc(2*(flux->bunches*samples*sizeof(double)));//per bunch per sample, store how many where detected and the corresponding detection time
    double protons_per_sample = flux->ppb/samples;
    NeutrinoMomentum nm;
    double rbylplusd = radius/(length + distance);
    double time = flux->rise_time;
    int i, j;
    //loop over bunches
    for(i = 0; i < flux->bunches; i++){
        //loop over samples
        for(j = 0; j < samples; j++){
            //sample the distribution
            nm = flux->distribution(protons_per_sample, flux->args);
            //find the energy assuming relativistic limit
            double energy = (nm.p + nm.p_t);
            //check that it passes the energy threshold:
            if(energy < energy_threshold){
                detect_array[2* (i*samples + j) + 0] = 0;
                continue;
            }
            //Have to consider the case where p_t and p are 0 seperately to avoid dividing by 0 errors. If p is 0, there are no detections. If p_t is 0, 
            //the full length of the detector can be used to detect the neutrino
            if(nm.p_t <= 0.0){
                //area that the neutrino moves through (in terms of interaction):
                double nArea = length * cross_section * energy;
                //times by the number denity of water to get an interaction probability
                double probability = nArea*water_number_density;

                //I'm going to wack this into a poisson distribution, which seems reasonable if the samples taken have several neutrinos each
                detect_array[2*(i*samples + j) + 0] = poisson(probability*nm.number_neutrinos);
                number_of_detections += detect_array[2*(i*samples + j) + 0];
                //Then calculate the dectection time. Assuming the relativistic limit, then that is just rise time + bunch spacing*bunch number + travel time
                double traveltime = distance/c;
                detect_array[2*(i*samples + j) + 1] = traveltime + time;
                continue;
            }
            else if(nm.p <= 0.0){
                detect_array[2*(i*samples + j) + 0] = 0.0;
                detect_array[2*(i*samples + j) + 1] = time;
                continue;
            }
            double ptbyp = nm.p_t/nm.p;
            double pbypt = 1/ptbyp;
            //First thing to check is if the beam even hits the detector:
            if(nm.p*radius>nm.p_t*distance){
                detect_array[2*(i*samples + j) + 0] = 0.0;
                detect_array[2*(i*samples + j) + 1] = time;
                continue;
            }
            //If it hits the sides of the detector on exit
            if(ptbyp >= rbylplusd){
                //Need to calculate the path length in the detector. Easiest way is to parametrise the coords.
                double dz = radius*pbypt - distance;
                double dxy = radius - distance*ptbyp;
                double path_length = sqrt(dz*dz + dxy*dxy);

                //The rest is similar to the above
                double probability = path_length*cross_section*energy*water_number_density;
                detect_array[2*(i*samples + j) + 0] = poisson(probability*nm.number_neutrinos);
                number_of_detections = detect_array[2*(i*samples + j) + 0];

                //travel time from when it enters the detector, which assumes the time in the detector is much small
                double travel_time = sqrt(distance*distance*(1 + ptbyp*ptbyp))/c;
                detect_array[2*(i*samples + j) + 1] = travel_time + time;
            }
            //if it hits the end of the detector on exit
            else{
                //similar to above
                double dz = length;
                double dxy = length * ptbyp;
                double path_length = sqrt(dz*dz + dxy*dxy);

                double probability = path_length*cross_section*energy*water_number_density;
                detect_array[2*(i*samples + j) + 0] = poisson(probability*nm.number_neutrinos);
                number_of_detections = detect_array[2*(i*samples + j) + 0];

                double travel_time = sqrt(distance*distance*(1 + ptbyp*ptbyp))/c;
                detect_array[2*(i*samples + j) + 1] = travel_time + time;
            }
        }// samples
        time += flux->bunch_spacing;
    }//bunches

    //After populating the detection array, I want to move it into a cleaner array that is simply a lisNeed to calculate the path length in the detector. Easiest way is to parametrise the coords.t of times
    *array_size = number_of_detections;
    double* detect_times = (double *)malloc(*array_size*sizeof(double));
    //more counters 
    int detections, array_indices = 0;
    //loop over bunches
    for(i = 0; i < flux->bunches; i++){
        //loop over samples
        for(j = 0; j < samples; j++){
            //Each entry of detect array can have multiple detections 
            for(detections = 0; detections < detect_array[2*(i*samples + j) + 0]; detections++){
                detect_times[array_indices] = detect_array[2*(i*samples + j) + 1];
                array_indices += 1;
            }
        }
    }

    free(detect_array);
    qsort(detect_times, *array_size, sizeof(double), compar);
    return detect_times;
}//an overall terribly designed function, worthy of a complete rewrite

PyObject* cylindrical_water_py(PyObject *self, PyObject *args)
{
    NeutrinoFlux* flux;
    long samples;
    double distance, radius, length;
    if(!PyArg_ParseTuple(args, "O!dddl", &NeutrinoFluxPy, &flux, &distance, &radius, &length, &samples)){
        return NULL;
    }

    if(samples < 0){
        samples = flux->ppb;
    }

    long array_size;
    double* time_array = cylindrical_water(flux, samples, distance, radius, length, &array_size);
    PyObject *list = PyList_New(array_size);
    long i;
    for(i = 0; i < array_size; i++){
        PyObject *val = PyFloat_FromDouble(time_array[i]);
        PyList_SET_ITEM(list, i, val);
    }

    free(time_array);
    return list;
}