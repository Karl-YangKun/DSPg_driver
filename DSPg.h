/*******The interface is for dspg system. After initialization in application, it will be used when dspg system run.**********/

#ifndef DSPG_H_
#define DSPG_H_

#include "DSPg_config.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    //write data 
    bool (*Write)(const uint8 *data,uint32 data_size);
    //read data 
    bool (*Read)(uint8 *data,uint16 data_size);
    //time delay 
    void (*Delay)(uint16 ms);
    //pull io high or low 
    void (*Set_IO)(dspg_io_t io, bool high);
    //print log
    void (*Debug)( const char *format,uint8 n_args, va_list args);
    
    //the type of communication between host and dspg
    comm_inf_t comm;
    //frameware data and size
    source_t fw;
    //usecase file data and size
    source_t use_case[dspg_max_mode];

} interface_t;

//Initialize the dspg system
bool DSPg_Init(interface_t interfaces);
//Enter usecase or exit 
bool DSPg_SetMode(usecase_t mode);

#endif /* DSPG_H_ */
