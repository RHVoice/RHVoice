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

#ifndef HTS106_ENGINE_H
#define HTS106_ENGINE_H

#ifdef __cplusplus
#define HTS106_ENGINE_H_START extern "C" {
#define HTS106_ENGINE_H_END   }
#else
#define HTS106_ENGINE_H_START
#define HTS106_ENGINE_H_END
#endif                          /* __CPLUSPLUS */

HTS106_ENGINE_H_START;

#include <stdio.h>

#include "core/question_matcher.h"

/*  -------------------------- common -----------------------------  */

typedef int HTS106_Boolean;
#ifndef TRUE
#define TRUE  1
#endif                          /* !TRUE */
#ifndef FALSE
#define FALSE 0
#endif                          /* !FALSE */

#define ZERO  1.0e-10           /* ~(0) */
#define LZERO (-1.0e+10)        /* ~log(0) */
#define LTPI  1.83787706640935  /* log(2*PI) */

typedef FILE HTS106_File;

/* HTS106_fopen: wrapper for fopen */
HTS106_File *HTS106_fopen(const char *name, const char *opt);

/* HTS106_fclose: wrapper for fclose */
void HTS106_fclose(HTS106_File * fp);

/*  ------------------------ copyright ----------------------------  */

#define HTS106_VERSION   "1.06"
#define HTS106_URL       "http://hts-engine.sourceforge.net/"
#define HTS106_COPYRIGHT "2001-2011  Nagoya Institute of Technology", \
                      "2001-2008  Tokyo Institute of Technology"
#define HTS106_NCOPYRIGHT 2

/* HTS106_show_copyright: write copyright to file pointer */
void HTS106_show_copyright(HTS106_File * fp);

/* HTS106_get_copyright: write copyright to string */
void HTS106_get_copyright(char *str);

/*  -------------------------- audio ------------------------------  */

#define AUDIO_PLAY_NONE

#if !defined(AUDIO_PLAY_WINCE) && !defined(AUDIO_PLAY_WIN32) && !defined(AUDIO_PLAY_PORTAUDIO) && !defined(AUDIO_PLAY_NONE) && !defined(AUDIO_PLAY_CALLER)
#if defined(__WINCE__) || defined(_WINCE) || defined(_WINCE) || defined(__WINCE)
#define AUDIO_PLAY_WINCE
#elif defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#define AUDIO_PLAY_WIN32
#else
#define AUDIO_PLAY_NONE
#endif                          /* WINCE || WIN32 */
#endif                          /* !AUDIO_PLAY_WINCE && !AUDIO_PLAY_WIN32 && !AUDIO_PLAY_PORTAUDIO && !AUDIO_PLAY_NONE */

/* HTS106_Audio: audio output for Windows */
#if defined (AUDIO_PLAY_WIN32) || defined(AUDIO_PLAY_WINCE)
#include <windows.h>
#include <mmsystem.h>
typedef struct _HTS106_Audio {
   int sampling_rate;           /* sampling rate */
   int max_buff_size;           /* buffer size of audio output device */
   HWAVEOUT hwaveout;           /* audio device handle */
   WAVEFORMATEX waveformatex;   /* wave formatex */
   short *buff;                 /* current buffer */
   int buff_size;               /* current buffer size */
   int which_buff;              /* double buffering flag */
   HTS106_Boolean now_buff_1;      /* double buffering flag */
   HTS106_Boolean now_buff_2;      /* double buffering flag */
   WAVEHDR buff_1;              /* buffer */
   WAVEHDR buff_2;              /* buffer */
} HTS106_Audio;
#endif                          /* AUDIO_PLAY_WIN32 || AUDIO_PLAY_WINCE */

/* HTS106_Audio: audio output for PortAudio */
#ifdef AUDIO_PLAY_PORTAUDIO
#include "portaudio.h"
typedef struct _HTS106_Audio {
   int sampling_rate;           /* sampling rate */
   int max_buff_size;           /* buffer size of audio output device */
   PaStreamParameters parameters;       /* parameters for output stream */
   PaStream *stream;            /* output stream */
   PaError err;                 /* error code */
   short *buff;                 /* current buffer */
   int buff_size;               /* current buffer size */
} HTS106_Audio;
#endif                          /* AUDIO_PLAY_PORTAUDIO */

/* HTS106_Audio: dummy audio output */
#ifdef AUDIO_PLAY_NONE
typedef struct _HTS106_Audio {
   int i;                       /* make compiler happy */
} HTS106_Audio;
#endif                          /* AUDIO_PLAY_NONE */

/* HTS106_Audio: audio output defined by the caller */
#ifdef AUDIO_PLAY_CALLER
typedef struct _HTS106_Audio {
   void* data;
} HTS106_Audio;
#endif                          /* AUDIO_PLAY_CALLER */

