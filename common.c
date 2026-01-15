#include <python3.14/Python.h>
//Struct for a simple extraction. Might add more details later. For now it just has protons per bunch, rise time and fall time
typedef struct {
    PyObject_HEAD
    double ppb; //protons per bunch
    long bunches; //Number of bunches
    double bunch_spacing; //Bunch spacing
    double rise_time; //rise time for the kicker magnets
    double fall_time; //fall time for the kicker magnets (might be unnecassary?)
} Extraction;

//Deallocator: 
static void MyObject_dealloc(Extraction *self){
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
    self->tp_dealloc = (destructor)MyObject_dealloc;
    self->tp_flags = Py_TPFLAGS_DEFAULT;
    self->tp_doc =
    "A (somewhat simple) datatype describing 1 proton extraction. "
    "Usually passed to an accelerator to generate a neutrino flux";
    self->tp_methods = Extraction_methods;
    self->tp_init = (initproc)Extraction_init;
    self->tp_new = Extraction_new;
}