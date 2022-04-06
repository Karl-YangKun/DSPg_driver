#ifndef DSPG_H_
#define DSPG_H_

#include <stdlib.h>

typedef bool (*write_data)(char *data,uint16 data_size);
typedef bool (*read_data)(char *data,uint16 *data_size);
typedef void (*delay)(uint16 ms);




typedef struct
{
    write_data Write;

    read_data Read;

    delay Delay;
} Interface_t;

typedef struct
{
    const uint8 *fw;
    const uint8 *use_case[2];
} File_Source_t;

typedef enum
{
    dspg_idle,
    dspg_voice_call
} DSPg_Usecase_t;

void DSPg_SetInterface(Interface_t inf);

bool DSPg_Init(File_Source_t source);

bool DSPg_SetMode(DSPg_Usecase_t mode);

#endif /* DSPG_H_ */