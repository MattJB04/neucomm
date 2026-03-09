#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rs.h"


/* ================================================================
 * GF(2^8) arithmetic
 * ================================================================ */

#define GF_PRIM_8 0x11d
#define GF_MAX_8  255
#define ALPHA   2

static u8 EXP_8[512], LOG_8[256];

void gf_init_8(void)
{
    u8 x = 1;
    for (int i = 0; i < GF_MAX_8; i++) {
        EXP_8[i] = x;
        LOG_8[x]  = (u8)i;
        x = (u8)((x << 1) ^ (x & 0x80 ? (GF_PRIM_8 ^ 0x100) : 0));
    }
    for (int i = 0; i < GF_MAX_8; i++) EXP_8[i + GF_MAX_8] = EXP_8[i];
    LOG_8[0] = 0;
}

static inline u8 gmul_8(u8 a, u8 b) { return (!a || !b) ? 0 : EXP_8[(int)LOG_8[a] + LOG_8[b]]; }
static inline u8 gdiv_8(u8 a, u8 b) { return !a ? 0 : EXP_8[((int)LOG_8[a] - LOG_8[b] + GF_MAX_8) % GF_MAX_8]; }
static inline u8 gpow_8(u8 x, int e) { return !x ? 0 : EXP_8[((int)LOG_8[x] * (e % GF_MAX_8) + 2*GF_MAX_8) % GF_MAX_8]; }
static inline u8 ginv_8(u8 x)        { return EXP_8[GF_MAX_8 - LOG_8[x]]; }

/* Evaluate C(a) = c[0] + c[1]*a + c[2]*a^2 + ...  (c[0] is constant term) */
static u8 geval_8(const u8 *c, int n, u8 a)
{
    u8 r = 0;
    for (int i = n - 1; i >= 0; i--)
        r = c[i] ^ gmul_8(r, a);
    return r;
}

/* ================================================================
 * GF(2^16) arithmetic
 * ================================================================ */

#define GF_PRIM_16  0x1100B   /* x^16 + x^12 + x^3 + x + 1  (irreducible over GF(2)) */
#define GF_MAX_16   65535     /* 2^16 - 1 */
#define GF_SIZE_16  65536     /* 2^16     */

static u16 EXP_16[2 * GF_MAX_16];
static u16 LOG_16[GF_SIZE_16];

void gf_init_16(void)
{
    u16 x = 1;
    for (int i = 0; i < GF_MAX_16; i++) {
        EXP_16[i] = x;
        LOG_16[x]  = (u16)i;
        /* Multiply by α = 2: shift left 1, reduce mod primitive poly if bit 16 set */
        x = (u16)((x << 1) ^ (x & 0x8000 ? (GF_PRIM_16 ^ 0x10000) : 0));
    }
    for (int i = 0; i < GF_MAX_16; i++) EXP_16[i + GF_MAX_16] = EXP_16[i];
    LOG_16[0] = 0; /* log(0) is undefined; 0 by convention */
}

static inline u16 gmul_16(u16 a, u16 b) { return (!a || !b) ? 0 : EXP_16[(int)LOG_16[a] + LOG_16[b]];}
static inline u16 gdiv_16(u16 a, u16 b) { return !a ? 0 : EXP_16[((int)LOG_16[a] - LOG_16[b] + GF_MAX_16) % GF_MAX_16];}
static inline u16 gpow_16(u16 x, int e) { return !x ? 0 : EXP_16[((int)LOG_16[x] * (e % GF_MAX_16) + 2 * GF_MAX_16) % GF_MAX_16];}
static inline u16 ginv_16(u16 x) { return EXP_16[GF_MAX_16 - LOG_16[x]]; }

static u16 geval_16(const u16 *c, int n, u16 a)
{
    u16 r = 0;
    for (int i = n - 1; i >= 0; i--)
        r = c[i] ^ gmul_16(r, a);
    return r;
}

/* ================================================================
 * Codec parameters
 * ================================================================ */

#define K      4
#define NROOTS 2
#define N      (K + NROOTS)
#define FCR    1            /* First consecutive root exponent */

