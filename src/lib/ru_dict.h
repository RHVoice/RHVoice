#ifndef RU_DICT_H
#define RU_DICT_H
typedef struct ru_dict_entry_struct {const char *word; signed char stress;} ru_dict_entry;
extern const ru_dict_entry ru_dict[];
#define ru_dict_size 98749
#endif
