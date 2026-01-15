#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "basic_func.h"

/*A very basic model. Takes in a bunch size, and applies the MINERvA experimental event rate (0.81 per 2.25e13 POT). Returns the number of detected neutrinos*/
int simple_minerva(double bunch_size){
    //random number first
    srand(time(NULL));
    double threshold = rand()/(double)RAND_MAX;
    int value = 0;
    double expectation = 0.81 * ((bunch_size)/2.25e13);
    double log_p;
    double p;
    double log_sum = 0;
    //Then loop through possin value until the threshold is met:
    while(1){
        log_p = ((double)value*log(expectation)) - expectation + log_sum;
        p = exp(log_p);
        if(p > threshold)
        {
            return value;
        }
        else
        {
            threshold -=  p;
            value++;
        }
        log_sum -= log(value);
    }
}