/* ================================================================
 * GF Gaussian elimination: solves A*x = b in-place over GF(2^8).
 * A is n×n, stored row-major. Solution written into b.
 * Returns 0 on success, -1 if singular.
 * ================================================================ */

static int ggauss_8(u8 *A, u8 *b, int n)
{
    for (int c = 0; c < n; c++) {
        /* Find pivot */
        int piv = -1;
        for (int r = c; r < n; r++) if (A[r*n + c]) { piv = r; break; }
        if (piv < 0) return -1;

        /* Swap rows */
        if (piv != c) {
            for (int j = 0; j < n; j++) {
                u8 t = A[c*n+j]; A[c*n+j] = A[piv*n+j]; A[piv*n+j] = t;
            }
            u8 t = b[c]; b[c] = b[piv]; b[piv] = t;
        }

        /* Scale pivot row so leading element = 1 */
        u8 inv = ginv_8(A[c*n + c]);
        for (int j = 0; j < n; j++) A[c*n+j] = gmul_8(A[c*n+j], inv);
        b[c] = gmul_8(b[c], inv);

        /* Eliminate this column from all other rows */
        for (int r = 0; r < n; r++) {
            if (r == c || !A[r*n + c]) continue;
            u8 f = A[r*n + c];
            for (int j = 0; j < n; j++) A[r*n+j] ^= gmul_8(f, A[c*n+j]);
            b[r] ^= gmul_8(f, b[c]);
        }
    }
    return 0;
}

static int ggauss_16(u16 *A, u16 *b, int n)
{
    for (int c = 0; c < n; c++) {
        /* Find pivot */
        int piv = -1;
        for (int r = c; r < n; r++) if (A[r*n + c]) { piv = r; break; }
        if (piv < 0) return -1;

        /* Swap rows */
        if (piv != c) {
            for (int j = 0; j < n; j++) {
                u16 t = A[c*n+j]; A[c*n+j] = A[piv*n+j]; A[piv*n+j] = t;
            }
            u16 t = b[c]; b[c] = b[piv]; b[piv] = t;
        }

        /* Scale pivot row so leading element = 1 */
        u16 inv = ginv_16(A[c*n + c]);
        for (int j = 0; j < n; j++) A[c*n+j] = gmul_16(A[c*n+j], inv);
        b[c] = gmul_16(b[c], inv);

        /* Eliminate this column from all other rows */
        for (int r = 0; r < n; r++) {
            if (r == c || !A[r*n + c]) continue;
            u16 f = A[r*n + c];
            for (int j = 0; j < n; j++) A[r*n+j] ^= gmul_16(f, A[c*n+j]);
            b[r] ^= gmul_16(f, b[c]);
        }
    }
    return 0;
}

/* ================================================================
 * Encoder
 *
 * We need parity p[0..NROOTS-1] such that:
 *   C(α^(FCR+s)) = 0  for s = 0..NROOTS-1
 * where C(x) = msg[0] + ... + msg[K-1]*x^(K-1) + p[0]*x^K + ... + p[NROOTS-1]*x^(N-1)
 *
 * Rearranging:
 *   sum_{j=0}^{NROOTS-1} p[j] * α^((FCR+s)*(K+j))  =  msg(α^(FCR+s))
 * This gives a NROOTS×NROOTS Vandermonde-like linear system.
 *
 * Returns heap-allocated codeword of length N. Caller must free().
 * ================================================================ */

u8 *rs_encode_8(const u8 *msg)
{
    u8 *cw = calloc(N, 1);
    if (!cw) return NULL;
    memcpy(cw, msg, K);

    u8 A[NROOTS * NROOTS], rhs[NROOTS];

    for (int s = 0; s < NROOTS; s++) {
        /* rhs[s] = msg(α^(FCR+s)) */
        rhs[s] = geval_8(msg, K, gpow_8(ALPHA, FCR + s));
        /* A[s][j] = α^((FCR+s)*(K+j)) */
        for (int j = 0; j < NROOTS; j++)
            A[s * NROOTS + j] = gpow_8(ALPHA, (FCR + s) * (K + j));
    }

    if (ggauss_8(A, rhs, NROOTS) != 0) {
        fprintf(stderr, "Encoder: singular system (should not happen)\n");
        free(cw);
        return NULL;
    }

    for (int j = 0; j < NROOTS; j++)
        cw[K + j] = rhs[j];

    return cw;
}

