#include "convoluted.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;

#define INF __UINT8_MAX__
#define K 7
const u8 poly1 = 0b101011; //generator polynomial 1
const u8 poly2 = 0b101011; //generator polynomial 2
#define NUM_STATES (1 << (K - 1))

/*-------------------*/
/* Utility Functions */
/*-------------------*/

//Shift a byte and a bit to the end
static inline u8 shift_in(u8 x, u8 bit){
    return (x << 1) | (bit & 1);
}

//Calculate the parity of a byte:
static inline u8 parity(u8 x){
    return __builtin_parity(x);
}

//Calculate the hamming distance of two bytes:
static inline int hamming(u8 a, u8 b){
    return __builtin_popcount(a ^ b);
}

/*Convert array of bits to ASCII characters
returns newly allocated string*/
char *bits_to_string(const u8 *bits, size_t T) {
    size_t str_len = T / 8;
    char *str = malloc(str_len + 1);
    if (!str) return NULL;
    
    for (size_t i = 0; i < str_len; i++) {
        u8 byte = 0;
        for (int b = 0; b < 8; b++) {
            byte = (byte << 1) | bits[i * 8 + b];
        }
        str[i] = byte;
    }
    str[str_len] = '\0';
    
    return str;
}
/*defo a better way to do this*/

/*---------*/
/* Trellis */
/*---------*/

//Define the trellis structure
typedef struct {
    u8 prev_state[2];
    u8 out_bits[2];
    u8 input_bit[2];
} TrellisState;

//Build the trellis auto-magically
void build_trellis(TrellisState *trellis, u8 poly1, u8 poly2) {
    const u8 state_mask = NUM_STATES - 1;
    int trans_count[NUM_STATES] = {0};
    
    for (int curr_state = 0; curr_state < NUM_STATES; curr_state++) {
        for (int input_bit = 0; input_bit < 2; input_bit++) {
            // Simulate encoder: shift in input bit
            int next_state = ((curr_state << 1) | input_bit) & state_mask;
            int full_state = (curr_state << 1) | input_bit;
            
            // Calculate output symbols
            u8 out1 = parity(full_state & poly1);
            u8 out2 = parity(full_state & poly2);
            u8 output = (out1 << 1) | out2;
            
            // Fill in the trellis
            int idx = trans_count[next_state];
            trellis[next_state].prev_state[idx] = curr_state;
            trellis[next_state].out_bits[idx] = output;
            trellis[next_state].input_bit[idx] = input_bit;
            
            trans_count[next_state]++;
        }
    }
}

/*---------*/
/* Encoder */
/*---------*/

u8 *convolve(const u8* input, size_t len, size_t *out_len){
    const u8 mask = (1u << K) - 1;// A mask to limit state to the constraint length

    // Each input byte produces 16 encoded bits (2 output bytes)
    *out_len = 2 * len;
    u8 *output = calloc(*out_len, sizeof(u8));
    if (!output) return NULL;

    //The bits that are currently being considered
    u8 state = 0; 

    for (size_t i = 0; i < len; i++) {
        u8 out_byte = 0;

        // Process upper nibble
        for (int b = 7; b >= 4; b--) {
            u8 bit = (input[i] >> b) & 1;
            state = shift_in(state, bit) & mask;
            out_byte = shift_in(out_byte, parity(state & poly1));
            out_byte = shift_in(out_byte, parity(state & poly2));
        }
        output[2 * i] = out_byte;

        out_byte = 0;

        // Process lower nibble
        for (int b = 3; b >= 0; b--) {
            u8 bit = (input[i] >> b) & 1;
            state = shift_in(state, bit) & mask;
            out_byte = shift_in(out_byte, parity(state & poly1));
            out_byte = shift_in(out_byte, parity(state & poly2));
        }
        output[2 * i + 1] = out_byte;
    }

    return output;
}

/*---------*/
/* Decoder */
/*---------*/

//Before decoding, another utility function. 
//Each new trellis state considers 2 coded bits at a time, so I unpack the coded bytes into pairs
//of two bits. This is not very efficient. oops
static u8* unpack_bytes(const u8 *bits, size_t length, size_t *T){
    *T = length * 4;  // 4 symbols per byte.
    u8 *rx = malloc(*T);
    if (!rx) return NULL;

    size_t t = 0;
    for (size_t i = 0; i < length; i++) {
        for (int b = 6; b >= 0; b -= 2) {
            rx[t++] = (bits[i] >> b) & 0x3;
        }
    }
    return rx;
}


