#include "accelerators.h"
#include "common.h"
#include "neutrino_flux.h"

//numi neutrino distribution, with no transverse momentum (effectively means that all neutrinos pass through the detector)
static NeutrinoMomentum numi_no_transverse(double protons, double* args){
    double a = 1/(2.9745*1e6);// paper gives data as v/m^2/GeV/10^6POT for MINERvA. I want the raw number of neutrinos per POT, so I multiply by cross-section area of MINERvA and scale by POT
    //I think I will have to do this manually (there are more data points, but I'll stop at 10 for now)
    //data from paper "Neutrino Flux Predictions for the NuMI Beam"
    double bucket_center[] = {0.25, 0.75, 1.25, 1.75, 2.25, 2.75, 3.25, 3.75, 4.25, 4.75, 5.25, 5.75, 6.25, 6.75, 7.25, 7.75, 8.25, 8.75, 9.25, 9.75};
    double buckets_value[] = {a*7.0, a*15.0, a*35.0, a*51.0, a*67.0, a*86.0, a*79.0, a*49.0, a*21.0, a*12.0, a*8.0, a*6.0, a*5.0, a*4.5, a*4.0, a*3.75, a*3.5, a*3.2, a*2.0, a*1.7};
    int number_buckets = 20;

    //Normalise the distribution for sampling
    double bucket_width = number_buckets*0.5;
    double area = 0; //area should give average number of neutrinos per proton (per area) so can be used later for scaling
    int i;
    for(i = 0; i < number_buckets; i++){
        area += buckets_value[i]*bucket_width;
    }
    double one_area = 1/area;
    for(i = 0; i < number_buckets; i++){
        buckets_value[i] *= one_area;
    }
    //all of the above are deterministic, so hopefully the c compiler will do all those calculations and store as binary
    NeutrinoMomentum out;
    double threshold = (double)rand()/(double)(RAND_MAX);
    double probability = 0;
    for (i = 0; i < number_buckets; i++){
        probability += buckets_value[i];
        if(probability >= threshold){
            out.p = bucket_center[i];
        }
    }

    out.number_neutrinos = area * protons;
    out.p_t = 0;
    return out;
}


PyObject* numi_no_transverse_py(PyObject *self, PyObject *args)
{
    Extraction* extraction;
    if(!PyArg_ParseTuple(args, "O!", &ExtractionPy, &extraction)){
        return NULL;
    }

    NeutrinoFlux* flux = (NeutrinoFlux*)NeutrinoFluxPy.tp_alloc(&NeutrinoFluxPy, 0);
    flux->distribution = &numi_no_transverse;
    flux->ppb = extraction->ppb;
    flux->bunches = extraction->bunches;
    flux->bunch_spacing = extraction->bunch_spacing;
    flux->rise_time = extraction->rise_time;
    flux->fall_time = extraction->fall_time;
    flux->args = NULL;
    return (PyObject *)flux;
}