u16 *rs_encode_16(const u16 *msg)
{
    u16 *cw = calloc(N, sizeof(u16));
    if (!cw) return NULL;
    memcpy(cw, msg, K * sizeof(u16));

    u16 A[NROOTS * NROOTS], rhs[NROOTS];

    for (int s = 0; s < NROOTS; s++) {
        /* rhs[s] = msg(α^(FCR+s)) */
        rhs[s] = geval_16(msg, K, gpow_16(ALPHA, FCR + s));
        /* A[s][j] = α^((FCR+s)*(K+j)) */
        for (int j = 0; j < NROOTS; j++)
            A[s * NROOTS + j] = gpow_16(ALPHA, (FCR + s) * (K + j));
    }

    if (ggauss_16(A, rhs, NROOTS) != 0) {
        fprintf(stderr, "Encoder: singular system (should not happen)\n");
        free(cw);
        return NULL;
    }

    for (int j = 0; j < NROOTS; j++)
        cw[K + j] = rhs[j];

    return cw;
}

/* ================================================================
 * Syndromes: S_s = C(α^(FCR+s)) for s = 0..NROOTS-1
 * Valid codeword has all-zero syndromes.
 * ================================================================ */

static void syndromes_8(const u8 *cw, u8 *s)
{
    for (int i = 0; i < NROOTS; i++)
        s[i] = geval_8(cw, N, gpow_8(ALPHA, FCR + i));
}

static int all_zero_8(const u8 *s, int n)
{
    for (int i = 0; i < n; i++) if (s[i]) return 0;
    return 1;
}

static void syndromes_16(const u16 *cw, u16 *s)
{
    for (int i = 0; i < NROOTS; i++)
        s[i] = geval_16(cw, N, gpow_16(ALPHA, FCR + i));
}

static int all_zero_16(const u16 *s, int n)
{
    for (int i = 0; i < n; i++) if (s[i]) return 0;
    return 1;
}

/* ================================================================
 * Erasure decoder
 *
 * For each syndrome s:
 *   C(α^(FCR+s)) = 0
 *   sum_{j erased} e_j * α^((FCR+s)*epos[j])  =  sum_{i known} cw[i]*α^((FCR+s)*i)
 *   (RHS = partial syndrome = geval(partial_cw, N, α^(FCR+s)))
 *
 * This gives a ne×ne linear system for the erasure values e_j.
 *
 * coded[N]: .valid=1 means received, .valid=0 means erased.
 * At most NROOTS erasures can be corrected.
 * Returns heap-allocated decoded message[K], or NULL on failure.
 * ================================================================ */

u8 *rs_decode_erasures_8(const option_8 *coded)
{
    int ne = 0, ep[NROOTS];
    for (int i = 0; i < N; i++) {
        if (!coded[i].valid) {
            if (ne >= NROOTS) { printf("Too many erasures (max %d).\n", NROOTS); return NULL; }
            ep[ne++] = i;
        }
    }

    /* Build partial codeword (erasures set to 0) */
    u8 cw[N];
    for (int i = 0; i < N; i++) cw[i] = coded[i].valid ? coded[i].v : 0;

    if (ne == 0) {
        u8 s[NROOTS]; syndromes_8(cw, s);
        if (!all_zero_8(s, NROOTS))
            printf("Warning: non-zero syndromes with no erasures (uncorrectable error).\n");
        u8 *out = malloc(K); memcpy(out, cw, K); return out;
    }

    /* Build system: A[s][j] = α^((FCR+s)*ep[j]),  rhs[s] = partial_syndrome_s */
    u8 A[NROOTS * NROOTS], rhs[NROOTS];
    for (int s = 0; s < ne; s++) {
        rhs[s] = geval_8(cw, N, gpow_8(ALPHA, FCR + s));
        for (int j = 0; j < ne; j++)
            A[s * ne + j] = gpow_8(ALPHA, (FCR + s) * ep[j]);
    }

    if (ggauss_8(A, rhs, ne) != 0) { printf("Singular system — cannot decode.\n"); return NULL; }
    for (int j = 0; j < ne; j++) cw[ep[j]] = rhs[j];

    /* Verify */
    u8 sv[NROOTS]; syndromes_8(cw, sv);
    if (!all_zero_8(sv, NROOTS))
        printf("Warning: syndromes non-zero after erasure recovery.\n");

    u8 *out = malloc(K); memcpy(out, cw, K); return out;
}

