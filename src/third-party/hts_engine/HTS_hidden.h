/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2015  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef HTS_HIDDEN_H
#define HTS_HIDDEN_H

#ifdef __cplusplus
#define HTS_HIDDEN_H_START extern "C" {
#define HTS_HIDDEN_H_END   }
#else
#define HTS_HIDDEN_H_START
#define HTS_HIDDEN_H_END
#endif                          /* __CPLUSPLUS */

HTS_HIDDEN_H_START;

/* hts_engine libraries */
#include "HTS_engine.h"

/* common ---------------------------------------------------------- */

#define HTS_MAXBUFLEN 1024

#if !defined(WORDS_BIGENDIAN) && !defined(WORDS_LITTLEENDIAN)
#define WORDS_LITTLEENDIAN
#endif                          /* !WORDS_BIGENDIAN && !WORDS_LITTLEENDIAN */
#if defined(WORDS_BIGENDIAN) && defined(WORDS_LITTLEENDIAN)
#undef WORDS_BIGENDIAN
#endif                          /* WORDS_BIGENDIAN && WORDS_LITTLEENDIAN */

#define MAX_F0    20000.0
#define MIN_F0    20.0
#define MAX_LF0   9.9034875525361280454891979401956     /* log(20000.0) */
#define MIN_LF0   2.9957322735539909934352235761425     /* log(20.0) */
#define HALF_TONE 0.05776226504666210911810267678818    /* log(2.0) / 12.0 */
#define DB        0.11512925464970228420089957273422    /* log(10.0) / 20.0 */

/* misc ------------------------------------------------------------ */

typedef struct _HTS_File {
   unsigned char type;
   void *pointer;
} HTS_File;

/* HTS_fopen: wrapper for fopen */
HTS_File *HTS_fopen_from_fn(const char *name, const char *opt);

/* HTS_fopen_from_fp: wrapper for fopen */
HTS_File *HTS_fopen_from_fp(HTS_File * fp, size_t size);

/* HTS_fopen_from_data: wrapper for fopen */
HTS_File *HTS_fopen_from_data(void *data, size_t size);

/* HTS_fclose: wrapper for fclose */
void HTS_fclose(HTS_File * fp);

/* HTS_fgetc: wrapper for fgetc */
int HTS_fgetc(HTS_File * fp);

/* HTS_feof: wrapper for feof */
int HTS_feof(HTS_File * fp);

/* HTS_fseek: wrapper for fseek */
int HTS_fseek(HTS_File * fp, long offset, int origin);

/* HTS_ftell: wrapper for ftell */
size_t HTS_ftell(HTS_File * fp);

/* HTS_fread_big_endian: fread with byteswap */
size_t HTS_fread_big_endian(void *buf, size_t size, size_t n, HTS_File * fp);

/* HTS_fread_little_endian: fread with byteswap */
size_t HTS_fread_little_endian(void *buf, size_t size, size_t n, HTS_File * fp);

/* HTS_fwrite_little_endian: fwrite with byteswap */
size_t HTS_fwrite_little_endian(const void *buf, size_t size, size_t n, FILE * fp);

/* HTS_get_pattern_token: get pattern token (single/double quote can be used) */
HTS_Boolean HTS_get_pattern_token(HTS_File * fp, char *buff);

/* HTS_get_token: get token from file pointer (separators are space,tab,line break) */
HTS_Boolean HTS_get_token_from_fp(HTS_File * fp, char *buff);

/* HTS_get_token: get token from file pointer with specified separator */
HTS_Boolean HTS_get_token_from_fp_with_separator(HTS_File * fp, char *buff, char separator);

/* HTS_get_token_from_string: get token from string (separator are space,tab,line break) */
HTS_Boolean HTS_get_token_from_string(const char *string, size_t * index, char *buff);

/* HTS_get_token_from_string_with_separator: get token from string with specified separator */
HTS_Boolean HTS_get_token_from_string_with_separator(const char *str, size_t * index, char *buff, char separator);

/* HTS_calloc: wrapper for calloc */
void *HTS_calloc(const size_t num, const size_t size);

/* HTS_strdup: wrapper for strdup */
char *HTS_strdup(const char *string);

/* HTS_calloc_matrix: allocate double matrix */
double **HTS_alloc_matrix(size_t x, size_t y);

/* HTS_free_matrix: free double matrix */
void HTS_free_matrix(double **p, size_t x);

