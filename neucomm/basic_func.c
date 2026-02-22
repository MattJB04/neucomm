#include "common.h"
#include "basic_func.h"

typedef struct simple_minerva_tuple{
    double* array;
    long size;
} simple_minerva_tuple;

typedef struct distribution{
    double* values;
    double* bucket_bounds;
    size_t num_of_values;
} distribution;

static const double a = 10e-42;
static double buckets_bounds[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static double buckets_values[] = {a*0.4, a*0.62, a*0.8, a*0.82, a*0.88, a*0.96, a*0.98, a*0.98, a*0.9, a*0.8, a*0.79, a*0.78, a*0.77, a*0.76, 
                                a*0.75, a*0.74, a*0.73, a*0.72, a*0.71, a*0.70, a*0.69, a*0.68, a*0.67, a*0.66, a*0.65, a*0.64, a*0.63};


static double water_muon_expectation(distribution* neutrino_flux, distribution* cross_section, double distance){
    //First some definitions.
    //Water number density
    double n_water = 3.34e28;
    //muon average lifetime in the rest frame
    double t_muon = 2.2e-6;
    //muon mass (all energies should be in GeV)
    double muon_mass = 0.10566;
    //speed of light
    double c = 299792458;
    //Is that it?

    //Threshold energy - for now I will just use the muon mass
    double threshold = muon_mass;

    //The final expectation value
    double average_muon = 0;
    //Now need to loop over the energy values of the neutrino flux distribution
    for(size_t energy_index = 0; energy_index < neutrino_flux->num_of_values; energy_index++){
        //need to grab the energy value I am currently considering. Take the average of the bucket values
        double energy = (neutrino_flux->bucket_bounds[energy_index] + neutrino_flux->bucket_bounds[energy_index + 1])*0.5;
        //check that the threshold energy is reached
        if(energy < threshold){
            continue;
        }
        //The number of neutrinos in this flux
        double number_neutrinos = neutrino_flux->values[energy_index];
        
        double cross_section_value = 0;
        //Need to find the cross-section for the energy we are considering:
        for(size_t i = 0; i < cross_section->num_of_values; i++){
            //cross section energy we are considering
            double cross_section_energy = (cross_section->bucket_bounds[i] + cross_section->bucket_bounds[i+1])/2;
            //If the next section is beyond bounds, end the loop
            if(cross_section_energy > energy){
                break;
            }

            //then add the cross_section for this step
            double energy_difference = cross_section->bucket_bounds[i+1] - cross_section->bucket_bounds[i];
            cross_section_value += cross_section->values[i]*energy_difference;

        }//cross section loop

        //Gamma factor
        double gamma = (energy - muon_mass)/muon_mass;

        // //check the values
        // printf("cross section: %e\n", cross_section_value);
        // printf("n_water: %e\n", n_water);
        // printf("distance: %e\n", distance);
        // printf("t_muon: %e\n", t_muon);
        // printf("gamma: %e\n", gamma);

        double temp1 = -1*cross_section_value*n_water*distance;
        double temp2 = cross_section_value*n_water*t_muon*c*sqrt(gamma*gamma - 1);
        double temp3 = number_neutrinos*exp(temp1)*(exp(temp2) - 1);

        average_muon += temp3;
    }//energy loop
    return average_muon;
}//That wasn't too bad!

/*A very basic model. Takes in a bunch size, and applies the MINERvA experimental event rate (0.81 per 2.25e13 POT). Returns the number of detected neutrinos*/
static struct simple_minerva_tuple simple_minerva(Extraction *extraction){
    double expectation = 0.81 * ((extraction->ppb)/2.25e13); //Expectation for a poisson distribution. Based on MINERvA averge event rate
    int* detection_arr = (int*)malloc(extraction->bunches*sizeof(int));//An array that holds the number of detected neutrinos per pulse
    long i; //loop counter
    long no_detections = 0; //keep track of how many extractions 
    //could add an openmp pragma here
    for(i = 0; i < extraction->bunches; i++){
        detection_arr[i] = poisson(expectation);
        if(detection_arr[i] > 0){
            no_detections++;
        }
    }
    double* time_array = (double*)malloc(no_detections*sizeof(double)); //The times that a detection occured.
    double travel_time = (1.035e3/c);
    double time_tracker = extraction->rise_time + travel_time;
    long array_tracker = 0;
    for(i = 0; i < extraction->bunches; i++){
        if(detection_arr[i] > 0){
            time_array[array_tracker] = time_tracker + i*extraction->bunch_spacing;
            array_tracker ++;
        }
    }
    free(detection_arr);
    struct simple_minerva_tuple temp = {time_array, no_detections};
    return temp;
}

PyObject* simple_minerva_py(PyObject *self, PyObject *args)
{
    Extraction* extraction;
    if(!PyArg_ParseTuple(args, "O!", &ExtractionPy, &extraction)){
        return NULL;
    }

    simple_minerva_tuple temp = simple_minerva(extraction);
    PyObject *list = PyList_New(temp.size);
    long i;
    for(i = 0; i < temp.size; i++){
        PyObject *val = PyFloat_FromDouble(temp.array[i]);
        PyList_SET_ITEM(list, i, val);
    }

    free(temp.array);
    return list;
}