u16 *rs_decode_erasures_16(const option_16 *coded)
{
    int ne = 0, ep[NROOTS];
    for (int i = 0; i < N; i++) {
        if (!coded[i].valid) {
            if (ne >= NROOTS) { printf("Too many erasures (max %d).\n", NROOTS); return NULL; }
            ep[ne++] = i;
        }
    }

    /* Build partial codeword (erasures set to 0) */
    u16 cw[N];
    for (int i = 0; i < N; i++) cw[i] = coded[i].valid ? coded[i].v : 0;

    if (ne == 0) {
        u16 s[NROOTS]; syndromes_16(cw, s);
        if (!all_zero_16(s, NROOTS))
            printf("Warning: non-zero syndromes with no erasures (uncorrectable error).\n");
        u16 *out = malloc(K * sizeof(u16)); memcpy(out, cw, K * sizeof(u16)); return out;
    }

    /* Build system: A[s][j] = α^((FCR+s)*ep[j]),  rhs[s] = partial_syndrome_s */
    u16 A[NROOTS * NROOTS], rhs[NROOTS];
    for (int s = 0; s < ne; s++) {
        rhs[s] = geval_16(cw, N, gpow_16(ALPHA, FCR + s));
        for (int j = 0; j < ne; j++)
            A[s * ne + j] = gpow_16(ALPHA, (FCR + s) * ep[j]);
    }

    if (ggauss_16(A, rhs, ne) != 0) { printf("Singular system — cannot decode.\n"); return NULL; }
    for (int j = 0; j < ne; j++) cw[ep[j]] = rhs[j];

    /* Verify */
    u16 sv[NROOTS]; syndromes_16(cw, sv);
    if (!all_zero_16(sv, NROOTS))
        printf("Warning: syndromes non-zero after erasure recovery.\n");

    u16 *out = malloc(K * sizeof(u16)); memcpy(out, cw, K * sizeof(u16)); return out;
}

/* ================================================================
 * Error decoder  (corrects 1 error when NROOTS = 2)
 *
 * Single error at position p with magnitude e:
 *   S0 = e * α^(FCR*p)
 *   S1 = e * α^((FCR+1)*p)
 *   => α^p = S1/S0   =>   p = log_α(S1/S0)
 *   => e   = S0 / α^(FCR*p)
 * ================================================================ */

u8 *rs_decode_errors_8(const u8 *cw_in)
{
    u8 cw[N]; memcpy(cw, cw_in, N);
    u8 s[NROOTS]; syndromes_8(cw, s);

    if (all_zero_8(s, NROOTS)) {
        u8 *out = malloc(K); memcpy(out, cw, K); return out;
    }

    if (!s[0]) {
        printf("Cannot locate error (S0=0, more than 1 error?).\n");
        return NULL;
    }

    u8 alpha_p = gdiv_8(s[1], s[0]);   /* = α^p */
    int p = LOG_8[alpha_p];

    if (p >= N) { printf("Error location %d out of codeword range.\n", p); return NULL; }

    u8 e = gdiv_8(s[0], gpow_8(ALPHA, FCR * p));
    cw[p] ^= e;

    syndromes_8(cw, s);
    if (!all_zero_8(s, NROOTS)) {
        printf("Correction failed — likely more than 1 error.\n");
        return NULL;
    }

    u8 *out = malloc(K); memcpy(out, cw, K); return out;
}

