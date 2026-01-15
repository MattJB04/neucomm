#include <python3.14/Python.h>
//Struct for a simple extraction. Might add more details later. For now it just has protons per bunch, rise time and fall time. C version.
typedef struct Extraction;
//Struct for a simple extraction. Might add more details later. For now it just has protons per bunch, rise time and fall time. Python version.
PyTypeObject ExtractionPy;
//Needs an initialliser for reasons
void ExtractionPy_init(PyTypeObject* self);