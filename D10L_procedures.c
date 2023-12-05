#include "D10L_utils.h"

static bool D10L_Init(const source_t fw)
{
    DBM_DEBUG("--Initialize D10L");

    //Reset D10L
    DSPg_SetIO(reset_io,FALSE);
    DELAY(DELAY_IN_RESET);
    DSPg_SetIO(reset_io,TRUE);
    // time for PLL to stable (D10)
    DELAY(DELAY_AFTER_RESET);

    if(!D10L_Sync())
    {
        DBM_DEBUG("Sync fail!");
        return FALSE;
    }

    D10L_PrebootRequest();

    if(!D10L_LoadFile(fw,4))
    {
        DBM_DEBUG("Loading fw fail!");
        return FALSE;
    }
    DELAY(20);
    D10L_AfterPowerUp();
#if (!EVENTS_TO_HOST)
    D10L_AfterPowerUp();
#endif
    return TRUE;
}

static bool D10L_EnterHibernate(void)
{
    WRITE_REG(0x1,6,r16d16);

    DBM_DEBUG("--D10L_EnterHibernate,enter hibernate mode");
    return TRUE;
}

static bool D10L_ExitHibernate(void)
{
    DBM_DEBUG("--D10L_ExitHibernate,exit hibernate mode %d");
    D10L_ReadID();
    return TRUE;
}

static bool D10L_EnterUseCase(usecase_t ucase,source_t model,source_t asrp)
{
    DBM_DEBUG("--D10L_EnterUseCase,enter enum:usecase_t:%d ",ucase);

    D10L_ModelLoading(model,WWE_CYBERON);
    switch (ucase)
    {
    case dspg_voice_call:
        D10L_UsecaseBargeInEnter(asrp,WWE_CYBERON);
        break;

    case dspg_voice_rcu:
        D10L_UsecaseBargeInEnter(asrp,WWE_CYBERON);
        break;
    
    case dspg_low_power:
        D10L_UsecaseLpEnter(WWE_CYBERON);
        break;

    default:
        DBM_DEBUG("--D10L_EnterUseCase,can't switch to enum:usecase_t:%d ",ucase);
        break;
    }

    if(asrp.data !=NULL)
    {
        uint32 vers[3];
        DBM_DEBUG("--Read ASRP version");
        READ_REG(0x100,&vers[0],r16d16);
        READ_REG(0x101,&vers[1],r16d16);
        READ_REG(0x104,&vers[2],r16d16);
        DBM_DEBUG("--The version: %x.%x.%x", vers[0],vers[1],vers[2]);
    }
    
    D10L_CheckError();

    return TRUE;
}

static bool D10L_ExitUseCase(usecase_t ucase)
{
//    source_t model={0,0};

    switch (ucase)
    {
    case dspg_voice_call:
        D10L_UsecaseBrageInExit();
        break;

    case dspg_voice_rcu:
        D10L_UsecaseNrExit(FALSE);
        break;
    
    case dspg_low_power:
        D10L_UsecaseLpExit();
        break;

    default:
        DBM_DEBUG("--D10L_ExitUseCase,can't exit enum:usecase_t:%d ",ucase);
        break;
    }

    
    // D10L_ModelLoading(model,WWE_NONE);

    DBM_DEBUG("--D10L_ExitUseCase,enter idle mode");
    return TRUE;
}

static trigger_word_t D10L_IntHandler(void)
{
    uint32 interrupt_events = 0;
    trigger_word_t t_word = no_trigger;

    READ_REG(DETECTION_AND_SYSTEM_EVENTS_STATUS_14,&interrupt_events,r16d16);
    WRITE_REG( DETECTION_AND_SYSTEM_EVENTS_STATUS_14, interrupt_events,r16d16);

    DBM_DEBUG("--Event detected 0x%x",interrupt_events);

    while (interrupt_events != 0)
    {
        if (interrupt_events & (1 << 15))
        {
            D10L_CheckError();
            interrupt_events &=  (~ERROR_EVENT);
            return TRUE;
        }
        else if (interrupt_events & (1 << 14))
        {
            D10L_CheckError();
            interrupt_events &=  (~WARNING_EVENT);
        }
        else if (interrupt_events & (1 << 0))
        {
            DBM_DEBUG("FW POWER UP COMPLETED");
            // even for multy thread continue here init in interrupt handler
            D10L_AfterPowerUp();
            interrupt_events &=  (~PWRUP_COMP);
        }
        else if (interrupt_events & (1 << 4))
        {
//            LOG_CNTRL("AEP EVENT\n");
            interrupt_events &=  (~AEP_DET);
//            aep_event(chip);
        }
        else if (interrupt_events & (1 << 7))
        {
//            LOG_CNTRL("SENSOR_DET\n");
            interrupt_events &=  (~SENSOR_DET);
//            check_sensor_event(chip);
        }
        else
        {
            t_word = D10L_VoiceTrigger(interrupt_events);
            interrupt_events &=  (~interrupt_events);
            if(interrupt_events ==0)
            {
                READ_REG(DETECTION_AND_SYSTEM_EVENTS_STATUS_14,&interrupt_events,r16d16);
                WRITE_REG( DETECTION_AND_SYSTEM_EVENTS_STATUS_14, interrupt_events,r16d16);
            }
        }
    }

    return t_word;
}

/*************************interface of dspg system******************************/
handler_t  handler=
                {
                    .init = D10L_Init,
                    .load_uc = D10L_EnterUseCase,
                    .exit_uc  = D10L_ExitUseCase,
                    .enter_hibernate = D10L_EnterHibernate,
                    .exit_hibernate = D10L_ExitHibernate,
                    .int_handler = D10L_IntHandler,
                };

handler_t DSPg_GetHander(void)
{
    return handler;
}
/********************************************************************************/