u16 *rs_decode_errors_16(const u16 *cw_in)
{
    u16 cw[N]; memcpy(cw, cw_in, N * sizeof(u16));
    u16 s[NROOTS]; syndromes_16(cw, s);

    if (all_zero_16(s, NROOTS)) {
        u16 *out = malloc(K * sizeof(u16)); memcpy(out, cw, K * sizeof(u16)); return out;
    }

    if (!s[0]) {
        printf("Cannot locate error (S0=0, more than 1 error?).\n");
        return NULL;
    }

    u16 alpha_p = gdiv_16(s[1], s[0]);   /* = α^p */
    int p = LOG_16[alpha_p];

    if (p >= N) { printf("Error location %d out of codeword range.\n", p); return NULL; }

    u16 e = gdiv_16(s[0], gpow_16(ALPHA, FCR * p));
    cw[p] ^= e;

    syndromes_16(cw, s);
    if (!all_zero_16(s, NROOTS)) {
        printf("Correction failed — likely more than 1 error.\n");
        return NULL;
    }

    u16 *out = malloc(K * sizeof(u16)); memcpy(out, cw, K * sizeof(u16)); return out;
}

/*----------------*/
/* Python Section */
/*----------------*/

//right, how do I actually go about using this?

//So I get sent a list of bits. This only works for bytes, so I will have to turn that into bytes. Like last time, should I pad the end with 0's if the message
//isnt long enough?

//Want to be able to handle more then multiple of 4 bytes

//All I want is a list of bits
PyObject* encode_rs_8(PyObject *self, PyObject *args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input);
    u8* parsed_input = py_byte_to_u8(input, input_length);

    //Need to check that we have 4 bytes. If not, pad the end with 0's. again (theres a lot of 0's with this one)

    unsigned int n_padding = input_length % 4;
    if(n_padding != 0){
        printf("Warning: the Reed Soloman encoder is designed for a multiple of 4 bytes\n");
        printf("Padding the end of the message with %d bytes of 0", n_padding);
        parsed_input = (u8*)realloc(parsed_input, input_length+n_padding);
        for(size_t i = 0; i < n_padding; i++){
            parsed_input[input_length + i] = 0;
        }
        input_length += n_padding;
    }

    size_t coded_size = (input_length/4)*6;
    //coded array
    u8* coded_arr = (u8*)malloc(coded_size*sizeof(u8));
    //array holding the current 4 bytes we are considering
    u8* temp_arr = (u8*)malloc(4*sizeof(u8));
    //loop over every 4 bytes
    for(size_t i = 0; i < input_length/4; i++){
        //populate the temp arr
        for(size_t j = 0; j < 4; j++){
            temp_arr[j] = parsed_input[4*i + j];
        }

        //code it
        u8* temp_code = rs_encode_8(temp_arr);
        //check there wasnt an error
        if(temp_code == NULL){
            return NULL;
        }
        //fill in the coded array
        for(size_t j = 0; j < 6; j++){
            coded_arr[6*i + j] = temp_code[j];
        }
        free(temp_code);
    }
    free(parsed_input);
    free(temp_arr);

    //Should be able to convert into a list and return it
    PyObject* output = u8_to_py_byte(coded_arr, coded_size);
    free(coded_arr);

    return output;
}

//Some what trickier as I need to have 6 bits for a sensible decoding.
//I suppose I will just return an error
PyObject* decode_rs_erasures_8(PyObject *self, PyObject *args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input);
    option_8* parsed_input = py_byte_to_option_8(input, input_length);
    //If not a multiple of 6, Im thinking just ignore the last part bytes
    if(input_length % 6 != 0){
        printf("Warning! Expected a multiple of 6 bytes\n");
        printf("The last %lu bytes will be ignored\n", input_length % 6);
        input_length = (input_length/6) * 6;//if not divisible by 6, then parsed_input_length/6 should truncate to 0 (as its an int type)
        parsed_input = (option_8*)realloc(parsed_input, input_length*sizeof(option_8));
    }

    size_t uncoded_size = (input_length/6)*4;
    option_8* temp_arr = (option_8*)malloc(6 * sizeof(option_8));
    u8* uncoded_arr = (u8*)malloc(uncoded_size*sizeof(u8));
    //Now loop through groups of 6 bytes
    for(size_t i = 0; i < input_length/6; i++){
        //populate the temp arr
        for(size_t j = 0; j < 6; j++){
            temp_arr[j] = parsed_input[6*i + j];
        }
        //uncode it
        u8* uncode_temp = rs_decode_erasures_8(temp_arr);
        if(uncode_temp == NULL){
            return NULL;
        }
        //populate uncoded array
        for(size_t j = 0; j < 4; j++){
            uncoded_arr[4*i + j] = uncode_temp[j];
        }
        free(uncode_temp);
    }
    free(temp_arr);
    free(parsed_input);

    PyObject* parsed_output = u8_to_py_byte(uncoded_arr, uncoded_size);
    free(uncoded_arr);

    return parsed_output;
}

