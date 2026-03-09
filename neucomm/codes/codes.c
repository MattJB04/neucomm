#include "codes.h"
#include <string.h>
/*-------------------*/
/* Utility Functions */
/*-------------------*/

//Shift a byte and a bit to the end
inline u8 shift_in(u8 x, u8 bit){
    return (x << 1) | (bit & 1);
}

//Calculate the parity of a byte:
inline u8 parity(u8 x){
    return __builtin_parity(x);
}

//Calculate the hamming distance of two bytes:
inline int hamming(u8 a, u8 b){
    return __builtin_popcount(a ^ b);
}

//Takes in a python list of binary, and returns a c array of bytes. allocates the array bytes, and expects the user to deallocate
u8 *py_bin_to_byte(PyObject* list, size_t list_size, size_t* bytes_size) {
    if(list_size % 8 != 0){
        printf("Warning: The number of bits are not divisable by 8. This is undefined behaviour\n");
    }

    *bytes_size = list_size/8;
    u8* bytes = (u8*)calloc(*bytes_size, sizeof(u8));
    size_t i, j;
    for(i = 0; i < *bytes_size; i++){
        for(j = 0; j < 8; j++){
            bytes[i] = shift_in(bytes[i], (u8)PyLong_AsLong(PyList_GetItem(list, (8*i) + j)));
        }
    }
    return bytes;
}

//Takes a python list of bytes, and returns a c array of bytes. Allows for erasures
option_8 *py_byte_to_option_8(PyObject* list, size_t list_size){
    option_8* arr = (option_8*)malloc(list_size*sizeof(option_8));

    for(size_t i = 0; i < list_size; i++){
        PyObject* item = PyList_GetItem(list, i);
        if(PyLong_Check(item)){
            arr[i].valid = 1;
            arr[i].v = PyLong_AsLong(item);
        }
        else{
            arr[i].valid = 0;
            arr[i].v = 0;
        }    
    }
    return arr;
}

option_16 *py_byte_to_option_16(PyObject* list, size_t list_size){
    option_16* arr = (option_16*)malloc(list_size*sizeof(option_16));

    for(size_t i = 0; i < list_size; i++){
        PyObject* item = PyList_GetItem(list, i);
        if(PyLong_Check(item)){
            arr[i].valid = 1;
            arr[i].v = PyLong_AsLong(item);
        }
        else{
            arr[i].valid = 0;
            arr[i].v = 0;
        }
    }
    return arr;
}

u8 *py_byte_to_u8(PyObject* list, size_t list_size){
    u8* arr = (u8*)malloc(list_size*sizeof(u8));

    for(size_t i = 0; i < list_size; i++){
        PyObject* item = PyList_GetItem(list, i);
        if(PyLong_Check(item)){
            arr[i] = PyLong_AsLong(item);
        }
        else{
            arr[i] = 0;
        }    
    }
    return arr;
}

u16 *py_byte_to_u16(PyObject* list, size_t list_size){
    u16* arr = (u16*)malloc(list_size*sizeof(u16));

    for(size_t i = 0; i < list_size; i++){
        PyObject* item = PyList_GetItem(list, i);
        if(PyLong_Check(item)){
            arr[i] = PyLong_AsLong(item);
        }
        else{
            arr[i] = 0;
        }
    }

    return arr;
}

//The inverse of the above function, 
//takes a c array of bytes, and returns a python list
PyObject* byte_to_py_bin(u8* bytes, size_t bytes_size, size_t* list_size){
    *list_size = bytes_size*8;
    PyObject* list = PyList_New(*list_size);

    size_t i;
    int j;
    for(i = 0; i < bytes_size; i++){
        for(j = 7; j>=0; j--){
            PyObject *val = Py_BuildValue("i", (bytes[i] & (1 << j)? 1 : 0));
            PyList_SET_ITEM(list, (8*i) + (7-j), val);
        }
    }   
    return list;
}

//Takes an array of bytes, and returns a python list of bytes.
PyObject* u8_to_py_byte(u8* bytes, size_t bytes_size){
    PyObject* list = PyList_New(bytes_size);
    for(size_t i = 0; i < bytes_size; i++){
        PyObject* val = Py_BuildValue("i", bytes[i]);
        PyList_SetItem(list, i, val);
    }
    return list;
}

PyObject* u16_to_py_byte(u16* bytes, size_t bytes_size){
    PyObject* list = PyList_New(bytes_size);
    for(size_t i = 0; i < bytes_size; i++){
        PyObject* val = Py_BuildValue("i", bytes[i]);
        PyList_SetItem(list, i, val);
    }
    return list;
}

PyObject* option_8_to_py_byte(option_8* bytes, size_t bytes_size){
    PyObject* list = PyList_New(bytes_size);
    for(size_t i = 0; i < bytes_size; i++){
        if(bytes[i].valid){
            PyObject* val = Py_BuildValue("i", bytes[i].v);
            PyList_SetItem(list, i, val);
        }
        else{
            PyList_SetItem(list, i, Py_None);
        }
    }
    return list;
}

PyObject* option_16_to_py_byte(option_16* bytes, size_t bytes_size){
    PyObject* list = PyList_New(bytes_size);
    for(size_t i = 0; i < bytes_size; i++){
        if(bytes[i].valid){
            PyObject* val = Py_BuildValue("i", bytes[i].v);
            PyList_SetItem(list, i, val);
        }
        else{
            PyList_SetItem(list, i, Py_None);
        }
    }
    return list;
}


PyObject* string_to_bits(PyObject* self, PyObject* args){
    u8* input;
    if(!PyArg_ParseTuple(args, "s", &input)){
        return NULL;
    }
    size_t input_length = strlen((char*)input);

    size_t list_length;
    PyObject* list = byte_to_py_bin(input, input_length, &list_length);
    return list;
}

PyObject* string_to_bytes(PyObject* self, PyObject* args){
    u8* input;
    if(!PyArg_ParseTuple(args, "s", &input)){
        return NULL;
    }
    size_t input_length = strlen((char*)input);
    PyObject* list = u8_to_py_byte(input, input_length);
    return list;
}

PyObject* bits_to_string(PyObject* self, PyObject* args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input);
    size_t parsed_length;
    u8* bytes = py_bin_to_byte(input, input_length, &parsed_length);
    
        
    char* string = (char*)malloc((parsed_length + 1)*sizeof(char));
    size_t i;
    for(i = 0; i < parsed_length; i++){
        string[i] = bytes[i];
    }
    free(bytes);
    string[parsed_length] = '\0';

    PyObject *py_string = Py_BuildValue("s", string);
    free(string);

    return py_string;
}

PyObject* bytes_to_string(PyObject* self, PyObject* args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_size = PyList_GET_SIZE(input);
    char* list = py_byte_to_u8(input, input_size);
    list = (char*)realloc(list, input_size + 1);
    list[input_size] = '\0';

    PyObject *string = Py_BuildValue("s", list);
    free(list);
    return string;
}