/*  ------------------------ audio method -------------------------  */

/* HTS106_Audio_initialize: initialize audio */
void HTS106_Audio_initialize(HTS106_Audio * audio, int sampling_rate, int max_buff_size);

/* HTS106_Audio_set_parameter: set parameters for audio */
void HTS106_Audio_set_parameter(HTS106_Audio * audio, int sampling_rate, int max_buff_size);

/* HTS106_Audio_write: send data to audio */
void HTS106_Audio_write(HTS106_Audio * audio, short data);

/* HTS106_Audio_flush: flush remain data */
void HTS106_Audio_flush(HTS106_Audio * audio);

/* HTS106_Audio_clear: free audio */
void HTS106_Audio_clear(HTS106_Audio * audio);

/*  -------------------------- model ------------------------------  */

/* HTS106_Window: Window coefficients to calculate dynamic features. */
typedef struct _HTS106_Window {
   int size;                    /* number of windows (static + deltas) */
   int *l_width;                /* left width of windows */
   int *r_width;                /* right width of windows */
   double **coefficient;        /* window coefficient */
   int max_width;               /* maximum width of windows */
} HTS106_Window;

/* HTS106_Pattern: List of patterns in a question and a tree. */
typedef struct _HTS106_Pattern {
   char *string;                /* pattern string */
   struct _HTS106_Pattern *next;   /* pointer to the next pattern */
} HTS106_Pattern;

/* HTS106_Question: List of questions in a tree. */
typedef struct _HTS106_Question {
   char *string;                /* name of this question */
   HTS106_Pattern *head;           /* pointer to the head of pattern list */
   struct _HTS106_Question *next;  /* pointer to the next question */
} HTS106_Question;

/* HTS106_Node: List of tree nodes in a tree. */
typedef struct _HTS106_Node {
   int index;                   /* index of this node */
   int pdf;                     /* index of PDF for this node  ( leaf node only ) */
   struct _HTS106_Node *yes;       /* pointer to its child node (yes) */
   struct _HTS106_Node *no;        /* pointer to its child node (no) */
   struct _HTS106_Node *next;      /* pointer to the next node */
   HTS106_Question *quest;         /* question applied at this node */
} HTS106_Node;

/* HTS106_Tree: List of decision trees in a model. */
typedef struct _HTS106_Tree {
   HTS106_Pattern *head;           /* pointer to the head of pattern list for this tree */
   struct _HTS106_Tree *next;      /* pointer to next tree */
   HTS106_Node *root;              /* root node of this tree */
   int state;                   /* state index of this tree */
} HTS106_Tree;

/* HTS106_Model: Set of PDFs, decision trees and questions. */
typedef struct _HTS106_Model {
   int vector_length;           /* vector length (include static and dynamic features) */
   int ntree;                   /* # of trees */
   int *npdf;                   /* # of PDFs at each tree */
   double ***pdf;               /* PDFs */
   HTS106_Tree *tree;              /* pointer to the list of trees */
   HTS106_Question *question;      /* pointer to the list of questions */
} HTS106_Model;

/* HTS106_Stream: Set of models and a window. */
typedef struct _HTS106_Stream {
   int vector_length;           /* vector_length (include static and dynamic features) */
   HTS106_Model *model;            /* models */
   HTS106_Window window;           /* window coefficients */
   HTS106_Boolean msd_flag;        /* flag for MSD */
   int interpolation_size;      /* # of models for interpolation */
} HTS106_Stream;

/* HTS106_ModelSet: Set of duration models, HMMs and GV models. */
typedef struct _HTS106_ModelSet {
   HTS106_Stream duration;         /* duration PDFs and trees */
   HTS106_Stream *stream;          /* parameter PDFs, trees and windows */
   HTS106_Stream *gv;              /* GV PDFs */
   HTS106_Model gv_switch;         /* GV switch */
   int nstate;                  /* # of HMM states */
   int nstream;                 /* # of stream */
} HTS106_ModelSet;

/*  ----------------------- model method --------------------------  */

/* HTS106_ModelSet_initialize: initialize model set */
void HTS106_ModelSet_initialize(HTS106_ModelSet * ms, int nstream);

/* HTS106_ModelSet_load_duration: load duration model and number of state */
HTS106_Boolean HTS106_ModelSet_load_duration(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int interpolation_size);

/* HTS106_ModelSet_load_parameter: load parameter model */
HTS106_Boolean HTS106_ModelSet_load_parameter(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, HTS106_File ** win_fp, int stream_index, HTS106_Boolean msd_flag, int window_size, int interpolation_size);

/* HTS106_ModelSet_load_gv: load GV model */
HTS106_Boolean HTS106_ModelSet_load_gv(HTS106_ModelSet * ms, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int stream_index, int interpolation_size);

