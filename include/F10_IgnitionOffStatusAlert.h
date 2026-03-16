/**
 * @file F10_IgnitionOffStatusAlert.h
 * @brief Public API for F-10 IgnitionOffStatusAlert.
 *
 * @details This module evaluates an IGN OFF event and emits at most one
 *          summary status alert for UC-7. The current child lock state is
 *          obtained through an injected callback so the module remains small,
 *          deterministic, and free of dynamic allocation.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */

#ifndef F10_IGNITION_OFF_STATUS_ALERT_H
#define F10_IGNITION_OFF_STATUS_ALERT_H

#include <stdbool.h>

#include "childlock_types.h"

/**
 * @brief Summary message selected by F-10.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef enum
{
    IOA_STATUS_MESSAGE_NONE = 0U,            /**< No summary alert emitted. */
    IOA_STATUS_MESSAGE_CL_ON_SUMMARY = 1U,   /**< Child lock ON summary alert. */
    IOA_STATUS_MESSAGE_QUERY_FALLBACK = 2U   /**< Fallback when state query fails. */
} IgnitionOffStatusAlert_Message_t;

/**
 * @brief Alert priority selected by F-10.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef enum
{
    IOA_ALERT_PRIORITY_NONE = 0U,   /**< No alert priority. */
    IOA_ALERT_PRIORITY_STATUS = 1U  /**< Status summary priority. */
} IgnitionOffStatusAlert_Priority_t;

/**
 * @brief Callback used to query the current child lock state.
 *
 * @param[out] state Pointer receiving the current child lock state.
 * @return true if the state query succeeded, otherwise false.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef bool (*IgnitionOffStatusAlert_QueryStateCallback_t)(ChildLockState_t *state);

/**
 * @brief Injected dependencies for F-10.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef struct
{
    IgnitionOffStatusAlert_QueryStateCallback_t queryChildLockState; /**< State query hook. */
} IgnitionOffStatusAlert_Config_t;

/**
 * @brief Inputs provided to the IGN OFF handler.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef struct
{
    bool ignitionOffEvent;   /**< true when IGN OFF has occurred. */
    bool activeHmiAlert;     /**< true when another HMI alert is already active. */
    bool hmiHealthy;         /**< true when HMI is available for status output. */
} IgnitionOffStatusAlert_Input_t;

/**
 * @brief Output produced by the IGN OFF handler.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef struct
{
    IgnitionOffStatusAlert_Message_t statusSummaryMessage; /**< Selected summary message. */
    IgnitionOffStatusAlert_Priority_t alertPriority;       /**< Selected alert priority. */
    bool eventLog;                                         /**< true when a loggable alert event occurred. */
    bool alertIssued;                                      /**< true when a summary alert should be shown. */
} IgnitionOffStatusAlert_Output_t;

/**
 * @brief Runtime context for F-10.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
typedef struct
{
    IgnitionOffStatusAlert_Config_t config; /**< Injected dependencies. */
    bool isInitialized;                     /**< Defensive initialization guard. */
    bool alertAlreadyIssued;                /**< Alert-once policy state. */
} IgnitionOffStatusAlert_t;

/**
 * @brief Initializes the F-10 context and resets alert-once state.
 *
 * @param[out] alert Pointer to the runtime context.
 * @param[in] config Pointer to the dependency configuration.
 * @return true if initialization succeeded, otherwise false.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
bool IgnitionOffStatusAlert_Init(IgnitionOffStatusAlert_t *alert,
                                 const IgnitionOffStatusAlert_Config_t *config);

/**
 * @brief Handles IGN OFF status alert generation for UC-7.
 *
 * @param[in] alert Pointer to the initialized runtime context.
 * @param[in] input Pointer to the current IGN OFF handling inputs.
 * @param[out] output Pointer receiving the selected alert output.
 * @return true if the request was processed, otherwise false.
 *
 * @traceability SDD-F-10
 * @asil ASIL-TBD
 * @note Related UC: UC-7
 */
bool IgnitionOffStatusAlert_HandleEvent(IgnitionOffStatusAlert_t *alert,
                                        const IgnitionOffStatusAlert_Input_t *input,
                                        IgnitionOffStatusAlert_Output_t *output);

#endif /* F10_IGNITION_OFF_STATUS_ALERT_H */
