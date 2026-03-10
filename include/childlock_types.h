/**
 * @file    childlock_types.h
 * @brief   Electronic Child Lock System - Common Types, Enumerations, and Constants
 *
 * @details Defines all shared data types, enumerations, and compile-time constants
 *          used across the Child Lock software modules (F-01 ~ F-10).
 *          No dynamic memory allocation. All types are fixed-size for embedded use.
 *
 * @version 1.0.0
 * @date    2026-03-11
 * @author  AI Model: Gemini 3.5 Pro
 * @copyright Synetics 20 CopyrightⓒSynetics_
 *
 * @req_id  REQ-SW-CL-000
 * @asil    ASIL B (used by safety-critical modules)
 * @traceability DD-CL-000
 *
 * @note    Complies with MISRA C:2012. No use of dynamic memory.
 * @note    ISO 26262 Part 6 - Software-level requirements.
 */

#ifndef CHILDLOCK_TYPES_H
#define CHILDLOCK_TYPES_H

#include <stdint.h>  /* uint8_t, uint16_t, uint32_t  */
#include <stdbool.h> /* bool, true, false              */

/* =========================================================================
 * Section 1: Safety-Related Constants (Design Parameters)
 * ========================================================================= */

/** @brief Vehicle speed threshold above which Unlock is inhibited (km/h). @req_id REQ-SW-CL-002 */
#define UNLOCK_INHIBIT_SPEED_KMH       (3U)

/** @brief Vehicle speed threshold for automatic Child Lock activation (km/h). */
#define AUTO_LOCK_SPEED_THRESHOLD_KMH  (3U)

/** @brief Maximum number of Door ECU command retransmissions. @req_id REQ-SW-CL-003 */
#define MAX_ECU_RETRY_COUNT            (3U)

/** @brief HMI alert display limit per event (once-only policy). */
#define HMI_ALERT_MAX_COUNT            (1U)

/** @brief Speed signal validity timeout in milliseconds. */
#define SPEED_SIGNAL_TIMEOUT_MS        (500U)

/* =========================================================================
 * Section 2: Rear Risk Evaluation Constants (F-07)
 * ========================================================================= */

/** @brief Distance threshold below which an approaching object is considered dangerous (m). @req_id REQ-SW-ESA-001 */
#define REAR_RISK_DIST_THRESHOLD_M         (3.0f)

/** @brief Relative speed threshold above which an approach is considered dangerous (m/s). */
#define REAR_RISK_RELSPEED_THRESHOLD_MPS   (2.0f)

/** @brief Duration (ms) approaching object must persist to confirm High Risk (hysteresis ON). */
#define REAR_RISK_CONFIRM_DURATION_MS      (500U)

/** @brief Duration (ms) safe condition must persist to clear High Risk (hysteresis OFF). */
#define REAR_RISK_CLEAR_DURATION_MS        (1000U)

/** @brief Minimum sensor confidence level to trust measurement (0.0f ~ 1.0f). */
#define REAR_RISK_SENSOR_CONFIDENCE_MIN    (0.7f)

/** @brief Maximum valid distance value from rear sensor (m). Values above = invalid. */
#define REAR_RISK_DIST_MAX_VALID_M         (50.0f)

/** @brief Maximum valid relative speed value (m/s). Values above = invalid. */
#define REAR_RISK_RELSPEED_MAX_VALID_MPS   (50.0f)

/* =========================================================================
 * Section 3: Enumerations
 * ========================================================================= */

/**
 * @brief Child Lock state enumeration.
 * @note  Maps directly to the hardware/Door ECU lock state.
 */
typedef enum
{
    CL_STATE_OFF = 0U, /**< Child Lock is deactivated (doors can be opened from inside). */
    CL_STATE_ON  = 1U  /**< Child Lock is activated   (inside handles are blocked).      */
} ChildLockState_t;

/**
 * @brief Safe state policy enumeration, triggered by failure conditions.
 * @traceability DD-CL-010
 */
typedef enum
{
    SAFE_STATE_NONE               = 0U, /**< Normal operation; no safe state active.          */
    SAFE_STATE_LOCKED             = 1U, /**< CL forced ON due to sensor/speed fault.           */
    SAFE_STATE_EMERGENCY_RELEASED = 2U, /**< CL forced OFF due to valid crash detection.       */
    SAFE_STATE_RESTORE            = 3U  /**< State restored from NVM after IGN reset.          */
} SafeState_t;

/**
 * @brief Risk level returned by F-07 RearRiskEvaluation.
 * @req_id REQ-SW-ESA-001
 */
