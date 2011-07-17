/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2011  Nagoya Institute of Technology          */
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

#ifndef HTS_ENGINE_H
#define HTS_ENGINE_H
#ifdef __cplusplus
#define HTS_ENGINE_H_START extern "C" {
#define HTS_ENGINE_H_END   }
#else
#define HTS_ENGINE_H_START
#define HTS_ENGINE_H_END
#endif                          /* __CPLUSPLUS */
HTS_ENGINE_H_START;

#include <stdio.h>

/*  ------------------------ copyright ----------------------------  */

#ifdef PACKAGE_VERSION
#define HTS_VERSION   PACKAGE_VERSION
#else
#define HTS_VERSION   "1.05"
#endif
#define HTS_URL       "http://hts-engine.sourceforge.net/"
#define HTS_COPYRIGHT "2001-2011  Nagoya Institute of Technology", \
                      "2001-2008  Tokyo Institute of Technology"
#define HTS_NCOPYRIGHT 2

/* HTS_show_copyright: write copyright to file pointer */
void HTS_show_copyright(FILE * fp);

/* HTS_get_copyright: write copyright to string */
void HTS_get_copyright(char *str);

/*  -------------------------- common -----------------------------  */

typedef int HTS_Boolean;
#ifndef TRUE
#define TRUE  1
#endif                          /* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif                          /* !FALSE */

#define ZERO  1.0e-10           /* ~(0) */
#define LZERO (-1.0e+10)        /* ~log(0) */
#define LTPI  1.83787706640935  /* log(2*PI) */

/*  -------------------------- audio ------------------------------  */

#if !defined(AUDIO_PLAY_WINCE) && !defined(AUDIO_PLAY_WIN32) && !defined(AUDIO_PLAY_PORTAUDIO) && !defined(AUDIO_PLAY_NONE)
#if defined(__WINCE__) || defined(_WINCE) || defined(_WINCE) || defined(__WINCE)
#define AUDIO_PLAY_WINCE
#elif defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define AUDIO_PLAY_WIN32
#else
#define AUDIO_PLAY_NONE
#endif                          /* WINCE || WIN32 */
#endif                          /* !AUDIO_PLAY_WINCE && !AUDIO_PLAY_WIN32 && !AUDIO_PLAY_PORTAUDIO && !AUDIO_PLAY_NONE */

/* HTS_Audio: audio output for Windows */
#if defined (AUDIO_PLAY_WIN32) || defined(AUDIO_PLAY_WINCE)
#include <windows.h>
#include <mmsystem.h>
typedef struct _HTS_Audio {
   int sampling_rate;           /* sampling rate */
   int max_buff_size;           /* buffer size of audio output device */
   HWAVEOUT hwaveout;           /* audio device handle */
   WAVEFORMATEX waveformatex;   /* wave formatex */
   short *buff;                 /* current buffer */
   int buff_size;               /* current buffer size */
   int which_buff;              /* double buffering flag */
   HTS_Boolean now_buff_1;      /* double buffering flag */
   HTS_Boolean now_buff_2;      /* double buffering flag */
   WAVEHDR buff_1;              /* buffer */
   WAVEHDR buff_2;              /* buffer */
} HTS_Audio;
#endif                          /* AUDIO_PLAY_WIN32 || AUDIO_PLAY_WINCE */

/* HTS_Audio: audio output for PortAudio */
#ifdef AUDIO_PLAY_PORTAUDIO
#include "portaudio.h"
typedef struct _HTS_Audio {
   int sampling_rate;           /* sampling rate */
   int max_buff_size;           /* buffer size of audio output device */
   PaStreamParameters parameters;       /* parameters for output stream */
   PaStream *stream;            /* output stream */
   PaError err;                 /* error code */
   short *buff;                 /* current buffer */
   int buff_size;               /* current buffer size */
} HTS_Audio;
#endif                          /* AUDIO_PLAY_PORTAUDIO */

/* HTS_Audio: dummy audio output */
#ifdef AUDIO_PLAY_NONE
typedef struct _HTS_Audio {
   int i;                       /* make compiler happy */
} HTS_Audio;
#endif                          /* AUDIO_PLAY_NONE */

/*  ------------------------ audio method -------------------------  */

/* HTS_Audio_initialize: initialize audio */
void HTS_Audio_initialize(HTS_Audio * audio, int sampling_rate,
                          int max_buff_size);

/* HTS_Audio_set_parameter: set parameters for audio */
void HTS_Audio_set_parameter(HTS_Audio * audio, int sampling_rate,
                             int max_buff_size);

/* HTS_Audio_write: send data to audio */
void HTS_Audio_write(HTS_Audio * audio, short data);

/* HTS_Audio_flush: flush remain data */
void HTS_Audio_flush(HTS_Audio * audio);

