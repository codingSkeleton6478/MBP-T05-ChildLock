/**
 * @file F04_RearDoorOpenBlockHandler.h
 * @brief Public API for F-04 RearDoorOpenBlockHandler.
 *
 * @details This module handles UC-4 behavior. When a rear inner-handle open
 *          event occurs while child lock is ON, the request is blocked and a
 *          driver notice can be emitted. Door feedback is checked to detect a
 *          block failure condition.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */

#ifndef F04_REAR_DOOR_OPEN_BLOCK_HANDLER_H
#define F04_REAR_DOOR_OPEN_BLOCK_HANDLER_H

#include <stdbool.h>

#include "childlock_types.h"

/**
 * @brief Door-state feedback classification used by F-04.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
typedef enum
{
    RDOBH_DOOR_FEEDBACK_CLOSED = 0U,   /**< Door remains closed. */
    RDOBH_DOOR_FEEDBACK_OPENED = 1U,   /**< Door is reported as opened. */
    RDOBH_DOOR_FEEDBACK_UNKNOWN = 2U   /**< Door feedback is unavailable/invalid. */
} RearDoorOpenBlockHandler_DoorFeedback_t;

/**
 * @brief Driver notice selected by F-04.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
typedef enum
{
    RDOBH_DRIVER_NOTICE_NONE = 0U,            /**< No notice required. */
    RDOBH_DRIVER_NOTICE_BLOCKED = 1U,         /**< Rear-door open attempt was blocked. */
    RDOBH_DRIVER_NOTICE_BLOCK_FAILURE = 2U    /**< Block command failed or mismatch detected. */
} RearDoorOpenBlockHandler_DriverNotice_t;

/**
 * @brief F-04 handling result classification.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
typedef enum
{
    RDOBH_RESULT_NO_EVENT = 0U,         /**< No rear inner-handle event to process. */
    RDOBH_RESULT_ALLOWED = 1U,          /**< Open request was allowed (CL OFF). */
    RDOBH_RESULT_BLOCKED = 2U,          /**< Open request was blocked (CL ON). */
    RDOBH_RESULT_BLOCK_FAILED = 3U,     /**< Block intent existed but door opened/mismatch. */
    RDOBH_RESULT_SAFE_BLOCKED = 4U      /**< Invalid CL state handled as safe block. */
} RearDoorOpenBlockHandler_BlockResult_t;

/**
 * @brief Inputs provided to the F-04 handler.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
typedef struct
{
    bool rearDoorInnerHandleEvent; /**< true when rear inner-handle pull event is detected. */
    ChildLockState_t currentClState; /**< Current child lock state. */
    RearDoorOpenBlockHandler_DoorFeedback_t doorStateFeedback; /**< Door state feedback. */
} RearDoorOpenBlockHandler_Input_t;

/**
 * @brief Outputs produced by F-04.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
typedef struct
{
    bool openRequestBlock; /**< true when rear door open request must be blocked. */
    RearDoorOpenBlockHandler_DriverNotice_t driverNotice; /**< Notice requested for the driver. */
    RearDoorOpenBlockHandler_BlockResult_t blockResult; /**< F-04 handling result. */
} RearDoorOpenBlockHandler_Output_t;

/**
 * @brief Handles UC-4 rear-door inner-handle open-block logic.
 *
 * @details If no event is present, no action is taken. If an event is present
 *          and child lock is OFF, opening is allowed. If an event is present
 *          and child lock is ON, opening is blocked and feedback is checked for
 *          mismatch/failure. Invalid child lock state is treated as fail-safe
 *          block behavior.
 *
 * @param[in] input Pointer to current input signals.
 * @param[out] output Pointer receiving F-04 outputs.
 * @return true when processing completed, otherwise false.
 *
 * @traceability SDD-F-04
 * @req_id REQ-SW-CL-004
 * @asil ASIL-TBD
 * @note Related UC: UC-4
 */
bool RearDoorOpenBlockHandler_HandleEvent(
    const RearDoorOpenBlockHandler_Input_t *input,
    RearDoorOpenBlockHandler_Output_t *output);

#endif /* F04_REAR_DOOR_OPEN_BLOCK_HANDLER_H */
