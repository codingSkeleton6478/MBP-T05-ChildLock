/**
 * @file    F06_HmiAndEventLogger.c
 * @brief   F-06: HMI and Event Logger - Implementation
 *
 * @version 1.1.0
 * @date    2026-03-14
 * @author  AI Model: Gemini
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id      REQ-SW-CL-004
 * @asil        ASIL B
 * @traceability DD-CL-F06
 */

#include "F06_HmiAndEventLogger.h"

/* =========================================================================
 * Private Helper - Config Validation
 * ========================================================================= */

static bool hasValidConfig(const F06_HmiLoggerConfig_t * const config)
{
    bool isValid = false;

    if (config != NULL)
    {
        isValid = ((config->displayCb != NULL) &&
                   (config->logCb != NULL) &&
                   (config->dtcCb != NULL));
    }

    return isValid;
}

/* =========================================================================
 * Public API Implementation
 * ========================================================================= */

// cppcheck-suppress unusedFunction
bool F06_HmiAndEventLogger_Init(
    F06_HmiLoggerContext_t      * const ctx,
    const F06_HmiLoggerConfig_t * const config)
{
    bool isInitSuccess = false;

    if (ctx != NULL)
    {
        ctx->isInitialized = false;

        if (hasValidConfig(config) == true)
        {
            ctx->config = *config;
            ctx->isInitialized = true;
            isInitSuccess = true;
        }
    }

    return isInitSuccess;
}

// cppcheck-suppress unusedFunction
void F06_HmiAndEventLogger_Process(
    const F06_HmiLoggerContext_t * const ctx,
    const WarningMsgId_t                 msgId,
    const bool                           soundReq,
    const F06_EventType_t                evtType,
    const FaultFlag_t                    fault,
    const ChildLockState_t               currState)
{
    if ((ctx == NULL) || (ctx->isInitialized == false))
    {
        return;
    }

    if (msgId != WARNING_MSG_NONE)
    {
        ctx->config.displayCb(msgId, soundReq);
    }

    if (evtType != F06_EVENT_TYPE_NONE)
    {
        ctx->config.logCb(evtType, currState);
    }

    if (fault != FAULT_FLAG_NONE)
    {
        ctx->config.dtcCb(fault);
    }
}