/* HTS_Audio_clear: free audio */
void HTS_Audio_clear(HTS_Audio * audio);

/*  -------------------------- model ------------------------------  */

/* HTS_Window: Window coefficients to calculate dynamic features. */
typedef struct _HTS_Window {
   int size;                    /* number of windows (static + deltas) */
   int *l_width;                /* left width of windows */
   int *r_width;                /* right width of windows */
   double **coefficient;        /* window coefficient */
   int max_width;               /* maximum width of windows */
} HTS_Window;

/* HTS_Pattern: List of patterns in a question and a tree. */
typedef struct _HTS_Pattern {
   char *string;                /* pattern string */
   struct _HTS_Pattern *next;   /* pointer to the next pattern */
} HTS_Pattern;

/* HTS_Question: List of questions in a tree. */
typedef struct _HTS_Question {
   char *string;                /* name of this question */
   HTS_Pattern *head;           /* pointer to the head of pattern list */
   struct _HTS_Question *next;  /* pointer to the next question */
} HTS_Question;

/* HTS_Node: List of tree nodes in a tree. */
typedef struct _HTS_Node {
   int index;                   /* index of this node */
   int pdf;                     /* index of PDF for this node  ( leaf node only ) */
   struct _HTS_Node *yes;       /* pointer to its child node (yes) */
   struct _HTS_Node *no;        /* pointer to its child node (no) */
   struct _HTS_Node *next;      /* pointer to the next node */
   HTS_Question *quest;         /* question applied at this node */
} HTS_Node;

/* HTS_Tree: List of decision trees in a model. */
typedef struct _HTS_Tree {
   HTS_Pattern *head;           /* pointer to the head of pattern list for this tree */
   struct _HTS_Tree *next;      /* pointer to next tree */
   HTS_Node *root;              /* root node of this tree */
   int state;                   /* state index of this tree */
} HTS_Tree;

/* HTS_Model: Set of PDFs, decision trees and questions. */
typedef struct _HTS_Model {
   int vector_length;           /* vector length (include static and dynamic features) */
   int ntree;                   /* # of trees */
   int *npdf;                   /* # of PDFs at each tree */
   double ***pdf;               /* PDFs */
   HTS_Tree *tree;              /* pointer to the list of trees */
   HTS_Question *question;      /* pointer to the list of questions */
} HTS_Model;

/* HTS_Stream: Set of models and a window. */
typedef struct _HTS_Stream {
   int vector_length;           /* vector_length (include static and dynamic features) */
   HTS_Model *model;            /* models */
   HTS_Window window;           /* window coefficients */
   HTS_Boolean msd_flag;        /* flag for MSD */
   int interpolation_size;      /* # of models for interpolation */
} HTS_Stream;

/* HTS_ModelSet: Set of duration models, HMMs and GV models. */
typedef struct _HTS_ModelSet {
   HTS_Stream duration;         /* duration PDFs and trees */
   HTS_Stream *stream;          /* parameter PDFs, trees and windows */
   HTS_Stream *gv;              /* GV PDFs */
   HTS_Model gv_switch;         /* GV switch */
   int nstate;                  /* # of HMM states */
   int nstream;                 /* # of stream */
} HTS_ModelSet;

/*  ----------------------- model method --------------------------  */

/* HTS_ModelSet_initialize: initialize model set */
void HTS_ModelSet_initialize(HTS_ModelSet * ms, int nstream);

/* HTS_ModelSet_load_duration: load duration model and number of state */
void HTS_ModelSet_load_duration(HTS_ModelSet * ms, FILE ** pdf_fp,
                                FILE ** tree_fp, int interpolation_size);

/* HTS_ModelSet_load_parameter: load parameter model */
void HTS_ModelSet_load_parameter(HTS_ModelSet * ms, FILE ** pdf_fp,
                                 FILE ** tree_fp, FILE ** win_fp,
                                 int stream_index, HTS_Boolean msd_flag,
                                 int window_size, int interpolation_size);

/* HTS_ModelSet_load_gv: load GV model */
void HTS_ModelSet_load_gv(HTS_ModelSet * ms, FILE ** pdf_fp, FILE ** tree_fp,
                          int stream_index, int interpolation_size);