/* HTS106_ModelSet_have_gv_tree: if context-dependent GV is used, return true */
HTS106_Boolean HTS106_ModelSet_have_gv_tree(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_load_gv_switch: load GV switch */
HTS106_Boolean HTS106_ModelSet_load_gv_switch(HTS106_ModelSet * ms, HTS106_File * fp);

/* HTS106_ModelSet_have_gv_switch: if GV switch is used, return true */
HTS106_Boolean HTS106_ModelSet_have_gv_switch(HTS106_ModelSet * ms);

/* HTS106_ModelSet_get_nstate: get number of state */
int HTS106_ModelSet_get_nstate(HTS106_ModelSet * ms);

/* HTS106_ModelSet_get_nstream: get number of stream */
int HTS106_ModelSet_get_nstream(HTS106_ModelSet * ms);

/* HTS106_ModelSet_get_vector_length: get vector length */
int HTS106_ModelSet_get_vector_length(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_is_msd: get MSD flag */
HTS106_Boolean HTS106_ModelSet_is_msd(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_get_window_size: get dynamic window size */
int HTS106_ModelSet_get_window_size(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_get_window_left_width: get left width of dynamic window */
int HTS106_ModelSet_get_window_left_width(HTS106_ModelSet * ms, int stream_index, int window_index);

/* HTS106_ModelSet_get_window_right_width: get right width of dynamic window */
int HTS106_ModelSet_get_window_right_width(HTS106_ModelSet * ms, int stream_index, int window_index);

/* HTS106_ModelSet_get_window_coefficient: get coefficient of dynamic window */
double HTS106_ModelSet_get_window_coefficient(HTS106_ModelSet * ms, int stream_index, int window_index, int coefficient_index);

/* HTS106_ModelSet_get_window_max_width: get max width of dynamic window */
int HTS106_ModelSet_get_window_max_width(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_get_duration_interpolation_size: get interpolation size (duration model) */
int HTS106_ModelSet_get_duration_interpolation_size(HTS106_ModelSet * ms);

/* HTS106_ModelSet_get_parameter_interpolation_size: get interpolation size (parameter model) */
int HTS106_ModelSet_get_parameter_interpolation_size(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_get_gv_interpolation_size: get interpolation size (GV model) */
int HTS106_ModelSet_get_gv_interpolation_size(HTS106_ModelSet * ms, int stream_index);

/* HTS106_ModelSet_use_gv: get GV flag */
HTS106_Boolean HTS106_ModelSet_use_gv(HTS106_ModelSet * ms, int index);

/* HTS106_ModelSet_get_duration_index: get index of duration tree and PDF */
void HTS106_ModelSet_get_duration_index(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, int *tree_index, int *pdf_index, int interpolation_index);

/* HTS106_ModelSet_get_duration: get duration using interpolation weight */
void HTS106_ModelSet_get_duration(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, double *mean, double *vari, double *iw);

/* HTS106_ModelSet_get_parameter_index: get index of parameter tree and PDF */
void HTS106_ModelSet_get_parameter_index(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, int *tree_index, int *pdf_index, int stream_index, int state_index, int interpolation_index);

/* HTS106_ModelSet_get_parameter: get parameter using interpolation weight */
void HTS106_ModelSet_get_parameter(HTS106_ModelSet * ms, char *string, const RHVoice_parsed_label_string* parsed, double *mean, double *vari, double *msd, int stream_index, int state_index, double *iw);

/* HTS106_ModelSet_get_gv: get GV using interpolation weight */
void HTS106_ModelSet_get_gv(HTS106_ModelSet * ms, char *string, double *mean, double *vari, int stream_index, double *iw);

/* HTS106_ModelSet_get_gv_switch: get GV switch */
HTS106_Boolean HTS106_ModelSet_get_gv_switch(HTS106_ModelSet * ms, char *string);

/* HTS106_ModelSet_clear: free model set */
void HTS106_ModelSet_clear(HTS106_ModelSet * ms);

/*  -------------------------- label ------------------------------  */

/* HTS106_LabelString: individual label string with time information */
typedef struct _HTS106_LabelString {
   struct _HTS106_LabelString *next;       /* pointer to next label string */
   char *name;                  /* label string */
   double start;                /* start frame specified in the given label */
   double end;                  /* end frame specified in the given label */
} HTS106_LabelString;

/* HTS106_Label: list of label strings */
typedef struct _HTS106_Label {
   HTS106_LabelString *head;       /* pointer to the head of label string */
   int size;                    /* # of label strings */
   HTS106_Boolean frame_flag;      /* flag for frame length modification */
   double speech_speed;         /* speech speed rate */
} HTS106_Label;

/*  ----------------------- label method --------------------------  */

/* HTS106_Label_initialize: initialize label */
void HTS106_Label_initialize(HTS106_Label * label);

/* HTS106_Label_load_from_fn: load label from file name */
void HTS106_Label_load_from_fn(HTS106_Label * label, int sampling_rate, int fperiod, char *fn);

/* HTS106_Label_load_from_fp: load label list from file pointer */
void HTS106_Label_load_from_fp(HTS106_Label * label, int sampling_rate, int fperiod, HTS106_File * fp);

/* HTS106_Label_load_from_string: load label from string */
void HTS106_Label_load_from_string(HTS106_Label * label, int sampling_rate, int fperiod, char *data);

/* HTS106_Label_load_from_string_list: load label list from string list */
void HTS106_Label_load_from_string_list(HTS106_Label * label, int sampling_rate, int fperiod, char **data, int size);

/* HTS106_Label_set_speech_speed: set speech speed rate */
void HTS106_Label_set_speech_speed(HTS106_Label * label, double f);

/* HTS106_Label_set_frame_specified_flag: set frame specified flag */
void HTS106_Label_set_frame_specified_flag(HTS106_Label * label, HTS106_Boolean i);

/* HTS106_Label_get_size: get number of label string */
int HTS106_Label_get_size(HTS106_Label * label);

/* HTS106_Label_get_string: get label string */
char *HTS106_Label_get_string(HTS106_Label * label, int string_index);

/* HTS106_Label_get_frame_specified_flag: get frame specified flag */
HTS106_Boolean HTS106_Label_get_frame_specified_flag(HTS106_Label * label);

/* HTS106_Label_get_start_frame: get start frame */
double HTS106_Label_get_start_frame(HTS106_Label * label, int string_index);

/* HTS106_Label_get_end_frame: get end frame */
double HTS106_Label_get_end_frame(HTS106_Label * label, int string_index);

/* HTS106_Label_get_speech_speed: get speech speed rate */
double HTS106_Label_get_speech_speed(HTS106_Label * label);

/* HTS106_Label_clear: free label */
void HTS106_Label_clear(HTS106_Label * label);

/*  -------------------------- sstream ----------------------------  */

/* HTS106_SStream: individual state stream */
typedef struct _HTS106_SStream {
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
   HTS106_Boolean *gv_switch;      /* GV flag sequence */
} HTS106_SStream;

/* HTS106_SStreamSet: set of state stream */
typedef struct _HTS106_SStreamSet {
   HTS106_SStream *sstream;        /* state streams */
   int nstream;                 /* # of streams */
   int nstate;                  /* # of states */
   int *duration;               /* duration sequence */
   int total_state;             /* total state */
   int total_frame;             /* total frame */
} HTS106_SStreamSet;

/*  ----------------------- sstream method ------------------------  */

/* HTS106_SStreamSet_initialize: initialize state stream set */
void HTS106_SStreamSet_initialize(HTS106_SStreamSet * sss);

/* HTS106_SStreamSet_create: parse label and determine state duration */
HTS106_Boolean HTS106_SStreamSet_create(HTS106_SStreamSet * sss, HTS106_ModelSet * ms, HTS106_Label * label, double *duration_iw, double **parameter_iw, double **gv_iw);

/* HTS106_SStreamSet_get_nstream: get number of stream */
int HTS106_SStreamSet_get_nstream(HTS106_SStreamSet * sss);

/* HTS106_SStreamSet_get_vector_length: get vector length */
int HTS106_SStreamSet_get_vector_length(HTS106_SStreamSet * sss, int stream_index);

/* HTS106_SStreamSet_is_msd: get MSD flag */
HTS106_Boolean HTS106_SStreamSet_is_msd(HTS106_SStreamSet * sss, int stream_index);

/* HTS106_SStreamSet_get_total_state: get total number of state */
int HTS106_SStreamSet_get_total_state(HTS106_SStreamSet * sss);

/* HTS106_SStreamSet_get_total_frame: get total number of frame */
int HTS106_SStreamSet_get_total_frame(HTS106_SStreamSet * sss);

/* HTS106_SStreamSet_get_msd: get msd parameter */
double HTS106_SStreamSet_get_msd(HTS106_SStreamSet * sss, int stream_index, int state_index);

/* HTS106_SStreamSet_window_size: get dynamic window size */
int HTS106_SStreamSet_get_window_size(HTS106_SStreamSet * sss, int stream_index);

/* HTS106_SStreamSet_get_window_left_width: get left width of dynamic window */
int HTS106_SStreamSet_get_window_left_width(HTS106_SStreamSet * sss, int stream_index, int window_index);

/* HTS106_SStreamSet_get_window_right_width: get right width of dynamic window */
int HTS106_SStreamSet_get_window_right_width(HTS106_SStreamSet * sss, int stream_index, int window_index);

/* HTS106_SStreamSet_get_window_coefficient: get coefficient of dynamic window */
double HTS106_SStreamSet_get_window_coefficient(HTS106_SStreamSet * sss, int stream_index, int window_index, int coefficient_index);

/* HTS106_SStreamSet_get_window_max_width: get max width of dynamic window */
int HTS106_SStreamSet_get_window_max_width(HTS106_SStreamSet * sss, int stream_index);

/* HTS106_SStreamSet_use_gv: get GV flag */
HTS106_Boolean HTS106_SStreamSet_use_gv(HTS106_SStreamSet * sss, int stream_index);

/* HTS106_SStreamSet_get_duration: get state duration */
int HTS106_SStreamSet_get_duration(HTS106_SStreamSet * sss, int state_index);

/* HTS106_SStreamSet_get_mean: get mean parameter */
double HTS106_SStreamSet_get_mean(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index);

/* HTS106_SStreamSet_set_mean: set mean parameter */
void HTS106_SStreamSet_set_mean(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index, double f);

/* HTS106_SStreamSet_get_vari: get variance parameter */
double HTS106_SStreamSet_get_vari(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index);

/* HTS106_SStreamSet_set_vari: set variance parameter */
void HTS106_SStreamSet_set_vari(HTS106_SStreamSet * sss, int stream_index, int state_index, int vector_index, double f);

/* HTS106_SStreamSet_get_gv_mean: get GV mean parameter */
double HTS106_SStreamSet_get_gv_mean(HTS106_SStreamSet * sss, int stream_index, int vector_index);

/* HTS106_SStreamSet_get_gv_mean: get GV variance parameter */
double HTS106_SStreamSet_get_gv_vari(HTS106_SStreamSet * sss, int stream_index, int vector_index);

/* HTS106_SStreamSet_set_gv_switch: set GV switch */
void HTS106_SStreamSet_set_gv_switch(HTS106_SStreamSet * sss, int stream_index, int state_index, HTS106_Boolean i);

/* HTS106_SStreamSet_get_gv_switch: get GV switch */
HTS106_Boolean HTS106_SStreamSet_get_gv_switch(HTS106_SStreamSet * sss, int stream_index, int state_index);

/* HTS106_SStreamSet_clear: free state stream set */
void HTS106_SStreamSet_clear(HTS106_SStreamSet * sss);

/*  -------------------------- pstream ----------------------------  */

/* HTS106_SMatrices: Matrices/Vectors used in the speech parameter generation algorithm. */
typedef struct _HTS106_SMatrices {
   double **mean;               /* mean vector sequence */
   double **ivar;               /* inverse diag variance sequence */
   double *g;                   /* vector used in the forward substitution */
   double **wuw;                /* W' U^-1 W  */
   double *wum;                 /* W' U^-1 mu */
} HTS106_SMatrices;

/* HTS106_PStream: Individual PDF stream. */
typedef struct _HTS106_PStream {
   int vector_length;           /* vector length (include static and dynamic features) */
   int static_length;           /* static features length */
   int length;                  /* stream length */
   int width;                   /* width of dynamic window */
   double **par;                /* output parameter vector */
   HTS106_SMatrices sm;            /* matrices for parameter generation */
   int win_size;                /* # of windows (static + deltas) */
   int *win_l_width;            /* left width of windows */
   int *win_r_width;            /* right width of windows */
   double **win_coefficient;    /* window coefficients */
   HTS106_Boolean *msd_flag;       /* Boolean sequence for MSD */
   double *gv_mean;             /* mean vector of GV */
   double *gv_vari;             /* variance vector of GV */
   HTS106_Boolean *gv_switch;      /* GV flag sequence */
   int gv_length;               /* frame length for GV calculation */
} HTS106_PStream;

/* HTS106_PStreamSet: Set of PDF streams. */
typedef struct _HTS106_PStreamSet {
   HTS106_PStream *pstream;        /* PDF streams */
   int nstream;                 /* # of PDF streams */
   int total_frame;             /* total frame */
} HTS106_PStreamSet;

/*  ----------------------- pstream method ------------------------  */

/* HTS106_PStreamSet_initialize: initialize parameter stream set */
void HTS106_PStreamSet_initialize(HTS106_PStreamSet * pss);

/* HTS106_PStreamSet_create: parameter generation using GV weight */
HTS106_Boolean HTS106_PStreamSet_create(HTS106_PStreamSet * pss, HTS106_SStreamSet * sss, double *msd_threshold, double *gv_weight);

/* HTS106_PStreamSet_get_nstream: get number of stream */
int HTS106_PStreamSet_get_nstream(HTS106_PStreamSet * pss);

/* HTS106_PStreamSet_get_static_length: get static features length */
int HTS106_PStreamSet_get_static_length(HTS106_PStreamSet * pss, int stream_index);

/* HTS106_PStreamSet_get_total_frame: get total number of frame */
int HTS106_PStreamSet_get_total_frame(HTS106_PStreamSet * pss);

/* HTS106_PStreamSet_get_parameter: get parameter */
double HTS106_PStreamSet_get_parameter(HTS106_PStreamSet * pss, int stream_index, int frame_index, int vector_index);

/* HTS106_PStreamSet_get_parameter_vector: get parameter vector */
double *HTS106_PStreamSet_get_parameter_vector(HTS106_PStreamSet * pss, int stream_index, int frame_index);

/* HTS106_PStreamSet_get_msd_flag: get generated MSD flag per frame */
HTS106_Boolean HTS106_PStreamSet_get_msd_flag(HTS106_PStreamSet * pss, int stream_index, int frame_index);

/* HTS106_PStreamSet_is_msd: get MSD flag */
HTS106_Boolean HTS106_PStreamSet_is_msd(HTS106_PStreamSet * pss, int stream_index);

/* HTS106_PStreamSet_clear: free parameter stream set */
void HTS106_PStreamSet_clear(HTS106_PStreamSet * pss);

/*  -------------------------- gstream ----------------------------  */

/* HTS106_GStream: Generated parameter stream. */
typedef struct _HTS106_GStream {
   int static_length;           /* static features length */
   double **par;                /* generated parameter */
} HTS106_GStream;

/* HTS106_GStreamSet: Set of generated parameter stream. */
typedef struct _HTS106_GStreamSet {
   int total_nsample;           /* total sample */
   int total_frame;             /* total frame */
   int nstream;                 /* # of streams */
   HTS106_GStream *gstream;        /* generated parameter streams */
   short *gspeech;              /* generated speech */
} HTS106_GStreamSet;

/*  ----------------------- gstream method ------------------------  */

/* HTS106_GStreamSet_initialize: initialize generated parameter stream set */
void HTS106_GStreamSet_initialize(HTS106_GStreamSet * gss);

/* HTS106_GStreamSet_create: generate speech */
HTS106_Boolean HTS106_GStreamSet_create(HTS106_GStreamSet * gss, HTS106_PStreamSet * pss, int stage, HTS106_Boolean use_log_gain, int sampling_rate, int fperiod, double alpha, double beta, HTS106_Boolean * stop, double volume, HTS106_Audio * audio);

/* HTS106_GStreamSet_get_total_nsample: get total number of sample */
int HTS106_GStreamSet_get_total_nsample(HTS106_GStreamSet * gss);

/* HTS106_GStreamSet_get_total_frame: get total number of frame */
int HTS106_GStreamSet_get_total_frame(HTS106_GStreamSet * gss);

/* HTS106_GStreamSet_get_static_length: get static features length */
int HTS106_GStreamSet_get_static_length(HTS106_GStreamSet * gss, int stream_index);

/* HTS106_GStreamSet_get_speech: get synthesized speech parameter */
short HTS106_GStreamSet_get_speech(HTS106_GStreamSet * gss, int sample_index);

/* HTS106_GStreamSet_get_parameter: get generated parameter */
double HTS106_GStreamSet_get_parameter(HTS106_GStreamSet * gss, int stream_index, int frame_index, int vector_index);

/* HTS106_GStreamSet_clear: free generated parameter stream set */
void HTS106_GStreamSet_clear(HTS106_GStreamSet * gss);

/*  -------------------------- engine -----------------------------  */

/* HTS106_Global: Global settings. */
typedef struct _HTS106_Global {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   HTS106_Boolean use_log_gain;    /* log gain flag (for LSP) */
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
   HTS106_Boolean stop;            /* stop flag */
   double volume;               /* volume */
} HTS106_Global;

/* HTS106_Engine: Engine itself. */
typedef struct _HTS106_Engine {
   HTS106_Global global;           /* global settings */
   HTS106_Audio audio;             /* audio output */
   HTS106_ModelSet ms;             /* set of duration models, HMMs and GV models */
   HTS106_Label label;             /* label */
   HTS106_SStreamSet sss;          /* set of state streams */
   HTS106_PStreamSet pss;          /* set of PDF streams */
   HTS106_GStreamSet gss;          /* set of generated parameter streams */
} HTS106_Engine;

/*  ----------------------- engine method -------------------------  */

/* HTS106_Engine_initialize: initialize engine */
void HTS106_Engine_initialize(HTS106_Engine * engine, int nstream);

/* HTS106_engine_load_duration_from_fn: load duration pdfs ,trees and number of state from file names */
HTS106_Boolean HTS106_Engine_load_duration_from_fn(HTS106_Engine * engine, char **pdf_fn, char **tree_fn, int interpolation_size);

/* HTS106_Engine_load_duration_from_fp: load duration pdfs, trees and number of state from file pointers */
HTS106_Boolean HTS106_Engine_load_duration_from_fp(HTS106_Engine * engine, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int interpolation_size);

/* HTS106_Engine_load_parameter_from_fn: load parameter pdfs, trees and windows from file names */
HTS106_Boolean HTS106_Engine_load_parameter_from_fn(HTS106_Engine * engine, char **pdf_fn, char **tree_fn, char **win_fn, int stream_index, HTS106_Boolean msd_flag, int window_size, int interpolation_size);

/* HTS106_Engine_load_parameter_from_fp: load parameter pdfs, trees and windows from file pointers */
HTS106_Boolean HTS106_Engine_load_parameter_from_fp(HTS106_Engine * engine, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, HTS106_File ** win_fp, int stream_index, HTS106_Boolean msd_flag, int window_size, int interpolation_size);

/* HTS106_Engine_load_gv_from_fn: load GV pdfs and trees from file names */
HTS106_Boolean HTS106_Engine_load_gv_from_fn(HTS106_Engine * engine, char **pdf_fn, char **tree_fn, int stream_index, int interpolation_size);

/* HTS106_Engine_load_gv_from_fp: load GV pdfs and trees from file pointers */
HTS106_Boolean HTS106_Engine_load_gv_from_fp(HTS106_Engine * engine, HTS106_File ** pdf_fp, HTS106_File ** tree_fp, int stream_index, int interpolation_size);

/* HTS106_Engine_load_gv_switch_from_fn: load GV switch from file names */
HTS106_Boolean HTS106_Engine_load_gv_switch_from_fn(HTS106_Engine * engine, char *fn);

/* HTS106_Engine_load_gv_switch_from_fp: load GV switch from file pointers */
HTS106_Boolean HTS106_Engine_load_gv_switch_from_fp(HTS106_Engine * engine, HTS106_File * fp);

/* HTS106_Engine_set_sampling_rate: set sampling rate */
void HTS106_Engine_set_sampling_rate(HTS106_Engine * engine, int i);

/* HTS106_Engine_get_sampling_rate: get sampling rate */
int HTS106_Engine_get_sampling_rate(HTS106_Engine * engine);

/* HTS106_Engine_set_fperiod: set frame shift */
void HTS106_Engine_set_fperiod(HTS106_Engine * engine, int i);

/* HTS106_Engine_get_fperiod: get frame shift */
int HTS106_Engine_get_fperiod(HTS106_Engine * engine);

/* HTS106_Engine_set_alpha: set alpha */
void HTS106_Engine_set_alpha(HTS106_Engine * engine, double f);

/* HTS106_Engine_set_gamma: set gamma (Gamma=-1/i: if i=0 then Gamma=0) */
void HTS106_Engine_set_gamma(HTS106_Engine * engine, int i);

/* HTS106_Engine_set_log_gain: set log gain flag (for LSP) */
void HTS106_Engine_set_log_gain(HTS106_Engine * engine, HTS106_Boolean i);

/* HTS106_Engine_set_beta: set beta */
void HTS106_Engine_set_beta(HTS106_Engine * engine, double f);

/* HTS106_Engine_set_audio_buff_size: set audio buffer size */
void HTS106_Engine_set_audio_buff_size(HTS106_Engine * engine, int i);

/* HTS106_Engine_get_audio_buff_size: get audio buffer size */
int HTS106_Engine_get_audio_buff_size(HTS106_Engine * engine);

/* HTS106_Egnine_set_msd_threshold: set MSD threshold */
void HTS106_Engine_set_msd_threshold(HTS106_Engine * engine, int stream_index, double f);

/* HTS106_Engine_set_duration_interpolation_weight: set interpolation weight for duration */
void HTS106_Engine_set_duration_interpolation_weight(HTS106_Engine * engine, int interpolation_index, double f);

/* HTS106_Engine_set_parameter_interpolation_weight: set interpolation weight for parameter */
void HTS106_Engine_set_parameter_interpolation_weight(HTS106_Engine * engine, int stream_index, int interpolation_index, double f);

/* HTS106_Engine_set_gv_interpolation_weight: set interpolation weight for GV */
void HTS106_Engine_set_gv_interpolation_weight(HTS106_Engine * engine, int stream_index, int interpolation_index, double f);

/* HTS106_Engine_set_gv_weight: set GV weight */
void HTS106_Engine_set_gv_weight(HTS106_Engine * engine, int stream_index, double f);

/* HTS106_Engine_set_stop_flag: set stop flag */
void HTS106_Engine_set_stop_flag(HTS106_Engine * engine, HTS106_Boolean b);

/* HTS106_Engine_set_volume: set volume */
void HTS106_Engine_set_volume(HTS106_Engine * engine, double f);

/* HTS106_Engine_get_total_state: get total number of state */
int HTS106_Engine_get_total_state(HTS106_Engine * engine);

/* HTS106_Engine_set_state_mean: set mean value of state */
void HTS106_Engine_set_state_mean(HTS106_Engine * engine, int stream_index, int state_index, int vector_index, double f);

/* HTS106_Engine_get_state_mean: get mean value of state */
double HTS106_Engine_get_state_mean(HTS106_Engine * engine, int stream_index, int state_index, int vector_index);

/* HTS106_Engine_get_state_duration: get state duration */
int HTS106_Engine_get_state_duration(HTS106_Engine * engine, int state_index);

/* HTS106_Engine_get_nstream: get number of stream */
int HTS106_Engine_get_nstream(HTS106_Engine * engine);

/* HTS106_Engine_get_nstate: get number of state */
int HTS106_Engine_get_nstate(HTS106_Engine * engine);

/* HTS106_Engine_load_label_from_fn: load label from file pointer */
void HTS106_Engine_load_label_from_fn(HTS106_Engine * engine, char *fn);

/* HTS106_Engine_load_label_from_fp: load label from file name */
void HTS106_Engine_load_label_from_fp(HTS106_Engine * engine, HTS106_File * fp);

/* HTS106_Engine_load_label_from_string: load label from string */
void HTS106_Engine_load_label_from_string(HTS106_Engine * engine, char *data);

/* HTS106_Engine_load_label_from_string_list: load label from string list */
void HTS106_Engine_load_label_from_string_list(HTS106_Engine * engine, char **data, int size);

/* HTS106_Engine_create_sstream: parse label and determine state duration */
HTS106_Boolean HTS106_Engine_create_sstream(HTS106_Engine * engine);

/* HTS106_Engine_create_pstream: generate speech parameter vector sequence */
HTS106_Boolean HTS106_Engine_create_pstream(HTS106_Engine * engine);

/* HTS106_Engine_create_gstream: synthesis speech */
HTS106_Boolean HTS106_Engine_create_gstream(HTS106_Engine * engine);

/* HTS106_Engine_save_information: output trace information */
void HTS106_Engine_save_information(HTS106_Engine * engine, HTS106_File * fp);

/* HTS106_Engine_save_label: output label with time */
void HTS106_Engine_save_label(HTS106_Engine * engine, HTS106_File * fp);

/* HTS106_Engine_save_generated_parameter: output generated parameter */
void HTS106_Engine_save_generated_parameter(HTS106_Engine * engine, HTS106_File * fp, int stream_index);

/* HTS106_Engine_save_generated_speech: output generated speech */
void HTS106_Engine_save_generated_speech(HTS106_Engine * engine, HTS106_File * fp);

/* HTS106_Engine_save_riff: output RIFF format file */
void HTS106_Engine_save_riff(HTS106_Engine * engine, HTS106_File * wavfp);

/* HTS106_Engine_refresh: free memory per one time synthesis */
void HTS106_Engine_refresh(HTS106_Engine * engine);

/* HTS106_Engine_clear: free engine */
void HTS106_Engine_clear(HTS106_Engine * engine);

/*  -------------------------- vocoder ----------------------------  */

/* HTS106_Vocoder: structure for setting of vocoder */
typedef struct _HTS106_Vocoder {
   int stage;                   /* Gamma=-1/stage: if stage=0 then Gamma=0 */
   double gamma;                /* Gamma */
   HTS106_Boolean use_log_gain;    /* log gain flag (for LSP) */
   int fprd;                    /* frame shift */
   int iprd;                    /* interpolation period */
   int seed;                    /* seed of random generator */
   unsigned long next;          /* temporary variable for random generator */
   HTS106_Boolean gauss;           /* flag to use Gaussian noise */
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
} HTS106_Vocoder;

/*  ----------------------- vocoder method ------------------------  */

/* HTS106_Vocoder_initialize: initialize vocoder */
void HTS106_Vocoder_initialize(HTS106_Vocoder * v, const int m, const int stage, HTS106_Boolean use_log_gain, const int rate, const int fperiod);

/* HTS106_Vocoder_synthesize: pulse/noise excitation and MLSA/MGLSA filster based waveform synthesis */
void HTS106_Vocoder_synthesize(HTS106_Vocoder * v, const int m, double lf0, double *spectrum, const int nlpf, double *lpf, double alpha, double beta, double volume, short *rawdata, HTS106_Audio * audio);

/* HTS106_Vocoder_postfilter_mcp: postfilter for MCP */
void HTS106_Vocoder_postfilter_mcp(HTS106_Vocoder * v, double *mcp, const int m, double alpha, double beta);

/* HTS106_Vocoder_clear: clear vocoder */
void HTS106_Vocoder_clear(HTS106_Vocoder * v);

HTS106_ENGINE_H_END;

#endif                          /* !HTS106_ENGINE_H */
