
#include "DSPg.h"
#include "DSPg_procedures.h"

handler_t handler;


bool DSPg_Init(File_Source_t source)
{
    handler=DSPg_GetHander();

    if(!handler.init()) return FALSE;

    if(!handler.load_fw()) return FALSE;
        
    if(!handler.defalut_config()) return FALSE;
    
    return TRUE;
}

bool DSPg_SetMode(DSPg_Usecase_t mode)
{
    if(mode != idle)
        return handler.load_uc(mode);
    else
        return handler.exit_uc();
}