/* HTS_ModelSet_have_gv_tree: if context-dependent GV is used, return true */
HTS_Boolean HTS_ModelSet_have_gv_tree(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_load_gv_switch: load GV switch */
void HTS_ModelSet_load_gv_switch(HTS_ModelSet * ms, FILE * fp);

/* HTS_ModelSet_have_gv_switch: if GV switch is used, return true */
HTS_Boolean HTS_ModelSet_have_gv_switch(HTS_ModelSet * ms);

/* HTS_ModelSet_get_nstate: get number of state */
int HTS_ModelSet_get_nstate(HTS_ModelSet * ms);

/* HTS_ModelSet_get_nstream: get number of stream */
int HTS_ModelSet_get_nstream(HTS_ModelSet * ms);

/* HTS_ModelSet_get_vector_length: get vector length */
int HTS_ModelSet_get_vector_length(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_is_msd: get MSD flag */
HTS_Boolean HTS_ModelSet_is_msd(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_get_window_size: get dynamic window size */
int HTS_ModelSet_get_window_size(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS_ModelSet_get_window_left_width(HTS_ModelSet * ms, int stream_index,
                                       int window_index);

/* HTS_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS_ModelSet_get_window_right_width(HTS_ModelSet * ms, int stream_index,
                                        int window_index);

/* HTS_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_ModelSet_get_window_coefficient(HTS_ModelSet * ms, int stream_index,
                                           int window_index,
                                           int coefficient_index);

/* HTS_ModelSet_get_window_max_width: get max width of dynamic window */
int HTS_ModelSet_get_window_max_width(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_get_duration_interpolation_size: get interpolation size (duration model) */
int HTS_ModelSet_get_duration_interpolation_size(HTS_ModelSet * ms);

/* HTS_ModelSet_get_parameter_interpolation_size: get interpolation size (parameter model) */
int HTS_ModelSet_get_parameter_interpolation_size(HTS_ModelSet * ms,
                                                  int stream_index);

/* HTS_ModelSet_get_gv_interpolation_size: get interpolation size (GV model) */
int HTS_ModelSet_get_gv_interpolation_size(HTS_ModelSet * ms, int stream_index);

/* HTS_ModelSet_use_gv: get GV flag */
HTS_Boolean HTS_ModelSet_use_gv(HTS_ModelSet * ms, int index);

/* HTS_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS_ModelSet_get_duration_index(HTS_ModelSet * ms, char *string,
                                     int *tree_index, int *pdf_index,
                                     int interpolation_index);

/* HTS_ModelSet_get_duration: get duration using interpolation weight */
void HTS_ModelSet_get_duration(HTS_ModelSet * ms, char *string, double *mean,
                               double *vari, double *iw);

/* HTS_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS_ModelSet_get_parameter_index(HTS_ModelSet * ms, char *string,
                                      int *tree_index, int *pdf_index,
                                      int stream_index, int state_index,
                                      int interpolation_index);

/* HTS_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS_ModelSet_get_parameter(HTS_ModelSet * ms, char *string, double *mean,
                                double *vari, double *msd, int stream_index,
                                int state_index, double *iw);

/* HTS_ModelSet_get_gv: get GV using interpolation weight */
void HTS_ModelSet_get_gv(HTS_ModelSet * ms, char *string, double *mean,
                         double *vari, int stream_index, double *iw);

/* HTS_ModelSet_get_gv_switch: get GV switch */
HTS_Boolean HTS_ModelSet_get_gv_switch(HTS_ModelSet * ms, char *string);

/* HTS_ModelSet_clear: free model set */
void HTS_ModelSet_clear(HTS_ModelSet * ms);

/*  -------------------------- label ------------------------------  */

/* HTS_LabelString: individual label string with time information */
typedef struct _HTS_LabelString {
   struct _HTS_LabelString *next;       /* pointer to next label string */
   char *name;                  /* label string */
   double start;                /* start frame specified in the given label */
   double end;                  /* end frame specified in the given label */
} HTS_LabelString;

/* HTS_Label: list of label strings */
typedef struct _HTS_Label {
   HTS_LabelString *head;       /* pointer to the head of label string */
   int size;                    /* # of label strings */
   HTS_Boolean frame_flag;      /* flag for frame length modification */
   double speech_speed;         /* speech speed rate */
} HTS_Label;

/*  ----------------------- label method --------------------------  */

/* HTS_Label_initialize: initialize label */
void HTS_Label_initialize(HTS_Label * label);

/* HTS_Label_load_from_fn: load label from file name */
void HTS_Label_load_from_fn(HTS_Label * label, int sampling_rate, int fperiod,
                            char *fn);

/* HTS_Label_load_from_fp: load label list from file pointer */
void HTS_Label_load_from_fp(HTS_Label * label, int sampling_rate, int fperiod,
                            FILE * fp);

/* HTS_Label_load_from_string: load label from string */
void HTS_Label_load_from_string(HTS_Label * label, int sampling_rate,
                                int fperiod, char *data);

/* HTS_Label_load_from_string_list: load label list from string list */
void HTS_Label_load_from_string_list(HTS_Label * label, int sampling_rate,
                                     int fperiod, char **data, int size);

/* HTS_Label_set_speech_speed: set speech speed rate */
void HTS_Label_set_speech_speed(HTS_Label * label, double f);

/* HTS_Label_set_frame_specified_flag: set frame specified flag */
void HTS_Label_set_frame_specified_flag(HTS_Label * label, HTS_Boolean i);

/* HTS_Label_get_size: get number of label string */
int HTS_Label_get_size(HTS_Label * label);

/* HTS_Label_get_string: get label string */
char *HTS_Label_get_string(HTS_Label * label, int string_index);

/* HTS_Label_get_frame_specified_flag: get frame specified flag */
HTS_Boolean HTS_Label_get_frame_specified_flag(HTS_Label * label);

/* HTS_Label_get_start_frame: get start frame */
double HTS_Label_get_start_frame(HTS_Label * label, int string_index);

/* HTS_Label_get_end_frame: get end frame */
double HTS_Label_get_end_frame(HTS_Label * label, int string_index);

/* HTS_Label_get_speech_speed: get speech speed rate */
double HTS_Label_get_speech_speed(HTS_Label * label);

/* HTS_Label_clear: free label */
void HTS_Label_clear(HTS_Label * label);

/*  -------------------------- sstream ----------------------------  */

/* HTS_SStream: individual state stream */
typedef struct _HTS_SStream {
   int vector_length;           /* vector length (include static and dynamic features) */
   double **mean;               /* mean vector sequence */
   double **vari;               /* variance vector sequence */
   double *msd;                 /* MSD parameter sequence */
   int win_size;                /* # of windows (static + deltas) */
   int *win_l_width;            /* left width of windows */
   int *win_r_width;            /* right width of windows */
   double **win_coefficient;    /* window cofficients */
   int win_max_width;           /* maximum width of windows */
   double *gv_mean;             /* mean vector of GV */
   double *gv_vari;             /* variance vector of GV */
   HTS_Boolean *gv_switch;      /* GV flag sequence */
} HTS_SStream;

/* HTS_SStreamSet: set of state stream */
typedef struct _HTS_SStreamSet {
   HTS_SStream *sstream;        /* state streams */
   int nstream;                 /* # of streams */
   int nstate;                  /* # of states */
   int *duration;               /* duration sequence */
   int total_state;             /* total state */
   int total_frame;             /* total frame */
} HTS_SStreamSet;

/*  ----------------------- sstream method ------------------------  */

/* HTS_SStreamSet_initialize: initialize state stream set */
void HTS_SStreamSet_initialize(HTS_SStreamSet * sss);

/* HTS_SStreamSet_create: parse label and determine state duration */
void HTS_SStreamSet_create(HTS_SStreamSet * sss, HTS_ModelSet * ms,
                           HTS_Label * label, double *duration_iw,
                           double **parameter_iw, double **gv_iw);

/* HTS_SStreamSet_get_nstream: get number of stream */
int HTS_SStreamSet_get_nstream(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_vector_length: get vector length */
int HTS_SStreamSet_get_vector_length(HTS_SStreamSet * sss, int stream_index);

/* HTS_SStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_SStreamSet_is_msd(HTS_SStreamSet * sss, int stream_index);

/* HTS_SStreamSet_get_total_state: get total number of state */
int HTS_SStreamSet_get_total_state(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_total_frame: get total number of frame */
int HTS_SStreamSet_get_total_frame(HTS_SStreamSet * sss);

/* HTS_SStreamSet_get_msd: get msd parameter */
double HTS_SStreamSet_get_msd(HTS_SStreamSet * sss, int stream_index,
                              int state_index);

/* HTS_SStreamSet_window_size: get dynamic window size */
int HTS_SStreamSet_get_window_size(HTS_SStreamSet * sss, int stream_index);

/* HTS_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS_SStreamSet_get_window_left_width(HTS_SStreamSet * sss, int stream_index,
                                         int window_index);

/* HTS_SStreamSet_get_window_right_width: get right width of dynamic window */
int HTS_SStreamSet_get_window_right_width(HTS_SStreamSet * sss,
                                          int stream_index, int window_index);

/* HTS_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS_SStreamSet_get_window_coefficient(HTS_SStreamSet * sss,
                                             int stream_index, int window_index,
                                             int coefficient_index);

/* HTS_SStreamSet_get_window_max_width: get max width of dynamic window */
int HTS_SStreamSet_get_window_max_width(HTS_SStreamSet * sss, int stream_index);

/* HTS_SStreamSet_use_gv: get GV flag */
HTS_Boolean HTS_SStreamSet_use_gv(HTS_SStreamSet * sss, int stream_index);

/* HTS_SStreamSet_get_duration: get state duration */
int HTS_SStreamSet_get_duration(HTS_SStreamSet * sss, int state_index);

/* HTS_SStreamSet_get_mean: get mean parameter */
double HTS_SStreamSet_get_mean(HTS_SStreamSet * sss, int stream_index,
                               int state_index, int vector_index);

/* HTS_SStreamSet_set_mean: set mean parameter */
void HTS_SStreamSet_set_mean(HTS_SStreamSet * sss, int stream_index,
                             int state_index, int vector_index, double f);

/* HTS_SStreamSet_get_vari: get variance parameter */
double HTS_SStreamSet_get_vari(HTS_SStreamSet * sss, int stream_index,
                               int state_index, int vector_index);

/* HTS_SStreamSet_set_vari: set variance parameter */
void HTS_SStreamSet_set_vari(HTS_SStreamSet * sss, int stream_index,
                             int state_index, int vector_index, double f);

/* HTS_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS_SStreamSet_get_gv_mean(HTS_SStreamSet * sss, int stream_index,
                                  int vector_index);

/* HTS_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS_SStreamSet_get_gv_vari(HTS_SStreamSet * sss, int stream_index,
                                  int vector_index);

/* HTS_SStreamSet_set_gv_switch: set GV switch */
void HTS_SStreamSet_set_gv_switch(HTS_SStreamSet * sss, int stream_index,
                                  int state_index, HTS_Boolean i);

/* HTS_SStreamSet_get_gv_switch: get GV switch */
HTS_Boolean HTS_SStreamSet_get_gv_switch(HTS_SStreamSet * sss, int stream_index,
                                         int state_index);

/* HTS_SStreamSet_clear: free state stream set */
void HTS_SStreamSet_clear(HTS_SStreamSet * sss);

/*  -------------------------- pstream ----------------------------  */

/* HTS_SMatrices: Matrices/Vectors used in the speech parameter generation algorithm. */
typedef struct _HTS_SMatrices {
   double **mean;               /* mean vector sequence */
   double **ivar;               /* inverse diag variance sequence */
   double *g;                   /* vector used in the forward substitution */
   double **wuw;                /* W' U^-1 W  */
   double *wum;                 /* W' U^-1 mu */
} HTS_SMatrices;

/* HTS_PStream: Individual PDF stream. */
typedef struct _HTS_PStream {
   int vector_length;           /* vector length (include static and dynamic features) */
   int static_length;           /* static features length */
   int length;                  /* stream length */
   int width;                   /* width of dynamic window */
   double **par;                /* output parameter vector */
   HTS_SMatrices sm;            /* matrices for parameter generation */
   int win_size;                /* # of windows (static + deltas) */
   int *win_l_width;            /* left width of windows */
   int *win_r_width;            /* right width of windows */
   double **win_coefficient;    /* window coefficients */
   HTS_Boolean *msd_flag;       /* Boolean sequence for MSD */
   double *gv_mean;             /* mean vector of GV */
   double *gv_vari;             /* variance vector of GV */
   HTS_Boolean *gv_switch;      /* GV flag sequence */
   int gv_length;               /* frame length for GV calculation */
} HTS_PStream;

/* HTS_PStreamSet: Set of PDF streams. */
typedef struct _HTS_PStreamSet {
   HTS_PStream *pstream;        /* PDF streams */
   int nstream;                 /* # of PDF streams */
   int total_frame;             /* total frame */
} HTS_PStreamSet;

/*  ----------------------- pstream method ------------------------  */

/* HTS_PStreamSet_initialize: initialize parameter stream set */
void HTS_PStreamSet_initialize(HTS_PStreamSet * pss);

/* HTS_PStreamSet_create: parameter generation using GV weight */
void HTS_PStreamSet_create(HTS_PStreamSet * pss, HTS_SStreamSet * sss,
                           double *msd_threshold, double *gv_weight);

/* HTS_PStreamSet_get_nstream: get number of stream */
int HTS_PStreamSet_get_nstream(HTS_PStreamSet * pss);

/* HTS_PStreamSet_get_static_length: get static features length */
int HTS_PStreamSet_get_static_length(HTS_PStreamSet * pss, int stream_index);

/* HTS_PStreamSet_get_total_frame: get total number of frame */
int HTS_PStreamSet_get_total_frame(HTS_PStreamSet * pss);

/* HTS_PStreamSet_get_parameter: get parameter */
double HTS_PStreamSet_get_parameter(HTS_PStreamSet * pss, int stream_index,
                                    int frame_index, int vector_index);

/* HTS_PStreamSet_get_parameter_vector: get parameter vector */
double *HTS_PStreamSet_get_parameter_vector(HTS_PStreamSet * pss,
                                            int stream_index, int frame_index);

/* HTS_PStreamSet_get_msd_flag: get generated MSD flag per frame */
HTS_Boolean HTS_PStreamSet_get_msd_flag(HTS_PStreamSet * pss, int stream_index,
                                        int frame_index);

/* HTS_PStreamSet_is_msd: get MSD flag */
HTS_Boolean HTS_PStreamSet_is_msd(HTS_PStreamSet * pss, int stream_index);

/* HTS_PStreamSet_clear: free parameter stream set */
void HTS_PStreamSet_clear(HTS_PStreamSet * pss);

/*  -------------------------- gstream ----------------------------  */

/* HTS_GStream: Generated parameter stream. */
typedef struct _HTS_GStream {
   int static_length;           /* static features length */
   double **par;                /* generated parameter */
} HTS_GStream;

/* HTS_GStreamSet: Set of generated parameter stream. */
typedef struct _HTS_GStreamSet {
   int total_nsample;           /* total sample */
   int total_frame;             /* total frame */
   int nstream;                 /* # of streams */
   HTS_GStream *gstream;        /* generated parameter streams */
   short *gspeech;              /* generated speech */
} HTS_GStreamSet;

/*  ----------------------- gstream method ------------------------  */

/* HTS_GStreamSet_initialize: initialize generated parameter stream set */
void HTS_GStreamSet_initialize(HTS_GStreamSet * gss);

/* HTS_GStreamSet_create: generate speech */
void HTS_GStreamSet_create(HTS_GStreamSet * gss, HTS_PStreamSet * pss,
                           int stage, HTS_Boolean use_log_gain,
                           int sampling_rate, int fperiod, double alpha,
                           double beta,
                           HTS_Boolean * stop, double volume,
                           HTS_Audio * audio);

/* HTS_GStreamSet_get_total_nsample: get total number of sample */
int HTS_GStreamSet_get_total_nsample(HTS_GStreamSet * gss);

/* HTS_GStreamSet_get_total_frame: get total number of frame */
int HTS_GStreamSet_get_total_frame(HTS_GStreamSet * gss);

/* HTS_GStreamSet_get_static_length: get static features length */
int HTS_GStreamSet_get_static_length(HTS_GStreamSet * gss, int stream_index);

/* HTS_GStreamSet_get_speech: get synthesized speech parameter */
short HTS_GStreamSet_get_speech(HTS_GStreamSet * gss, int sample_index);

/* HTS_GStreamSet_get_parameter: get generated parameter */
double HTS_GStreamSet_get_parameter(HTS_GStreamSet * gss, int stream_index,
                                    int frame_index, int vector_index);

/* HTS_GStreamSet_clear: free generated parameter stream set */
void HTS_GStreamSet_clear(HTS_GStreamSet * gss);

/*  -------------------------- engine -----------------------------  */

/* HTS_Global: Global settings. */
typedef struct _HTS_Global {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   HTS_Boolean use_log_gain;    /* log gain flag (for LSP) */
   int sampling_rate;           /* sampling rate */
   int fperiod;                 /* frame period */
   double alpha;                /* all-pass constant */
   double beta;                 /* postfiltering coefficient */
   int audio_buff_size;         /* audio buffer size (for audio device) */
   double *msd_threshold;       /* MSD thresholds */
   double *duration_iw;         /* weights for duration interpolation */
   double **parameter_iw;       /* weights for parameter interpolation */
   double **gv_iw;              /* weights for GV interpolation */
   double *gv_weight;           /* GV weights */
   HTS_Boolean stop;            /* stop flag */
   double volume;               /* volume */
} HTS_Global;

/* HTS_Engine: Engine itself. */
typedef struct _HTS_Engine {
   HTS_Global global;           /* global settings */
   HTS_Audio audio;             /* audio output */
   HTS_ModelSet ms;             /* set of duration models, HMMs and GV models */
   HTS_Label label;             /* label */
   HTS_SStreamSet sss;          /* set of state streams */
   HTS_PStreamSet pss;          /* set of PDF streams */
   HTS_GStreamSet gss;          /* set of generated parameter streams */
} HTS_Engine;

/*  ----------------------- engine method -------------------------  */

/* HTS_Engine_initialize: initialize engine */
void HTS_Engine_initialize(HTS_Engine * engine, int nstream);

/* HTS_engine_load_duration_from_fn: load duration pdfs ,trees and number of state from file names */
void HTS_Engine_load_duration_from_fn(HTS_Engine * engine, char **pdf_fn,
                                      char **tree_fn, int interpolation_size);

/* HTS_Engine_load_duration_from_fp: load duration pdfs, trees and number of state from file pointers */
void HTS_Engine_load_duration_from_fp(HTS_Engine * engine, FILE ** pdf_fp,
                                      FILE ** tree_fp, int interpolation_size);

/* HTS_Engine_load_parameter_from_fn: load parameter pdfs, trees and windows from file names */
void HTS_Engine_load_parameter_from_fn(HTS_Engine * engine, char **pdf_fn,
                                       char **tree_fn, char **win_fn,
                                       int stream_index, HTS_Boolean msd_flag,
                                       int window_size, int interpolation_size);

/* HTS_Engine_load_parameter_from_fp: load parameter pdfs, trees and windows from file pointers */
void HTS_Engine_load_parameter_from_fp(HTS_Engine * engine, FILE ** pdf_fp,
                                       FILE ** tree_fp, FILE ** win_fp,
                                       int stream_index, HTS_Boolean msd_flag,
                                       int window_size, int interpolation_size);

/* HTS_Engine_load_gv_from_fn: load GV pdfs and trees from file names */
void HTS_Engine_load_gv_from_fn(HTS_Engine * engine, char **pdf_fn,
                                char **tree_fn, int stream_index,
                                int interpolation_size);

/* HTS_Engine_load_gv_from_fp: load GV pdfs and trees from file pointers */
void HTS_Engine_load_gv_from_fp(HTS_Engine * engine, FILE ** pdf_fp,
                                FILE ** tree_fp, int stream_index,
                                int interpolation_size);

/* HTS_Engine_load_gv_switch_from_fn: load GV switch from file names */
void HTS_Engine_load_gv_switch_from_fn(HTS_Engine * engine, char *fn);

/* HTS_Engine_load_gv_switch_from_fp: load GV switch from file pointers */
void HTS_Engine_load_gv_switch_from_fp(HTS_Engine * engine, FILE * fp);

/* HTS_Engine_set_sampling_rate: set sampling rate */
void HTS_Engine_set_sampling_rate(HTS_Engine * engine, int i);

/* HTS_Engine_get_sampling_rate: get sampling rate */
int HTS_Engine_get_sampling_rate(HTS_Engine * engine);

/* HTS_Engine_set_fperiod: set frame shift */
void HTS_Engine_set_fperiod(HTS_Engine * engine, int i);

/* HTS_Engine_get_fperiod: get frame shift */
int HTS_Engine_get_fperiod(HTS_Engine * engine);

/* HTS_Engine_set_alpha: set alpha */
void HTS_Engine_set_alpha(HTS_Engine * engine, double f);

/* HTS_Engine_set_gamma: set gamma (Gamma=-1/i: if i=0 then Gamma=0) */
void HTS_Engine_set_gamma(HTS_Engine * engine, int i);

/* HTS_Engine_set_log_gain: set log gain flag (for LSP) */
void HTS_Engine_set_log_gain(HTS_Engine * engine, HTS_Boolean i);

/* HTS_Engine_set_beta: set beta */
void HTS_Engine_set_beta(HTS_Engine * engine, double f);

/* HTS_Engine_set_audio_buff_size: set audio buffer size */
void HTS_Engine_set_audio_buff_size(HTS_Engine * engine, int i);

/* HTS_Engine_get_audio_buff_size: get audio buffer size */
int HTS_Engine_get_audio_buff_size(HTS_Engine * engine);

/* HTS_Egnine_set_msd_threshold: set MSD threshold */
void HTS_Engine_set_msd_threshold(HTS_Engine * engine, int stream_index,
                                  double f);

/* HTS_Engine_set_duration_interpolation_weight: set interpolation weight for duration */
void HTS_Engine_set_duration_interpolation_weight(HTS_Engine * engine,
                                                  int interpolation_index,
                                                  double f);

/* HTS_Engine_set_parameter_interpolation_weight: set interpolation weight for parameter */
void HTS_Engine_set_parameter_interpolation_weight(HTS_Engine * engine,
                                                   int stream_index,
                                                   int interpolation_index,
                                                   double f);

/* HTS_Engine_set_gv_interpolation_weight: set interpolation weight for GV */
void HTS_Engine_set_gv_interpolation_weight(HTS_Engine * engine,
                                            int stream_index,
                                            int interpolation_index, double f);

/* HTS_Engine_set_gv_weight: set GV weight */
void HTS_Engine_set_gv_weight(HTS_Engine * engine, int stream_index, double f);

/* HTS_Engine_set_stop_flag: set stop flag */
void HTS_Engine_set_stop_flag(HTS_Engine * engine, HTS_Boolean b);

/* HTS_Engine_set_volume: set volume */
void HTS_Engine_set_volume(HTS_Engine * engine, double f);

/* HTS_Engine_get_total_state: get total number of state */
int HTS_Engine_get_total_state(HTS_Engine * engine);

/* HTS_Engine_set_state_mean: set mean value of state */
void HTS_Engine_set_state_mean(HTS_Engine * engine, int stream_index,
                               int state_index, int vector_index, double f);

/* HTS_Engine_get_state_mean: get mean value of state */
double HTS_Engine_get_state_mean(HTS_Engine * engine, int stream_index,
                                 int state_index, int vector_index);

/* HTS_Engine_get_state_duration: get state duration */
int HTS_Engine_get_state_duration(HTS_Engine * engine, int state_index);

/* HTS_Engine_get_nstream: get number of stream */
int HTS_Engine_get_nstream(HTS_Engine * engine);

/* HTS_Engine_get_nstate: get number of state */
int HTS_Engine_get_nstate(HTS_Engine * engine);

/* HTS_Engine_load_label_from_fn: load label from file pointer */
void HTS_Engine_load_label_from_fn(HTS_Engine * engine, char *fn);

/* HTS_Engine_load_label_from_fp: load label from file name */
void HTS_Engine_load_label_from_fp(HTS_Engine * engine, FILE * fp);

/* HTS_Engine_load_label_from_string: load label from string */
void HTS_Engine_load_label_from_string(HTS_Engine * engine, char *data);

/* HTS_Engine_load_label_from_string_list: load label from string list */
void HTS_Engine_load_label_from_string_list(HTS_Engine * engine, char **data,
                                            int size);

/* HTS_Engine_create_sstream: parse label and determine state duration */
void HTS_Engine_create_sstream(HTS_Engine * engine);

/* HTS_Engine_create_pstream: generate speech parameter vector sequence */
void HTS_Engine_create_pstream(HTS_Engine * engine);

/* HTS_Engine_create_gstream: synthesis speech */
void HTS_Engine_create_gstream(HTS_Engine * engine);

/* HTS_Engine_save_information: output trace information */
void HTS_Engine_save_information(HTS_Engine * engine, FILE * fp);

/* HTS_Engine_save_label: output label with time */
void HTS_Engine_save_label(HTS_Engine * engine, FILE * fp);

/* HTS_Engine_save_generated_parameter: output generated parameter */
void HTS_Engine_save_generated_parameter(HTS_Engine * engine, FILE * fp,
                                         int stream_index);

/* HTS_Engine_save_generated_speech: output generated speech */
void HTS_Engine_save_generated_speech(HTS_Engine * engine, FILE * fp);

/* HTS_Engine_save_riff: output RIFF format file */
void HTS_Engine_save_riff(HTS_Engine * engine, FILE * wavfp);

/* HTS_Engine_refresh: free memory per one time synthesis */
void HTS_Engine_refresh(HTS_Engine * engine);

/* HTS_Engine_clear: free engine */
void HTS_Engine_clear(HTS_Engine * engine);

/*  -------------------------- vocoder ----------------------------  */

/* HTS_Vocoder: structure for setting of vocoder */
typedef struct _HTS_Vocoder {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   double gamma;                /* Gamma */
   HTS_Boolean use_log_gain;    /* log gain flag (for LSP) */
   int fprd;                    /* frame shift */
   int iprd;                    /* interpolation period */
   int seed;                    /* seed of random generator */
   unsigned long next;          /* temporary variable for random generator */
   HTS_Boolean gauss;           /* flag to use Gaussian noise */
   double rate;                 /* sampling rate */
   double p1;                   /* used in excitation generation */
   double pc;                   /* used in excitation generation */
   double p;                    /* used in excitation generation */
   double inc;                  /* used in excitation generation */
   double *pulse_list;          /* used in excitation generation */
   int sw;                      /* switch used in random generator */
   int x;                       /* excitation signal */
   double *freqt_buff;          /* used in freqt */
   int freqt_size;              /* buffer size for freqt */
   double *spectrum2en_buff;    /* used in spectrum2en */
   int spectrum2en_size;        /* buffer size for spectrum2en */
   double r1, r2, s;            /* used in random generator */
   double *postfilter_buff;     /* used in postfiltering */
   int postfilter_size;         /* buffer size for postfiltering */
   double *c, *cc, *cinc, *d1;  /* used in the MLSA/MGLSA filter */
   double *lsp2lpc_buff;        /* used in lsp2lpc */
   int lsp2lpc_size;            /* buffer size of lsp2lpc */
   double *gc2gc_buff;          /* used in gc2gc */
   int gc2gc_size;              /* buffer size for gc2gc */
} HTS_Vocoder;

/*  ----------------------- vocoder method ------------------------  */

/* HTS_Vocoder_initialize: initialize vocoder */
void HTS_Vocoder_initialize(HTS_Vocoder * v, const int m, const int stage,
                            HTS_Boolean use_log_gain, const int rate,
                            const int fperiod);

/* HTS_Vocoder_synthesize: pulse/noise excitation and MLSA/MGLSA filster based waveform synthesis */
void HTS_Vocoder_synthesize(HTS_Vocoder * v, const int m, double lf0,
                            double *spectrum, const int nlpf, double *lpf,
                            double alpha, double beta, double volume,
                            short *rawdata, HTS_Audio * audio);

/* HTS_Vocoder_postfilter_mcp: postfilter for MCP */
void HTS_Vocoder_postfilter_mcp(HTS_Vocoder * v, double *mcp, const int m,
                                double alpha, double beta);

/* HTS_Vocoder_clear: clear vocoder */
void HTS_Vocoder_clear(HTS_Vocoder * v);

HTS_ENGINE_H_END;

#endif                          /* !HTS_ENGINE_H */
