#include "accelerators.h"
#include "common.h"

//numi neutrino distribution, with no transverse momentum (effectively means that all neutrinos pass through the detector)
static NeutrinoFlux* numi_no_transverse(Extraction* extraction){
    double a = 2.9745*1e-6;// paper gives data as v/m^2/GeV/10^6POT for MINERvA. I want the raw number of neutrinos per POT, so I multiply by cross-section area of MINERvA and scale by POT
    //I think I will have to do this manually (there are more data points, but I'll stop at 10 for now)
    //data from paper "Neutrino Flux Predictions for the NuMI Beam"
    double bucket_center[] = {0.25, 0.75, 1.25, 1.75, 2.25, 2.75, 3.25, 3.75, 4.25, 4.75, 5.25, 5.75, 6.25, 6.75, 7.25, 7.75, 8.25, 8.75, 9.25, 9.75};
    double buckets_value[] = {a*7.0, a*15.0, a*35.0, a*51.0, a*67.0, a*86.0, a*79.0, a*49.0, a*21.0, a*12.0, a*8.0, a*6.0, a*5.0, a*4.5, a*4.0, a*3.75, a*3.5, a*3.2, a*2.0, a*1.7};
    int number_buckets = 20;

    //for this simple version the flux distribution is the same - so a lot of this will seem pointless (But I want to future-proof myself a little bit)
    NeutrinoFlux *flux = PyObject_New(NeutrinoFlux, &NeutrinoFluxPy);
    
    flux->number_bunches = extraction->bunches;
    flux->energy_distribution = malloc(extraction->bunches*sizeof(*flux->energy_distribution));
    int i, j;
    //loop of distribution, assigning memory and filling it with data
    for(i = 0; i < extraction->bunches; i++){
        flux->energy_distribution[i] = Distribution_alloc(number_buckets);

        for(j = 0; j < number_buckets; j++){
            flux->energy_distribution[i]->buckets_center[j] = bucket_center[j];
            flux->energy_distribution[i]->buckets_value[j] = buckets_value[j];
        }
    }
    
    return flux;
}


PyObject* numi_no_transverse_py(PyObject *self, PyObject *args)
{
    Extraction* extraction;
    if(!PyArg_ParseTuple(args, "O!", &ExtractionPy, &extraction)){
        return NULL;
    }

    NeutrinoFlux* flux = numi_no_transverse(extraction);
    return (PyObject *)flux;
}