/* HTS_Free: wrapper for free */
void HTS_free(void *p);

/* HTS_error: output error message */
void HTS_error(int error, const char *message, ...);

/* audio ----------------------------------------------------------- */

/* HTS_Audio_initialize: initialize audio */
void HTS_Audio_initialize(HTS_Audio * audio);

/* HTS_Audio_set_parameter: set parameters for audio */
void HTS_Audio_set_parameter(HTS_Audio * audio, size_t sampling_frequency, size_t max_buff_size);

/* HTS_Audio_write: send data to audio */
void HTS_Audio_write(HTS_Audio * audio, short data);

/* HTS_Audio_flush: flush remain data */
void HTS_Audio_flush(HTS_Audio * audio);

/* HTS_Audio_clear: free audio */
void HTS_Audio_clear(HTS_Audio * audio);

/* model ----------------------------------------------------------- */

/* HTS_ModelSet_initialize: initialize model set */
void HTS_ModelSet_initialize(HTS_ModelSet * ms);

/* HTS_ModelSet_load: load HTS voices */
HTS_Boolean HTS_ModelSet_load(HTS_ModelSet * ms, char **voices, size_t num_voices);

/* HTS_ModelSet_get_sampling_frequency: get sampling frequency of HTS voices */
size_t HTS_ModelSet_get_sampling_frequency(HTS_ModelSet * ms);

/* HTS_ModelSet_get_fperiod: get frame period of HTS voices */
size_t HTS_ModelSet_get_fperiod(HTS_ModelSet * ms);