typedef enum
{
    RISK_LEVEL_NONE   = 0U, /**< No detected approaching object.    */
    RISK_LEVEL_LOW    = 1U, /**< Object detected but not dangerous. */
    RISK_LEVEL_MEDIUM = 2U, /**< Object approaching – monitor.      */
    RISK_LEVEL_HIGH   = 3U  /**< Imminent danger – trigger action.  */
} RiskLevel_t;

/**
 * @brief Warning message identifier for HMI output.
 * @req_id REQ-SW-ESA-002
 */
typedef enum
{
    WARNING_MSG_NONE              = 0U, /**< No warning to display.                         */
    WARNING_MSG_REAR_RISK_HIGH    = 1U, /**< "Rear object approaching! Do not open door."   */
    WARNING_MSG_CHILD_LOCK_ON     = 2U, /**< "Child Lock activated automatically."          */
    WARNING_MSG_SENSOR_FAULT      = 3U, /**< "Rear sensor error. Child Lock maintained."    */
    WARNING_MSG_CL_ALREADY_ON     = 4U  /**< "Child Lock already active. Warning issued."   */
} WarningMsgId_t;

/**
 * @brief Ignition state enumeration.
 */
typedef enum
{
    IGN_STATE_OFF = 0U, /**< Ignition / power is OFF. */
    IGN_STATE_ON  = 1U  /**< Ignition / power is ON.  */
} IgnitionState_t;

/**
 * @brief Fault flag bitmask for module-level fault reporting.
 * @note  Multiple flags can be OR-combined.
 */
typedef enum
{
    FAULT_FLAG_NONE          = 0x00U, /**< No fault detected.                */
    FAULT_FLAG_SPEED_FAULT   = 0x01U, /**< Vehicle speed signal invalid.     */
    FAULT_FLAG_REAR_SENSOR   = 0x02U, /**< Rear sensor health error.         */
    FAULT_FLAG_ECU_ACK_FAIL  = 0x04U, /**< Door ECU ACK not received.        */
    FAULT_FLAG_HMI_FAULT     = 0x08U, /**< HMI channel error.                */
    FAULT_FLAG_NVM_FAULT     = 0x10U  /**< NVM read/write error.             */
} FaultFlag_t;

/* =========================================================================
 * Section 4: Shared Data Structures
 * ========================================================================= */

/**
 * @brief Input data structure for F-07 RearRiskEvaluation.
 * @req_id REQ-SW-ESA-001
 * @asil   ASIL B
 */
typedef struct
{
    float    distanceM;       /**< Distance to rear object (m). Range: 0 ~ REAR_RISK_DIST_MAX_VALID_M. */
    float    relSpeedMps;     /**< Relative speed of object (m/s, positive = approaching).             */
    bool     sensorHealth;    /**< true = sensor operating normally, false = fault detected.           */
    float    sensorConfidence;/**< Sensor confidence factor (0.0 ~ 1.0).                              */
    uint32_t timestampMs;     /**< Current system timestamp in milliseconds (for duration tracking).   */
} RearRiskInput_t;

/**
 * @brief Output data structure for F-07 RearRiskEvaluation.
 * @req_id REQ-SW-ESA-001
 * @asil   ASIL B
 */
typedef struct
{
    bool        riskHigh;   /**< true = risk level is HIGH (triggers F-08 action).       */
    RiskLevel_t riskLevel;  /**< Numeric risk level classification.                      */
    bool        riskValid;  /**< true = evaluation result is valid (sensor OK).          */
    FaultFlag_t faultFlag;  /**< Fault flag; FAULT_FLAG_REAR_SENSOR if sensor unhealthy. */
} RearRiskOutput_t;

/**
 * @brief Input data structure for F-08 RearRiskProtectionController.
 * @req_id REQ-SW-ESA-002
 * @asil   ASIL B
 */
typedef struct
{
    bool             riskHigh;      /**< Risk assessment result from F-07.               */
    bool             riskValid;     /**< Validity flag from F-07.                        */
    ChildLockState_t currentCLState;/**< Current Child Lock state.                       */
    IgnitionState_t  ignitionState; /**< Current ignition state.                         */
} RearRiskProtectionInput_t;

/**
 * @brief Output data structure for F-08 RearRiskProtectionController.
 * @req_id REQ-SW-ESA-002
 * @asil   ASIL B
 */
typedef struct
{
    ChildLockState_t targetCLState;   /**< Desired Child Lock state to apply.             */
    WarningMsgId_t   warningMsgId;    /**< HMI warning message identifier.                */
    bool             warningSound;    /**< true = trigger audible warning sound.           */
    bool             clChanged;       /**< true = CL state was changed by this function.  */
} RearRiskProtectionOutput_t;

#endif /* CHILDLOCK_TYPES_H */
