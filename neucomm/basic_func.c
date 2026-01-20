#include "common.h"
#include "basic_func.h"

typedef struct simple_minerva_tuple{
    double* array;
    long size;
} simple_minerva_tuple;

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