PyObject* encode_rs_16(PyObject *self, PyObject *args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input);
    u16* parsed_input = py_byte_to_u16(input, input_length);

    //Need to check that we have 4 bytes. If not, pad the end with 0's. again (theres a lot of 0's with this one)

    unsigned int n_padding = input_length % 4;
    if(n_padding != 0){
        printf("Warning: the Reed Soloman encoder is designed for a multiple of 4 bytes\n");
        printf("Padding the end of the message with %d bytes of 0", n_padding);
        parsed_input = (u16*)realloc(parsed_input, input_length+n_padding);
        for(size_t i = 0; i < n_padding; i++){
            parsed_input[input_length + i] = 0;
        }
        input_length += n_padding;
    }

    size_t coded_size = (input_length/4)*6;
    //coded array
    u16* coded_arr = (u16*)malloc(coded_size*sizeof(u16));
    //array holding the current 4 bytes we are considering
    u16* temp_arr = (u16*)malloc(4*sizeof(u16));
    //loop over every 4 bytes
    for(size_t i = 0; i < input_length/4; i++){
        //populate the temp arr
        for(size_t j = 0; j < 4; j++){
            temp_arr[j] = parsed_input[4*i + j];
        }

        //code it
        u16* temp_code = rs_encode_16(temp_arr);
        //check there wasnt an error
        if(temp_code == NULL){
            return NULL;
        }
        //fill in the coded array
        for(size_t j = 0; j < 6; j++){
            coded_arr[6*i + j] = temp_code[j];
        }
        free(temp_code);
    }
    free(parsed_input);
    free(temp_arr);

    //Should be able to convert into a list and return it
    PyObject* output = u16_to_py_byte(coded_arr, coded_size);
    free(coded_arr);

    return output;
}

PyObject* decode_rs_erasures_16(PyObject *self, PyObject *args){
    PyObject* input;
    if(!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)){
        return NULL;
    }
    size_t input_length = PyList_GET_SIZE(input);
    option_16* parsed_input = py_byte_to_option_16(input, input_length);
    //If not a multiple of 6, Im thinking just ignore the last part bytes
    if(input_length % 6 != 0){
        printf("Warning! Expected a multiple of 6 bytes\n");
        printf("The last %lu bytes will be ignored\n", input_length % 6);
        input_length = (input_length/6) * 6;//if not divisible by 6, then parsed_input_length/6 should truncate to 0 (as its an int type)
        parsed_input = (option_16*)realloc(parsed_input, input_length*sizeof(option_16));
    }

    size_t uncoded_size = (input_length/6)*4;
    option_16* temp_arr = (option_16*)malloc(6 * sizeof(option_16));
    u16* uncoded_arr = (u16*)malloc(uncoded_size*sizeof(u16));
    //Now loop through groups of 6 bytes
    for(size_t i = 0; i < input_length/6; i++){
        //populate the temp arr
        for(size_t j = 0; j < 6; j++){
            temp_arr[j] = parsed_input[6*i + j];
        }
        //uncode it
        u16* uncode_temp = rs_decode_erasures_16(temp_arr);
        if(uncode_temp == NULL){
            return NULL;
        }
        //populate uncoded array
        for(size_t j = 0; j < 4; j++){
            uncoded_arr[4*i + j] = uncode_temp[j];
        }
        free(uncode_temp);
    }
    free(temp_arr);
    free(parsed_input);

    PyObject* parsed_output = u16_to_py_byte(uncoded_arr, uncoded_size);
    free(uncoded_arr);

    return parsed_output;
}