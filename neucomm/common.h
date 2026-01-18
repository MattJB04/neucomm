#include <python3.14/Python.h>
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

extern const double c; //speed of light