/* HTS_ModelSet_get_fperiod: get stream option */
const char *HTS_ModelSet_get_option(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_get_gv_flag: get GV flag */
HTS_Boolean HTS_ModelSet_get_gv_flag(HTS_ModelSet * ms, const char *string);

/* HTS_ModelSet_get_nstate: get number of state */
size_t HTS_ModelSet_get_nstate(HTS_ModelSet * ms);

/* HTS_Engine_get_fullcontext_label_format: get full-context label format */
const char *HTS_ModelSet_get_fullcontext_label_format(HTS_ModelSet * ms);

/* HTS_Engine_get_fullcontext_label_version: get full-context label version */
const char *HTS_ModelSet_get_fullcontext_label_version(HTS_ModelSet * ms);

/* HTS_ModelSet_get_nstream: get number of stream */
size_t HTS_ModelSet_get_nstream(HTS_ModelSet * ms);

/* HTS_ModelSet_get_nvoices: get number of HTS voices */
size_t HTS_ModelSet_get_nvoices(HTS_ModelSet * ms);

/* HTS_ModelSet_get_vector_length: get vector length */
size_t HTS_ModelSet_get_vector_length(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_is_msd: get MSD flag */
HTS_Boolean HTS_ModelSet_is_msd(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_get_window_size: get dynamic window size */
size_t HTS_ModelSet_get_window_size(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS_ModelSet_get_window_left_width(HTS_ModelSet * ms, size_t stream_index, size_t window_index);

/* HTS_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS_ModelSet_get_window_right_width(HTS_ModelSet * ms, size_t stream_index, size_t window_index);

/* HTS_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_ModelSet_get_window_coefficient(HTS_ModelSet * ms, size_t stream_index, size_t window_index, size_t coefficient_index);

/* HTS_ModelSet_get_window_max_width: get max width of dynamic window */
size_t HTS_ModelSet_get_window_max_width(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_use_gv: get GV flag */
HTS_Boolean HTS_ModelSet_use_gv(HTS_ModelSet * ms, size_t stream_index);

/* HTS_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS_ModelSet_get_duration_index(HTS_ModelSet * ms, size_t voice_index, const char *string, size_t * tree_index, size_t * pdf_index);

/* HTS_ModelSet_get_duration: get duration using interpolation weight */
void HTS_ModelSet_get_duration(HTS_ModelSet * ms, const char *string, const double *iw, double *mean, double *vari);

/* HTS_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS_ModelSet_get_parameter_index(HTS_ModelSet * ms, size_t voice_index, size_t stream_index, size_t state_index, const char *string, size_t * tree_index, size_t * pdf_index);

/* HTS_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS_ModelSet_get_parameter(HTS_ModelSet * ms, size_t stream_index, size_t state_index, const char *string, const double *const *iw, double *mean, double *vari, double *msd);

void HTS_ModelSet_get_gv_index(HTS_ModelSet * ms, size_t voice_index, size_t stream_index, const char *string, size_t * tree_index, size_t * pdf_index);

/* HTS_ModelSet_get_gv: get GV using interpolation weight */
void HTS_ModelSet_get_gv(HTS_ModelSet * ms, size_t stream_index, const char *string, const double *const *iw, double *mean, double *vari);

/* HTS_ModelSet_clear: free model set */
void HTS_ModelSet_clear(HTS_ModelSet * ms);

/* label ----------------------------------------------------------- */

/* HTS_Label_initialize: initialize label */
void HTS_Label_initialize(HTS_Label * label);

/* HTS_Label_load_from_fn: load label from file name */
void HTS_Label_load_from_fn(HTS_Label * label, size_t sampling_rate, size_t fperiod, const char *fn);

/* HTS_Label_load_from_strings: load label list from string list */
void HTS_Label_load_from_strings(HTS_Label * label, size_t sampling_rate, size_t fperiod, char **lines, size_t num_lines);

/* HTS_Label_get_size: get number of label string */
size_t HTS_Label_get_size(HTS_Label * label);

/* HTS_Label_get_string: get label string */
const char *HTS_Label_get_string(HTS_Label * label, size_t index);

/* HTS_Label_get_start_frame: get start frame */
double HTS_Label_get_start_frame(HTS_Label * label, size_t index);

/* HTS_Label_get_end_frame: get end frame */
double HTS_Label_get_end_frame(HTS_Label * label, size_t index);

/* HTS_Label_clear: free label */
void HTS_Label_clear(HTS_Label * label);

/* sstream --------------------------------------------------------- */

/* HTS_SStreamSet_initialize: initialize state stream set */
void HTS_SStreamSet_initialize(HTS_SStreamSet * sss);

/* HTS_SStreamSet_create: parse label and determine state duration */
HTS_Boolean HTS_SStreamSet_create(HTS_SStreamSet * sss, HTS_ModelSet * ms, HTS_Label * label, HTS_Boolean phoneme_alignment_flag, double speed, double *duration_iw, double **parameter_iw, double **gv_iw);

/* HTS_SStreamSet_get_nstream: get number of stream */
size_t HTS_SStreamSet_get_nstream(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_vector_length: get vector length */
size_t HTS_SStreamSet_get_vector_length(HTS_SStreamSet * sss, size_t stream_index);

/* HTS_SStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_SStreamSet_is_msd(HTS_SStreamSet * sss, size_t stream_index);

/* HTS_SStreamSet_get_total_state: get total number of state */
size_t HTS_SStreamSet_get_total_state(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_total_frame: get total number of frame */
size_t HTS_SStreamSet_get_total_frame(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_msd: get msd parameter */
double HTS_SStreamSet_get_msd(HTS_SStreamSet * sss, size_t stream_index, size_t state_index);

/* HTS_SStreamSet_window_size: get dynamic window size */
size_t HTS_SStreamSet_get_window_size(HTS_SStreamSet * sss, size_t stream_index);

/* HTS_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS_SStreamSet_get_window_left_width(HTS_SStreamSet * sss, size_t stream_index, size_t window_index);

/* HTS_SStreamSet_get_window_right_width: get right width of dynamic window */
int HTS_SStreamSet_get_window_right_width(HTS_SStreamSet * sss, size_t stream_index, size_t window_index);

/* HTS_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_SStreamSet_get_window_coefficient(HTS_SStreamSet * sss, size_t stream_index, size_t window_index, int coefficient_index);

/* HTS_SStreamSet_get_window_max_width: get max width of dynamic window */
size_t HTS_SStreamSet_get_window_max_width(HTS_SStreamSet * sss, size_t stream_index);

/* HTS_SStreamSet_use_gv: get GV flag */
HTS_Boolean HTS_SStreamSet_use_gv(HTS_SStreamSet * sss, size_t stream_index);

/* HTS_SStreamSet_get_duration: get state duration */
size_t HTS_SStreamSet_get_duration(HTS_SStreamSet * sss, size_t state_index);

/* HTS_SStreamSet_get_mean: get mean parameter */
double HTS_SStreamSet_get_mean(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index);

/* HTS_SStreamSet_set_mean: set mean parameter */
void HTS_SStreamSet_set_mean(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f);

/* HTS_SStreamSet_get_vari: get variance parameter */
double HTS_SStreamSet_get_vari(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index);

/* HTS_SStreamSet_set_vari: set variance parameter */
void HTS_SStreamSet_set_vari(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, size_t vector_index, double f);

/* HTS_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS_SStreamSet_get_gv_mean(HTS_SStreamSet * sss, size_t stream_index, size_t vector_index);

/* HTS_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS_SStreamSet_get_gv_vari(HTS_SStreamSet * sss, size_t stream_index, size_t vector_index);

/* HTS_SStreamSet_set_gv_switch: set GV switch */
void HTS_SStreamSet_set_gv_switch(HTS_SStreamSet * sss, size_t stream_index, size_t state_index, HTS_Boolean i);

/* HTS_SStreamSet_get_gv_switch: get GV switch */
HTS_Boolean HTS_SStreamSet_get_gv_switch(HTS_SStreamSet * sss, size_t stream_index, size_t state_index);

/* HTS_SStreamSet_clear: free state stream set */
void HTS_SStreamSet_clear(HTS_SStreamSet * sss);

/* pstream --------------------------------------------------------- */

/* check variance in finv() */
#define INFTY   ((double) 1.0e+38)
#define INFTY2  ((double) 1.0e+19)
#define INVINF  ((double) 1.0e-38)
#define INVINF2 ((double) 1.0e-19)

/* GV */
#define STEPINIT 0.1
#define STEPDEC  0.5
#define STEPINC  1.2
#define W1       1.0
#define W2       1.0
#define GV_MAX_ITERATION 5

/* HTS_PStreamSet_initialize: initialize parameter stream set */
void HTS_PStreamSet_initialize(HTS_PStreamSet * pss);

/* HTS_PStreamSet_create: parameter generation using GV weight */
HTS_Boolean HTS_PStreamSet_create(HTS_PStreamSet * pss, HTS_SStreamSet * sss, double *msd_threshold, double *gv_weight);

/* HTS_PStreamSet_get_nstream: get number of stream */
size_t HTS_PStreamSet_get_nstream(HTS_PStreamSet * pss);

/* HTS_PStreamSet_get_static_length: get features length */
size_t HTS_PStreamSet_get_vector_length(HTS_PStreamSet * pss, size_t stream_index);

/* HTS_PStreamSet_get_total_frame: get total number of frame */
size_t HTS_PStreamSet_get_total_frame(HTS_PStreamSet * pss);

/* HTS_PStreamSet_get_parameter: get parameter */
double HTS_PStreamSet_get_parameter(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index, size_t vector_index);

/* HTS_PStreamSet_get_parameter_vector: get parameter vector */
double *HTS_PStreamSet_get_parameter_vector(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);

/* HTS_PStreamSet_get_msd_flag: get generated MSD flag per frame */
HTS_Boolean HTS_PStreamSet_get_msd_flag(HTS_PStreamSet * pss, size_t stream_index, size_t frame_index);

/* HTS_PStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_PStreamSet_is_msd(HTS_PStreamSet * pss, size_t stream_index);

/* HTS_PStreamSet_clear: free parameter stream set */
void HTS_PStreamSet_clear(HTS_PStreamSet * pss);

/* gstream --------------------------------------------------------- */

/* HTS_GStreamSet_initialize: initialize generated parameter stream set */
void HTS_GStreamSet_initialize(HTS_GStreamSet * gss);

/* HTS_GStreamSet_create: generate speech */
HTS_Boolean HTS_GStreamSet_create(HTS_GStreamSet * gss, HTS_PStreamSet * pss, size_t stage, HTS_Boolean use_log_gain, size_t sampling_rate, size_t fperiod, double alpha, double beta, HTS_Boolean * stop, double volume, HTS_Audio * audio);

/* HTS_GStreamSet_get_total_nsamples: get total number of sample */
size_t HTS_GStreamSet_get_total_nsamples(HTS_GStreamSet * gss);

/* HTS_GStreamSet_get_total_frame: get total number of frame */
size_t HTS_GStreamSet_get_total_frame(HTS_GStreamSet * gss);

/* HTS_GStreamSet_get_static_length: get features length */
size_t HTS_GStreamSet_get_vector_length(HTS_GStreamSet * gss, size_t stream_index);

/* HTS_GStreamSet_get_speech: get synthesized speech parameter */
double HTS_GStreamSet_get_speech(HTS_GStreamSet * gss, size_t sample_index);

/* HTS_GStreamSet_get_parameter: get generated parameter */
double HTS_GStreamSet_get_parameter(HTS_GStreamSet * gss, size_t stream_index, size_t frame_index, size_t vector_index);

/* HTS_GStreamSet_clear: free generated parameter stream set */
void HTS_GStreamSet_clear(HTS_GStreamSet * gss);

/* vocoder --------------------------------------------------------- */

#ifndef LZERO
#define LZERO (-1.0e+10)        /* ~log(0) */
#endif                          /* !LZERO */

#ifndef ZERO
#define ZERO  (1.0e-10)         /* ~(0) */
#endif                          /* !ZERO */

#ifndef PI
#define PI  3.14159265358979323846
#endif                          /* !PI */

#ifndef PI2
#define PI2 6.28318530717958647692
#endif                          /* !PI2 */

#define RANDMAX 32767

#define SEED    1
#define B0      0x00000001
#define B28     0x10000000
#define B31     0x80000000
#define B31_    0x7fffffff
#define Z       0x00000000

#ifdef HTS_EMBEDDED
#define GAUSS     FALSE
#define PADEORDER 4             /* pade order (for MLSA filter) */
#define IRLENG    384           /* length of impulse response */
#else
#define GAUSS     TRUE
#define PADEORDER 5
#define IRLENG    576
#endif                          /* HTS_EMBEDDED */

#define CHECK_LSP_STABILITY_MIN 0.25
#define CHECK_LSP_STABILITY_NUM 4

/* for MGLSA filter */
#define NORMFLG1 TRUE
#define NORMFLG2 FALSE
#define MULGFLG1 TRUE
#define MULGFLG2 FALSE
#define NGAIN    FALSE

/* HTS_Vocoder: structure for setting of vocoder */
typedef struct _HTS_Vocoder {
   HTS_Boolean is_first;
   size_t stage;                /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   double gamma;                /* Gamma */
   HTS_Boolean use_log_gain;    /* log gain flag (for LSP) */
   size_t fprd;                 /* frame shift */
   unsigned long next;          /* temporary variable for random generator */
   HTS_Boolean gauss;           /* flag to use Gaussian noise */
   double rate;                 /* sampling rate */
   double pitch_of_curr_point;  /* used in excitation generation */
   double pitch_counter;        /* used in excitation generation */
   double pitch_inc_per_point;  /* used in excitation generation */
   double *excite_ring_buff;    /* used in excitation generation */
   size_t excite_buff_size;     /* used in excitation generation */
   size_t excite_buff_index;    /* used in excitation generation */
   unsigned char sw;            /* switch used in random generator */
   int x;                       /* excitation signal */
   double *freqt_buff;          /* used in freqt */
   size_t freqt_size;           /* buffer size for freqt */
   double *spectrum2en_buff;    /* used in spectrum2en */
   size_t spectrum2en_size;     /* buffer size for spectrum2en */
   double r1, r2, s;            /* used in random generator */
   double *postfilter_buff;     /* used in postfiltering */
   size_t postfilter_size;      /* buffer size for postfiltering */
   double *c, *cc, *cinc, *d1;  /* used in the MLSA/MGLSA filter */
   double *lsp2lpc_buff;        /* used in lsp2lpc */
   size_t lsp2lpc_size;         /* buffer size of lsp2lpc */
   double *gc2gc_buff;          /* used in gc2gc */
   size_t gc2gc_size;           /* buffer size for gc2gc */
} HTS_Vocoder;

/* HTS_Vocoder_initialize: initialize vocoder */
void HTS_Vocoder_initialize(HTS_Vocoder * v, size_t m, size_t stage, HTS_Boolean use_log_gain, size_t rate, size_t fperiod);

/* HTS_Vocoder_synthesize: pulse/noise excitation and MLSA/MGLSA filster based waveform synthesis */
void HTS_Vocoder_synthesize(HTS_Vocoder * v, size_t m, double lf0, double *spectrum, size_t nlpf, double *lpf, double alpha, double beta, double volume, double *rawdata, HTS_Audio * audio);

/* HTS_Vocoder_clear: clear vocoder */
void HTS_Vocoder_clear(HTS_Vocoder * v);

HTS_HIDDEN_H_END;

#endif                          /* !HTS_HIDDEN_H */