u8 *devolve(const u8 *bits, size_t length, size_t *decoded_len) {
    
    // Build trellis
    TrellisState trellis[NUM_STATES];
    memset(trellis, 0, sizeof(trellis));
    build_trellis(trellis, poly1, poly2);

    // Unpack encoded bytes into bit pairs
    size_t T;
    u8 *rx = unpack_bytes(bits, length, &T);
    if (!rx) return NULL;

    // Allocate Viterbi buffers
    int metric_prev[NUM_STATES];
    int metric_curr[NUM_STATES];
    
    u8 (*survivor)[NUM_STATES] = malloc(T * sizeof(*survivor));
    if (!survivor) {
        free(rx);
        return NULL;
    }

    // Initialize metrics (start from state 0)
    metric_prev[0] = 0;
    for (int s = 1; s < NUM_STATES; s++) {
        metric_prev[s] = INF;
    }

    // Viterbi forward pass
    for (size_t t = 0; t < T; t++) {
        // Initialize current metrics
        for (int s = 0; s < NUM_STATES; s++) {
            metric_curr[s] = INF;
        }

        // For each state, find best previous state
        for (int s = 0; s < NUM_STATES; s++) {
            for (int trans = 0; trans < 2; trans++) {
                int ps = trellis[s].prev_state[trans];
                u8 o = trellis[s].out_bits[trans];

                int m = metric_prev[ps] + hamming(rx[t], o);
                if (m < metric_curr[s]) {
                    metric_curr[s] = m;
                    survivor[t][s] = trans;
                }
            }
        }
        
        memcpy(metric_prev, metric_curr, sizeof(metric_prev));
    }

    // Traceback: find best final state
    int best_state = 0;
    int best_metric = metric_prev[0];
    for (int s = 1; s < NUM_STATES; s++) {
        if (metric_prev[s] < best_metric) {
            best_metric = metric_prev[s];
            best_state = s;
        }
    }

    // Allocate decoded output
    u8 *decoded = malloc(T);
    if (!decoded) {
        free(rx);
        free(survivor);
        return NULL;
    }

    // Traceback to recover input bits
    for (size_t t = T; t-- > 0;) {
        u8 trans = survivor[t][best_state];
        decoded[t] = trellis[best_state].input_bit[trans];
        best_state = trellis[best_state].prev_state[trans];
    }

    *decoded_len = T;
    
    free(rx);
    free(survivor);
    
    return decoded;
}

/*-----------------*/
/* Python wrappers */
/*-----------------*/

PyObject* encode_convolution(PyObject *self, PyObject *args){
    u8* input;
    if(!PyArg_ParseTuple(args, "s", &input)){
        return NULL;
    };
    size_t input_length = strlen((char*)input);

    size_t output_length;
    u8* output = convolve(input, input_length, &output_length);

    PyObject *list = PyList_New(output_length*8);
    size_t i;
    int j;
    for(i = 0; i < output_length; i++){
        for(j = 7; j>=0; j--){
            PyObject *val = Py_BuildValue("i", (output[i] & (1 << j)? 1 : 0));
            PyList_SET_ITEM(list, (8*i) + (7-j), val);
        }
    }

    free(output);
    return list;
}

PyObject* decode_convolution(PyObject *self, PyObject *args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input)/8;
    u8* parsed_input = calloc(input_length, sizeof(u8));

    size_t i;
    int j;
    for(i = 0; i < input_length; i++){
        for(j = 0; j < 8; j++){
            parsed_input[i] = shift_in(parsed_input[i], (u8)PyLong_AsLong(PyList_GetItem(input, (8*i) + j)));
        }
    }

    size_t output_length;
    u8* output_bits = devolve(parsed_input, input_length, &output_length);

    char* output = bits_to_string(output_bits, output_length);

    PyObject *output_string = Py_BuildValue("s", output);

    free(output);
    free(output_bits);
    free(parsed_input);
    return output_string;
}