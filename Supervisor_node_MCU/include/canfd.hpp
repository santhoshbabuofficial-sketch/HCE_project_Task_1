#ifndef CANFD_HPP
#define CANFD_HPP

/**
 * ============================================================
 * CANFD PROTOCOL LAYER
 * ============================================================
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Handle CAN RX/TX
 * - Decode system messages
 * - Route data to application modules
 * - Send control commands (START / STOP / E-STOP)
 *
 * RULE:
 * - No business logic here
 * - Only protocol handling + dispatch
 * ============================================================
 */

#include <cstdint>

class CanFd
{
public:
    /**
     * @brief Initialize CAN peripheral and filters
     * @return true if success
     */
    static bool Init();

    /**
     * @brief Poll or wait for CAN messages (thread loop entry)
     */
    static void ProcessRx();

    /**
     * @brief Send START_MOTOR command (0x400)
     */
    static void SendStartMotor();

    /**
     * @brief Send STOP_MOTOR command (0x100)
     */
    static void SendStopMotor();

    /**
     * @brief Send E-STOP broadcast (0x101)
     */
    static void SendEStopBroadcast();

    /* ============================================================
     * RX QUEUE DEPTH (NO MAGIC NUMBERS)
     * ============================================================
     * Public because it is needed by the file-scope K_MSGQ_DEFINE()
     * in canfd.cpp, which must be sized at compile time outside the
     * class body. Not part of the protocol contract with other
     * modules — purely an implementation-tuning constant.
     */
    static constexpr std::uint16_t kRxQueueDepth = 10U;

private:
    /* ============================================================
     * CAN IDs (NO MAGIC NUMBERS)
     * ============================================================
     */
    static constexpr std::uint32_t kStartMotorId = 0x400U;
    static constexpr std::uint32_t kStopMotorId  = 0x100U;
    static constexpr std::uint32_t kEstopId      = 0x101U;

    static constexpr std::uint32_t kSensorHbId   = 0x200U;
    static constexpr std::uint32_t kMotorHbId    = 0x201U;
    static constexpr std::uint32_t kPressureId    = 0x300U;
    static constexpr std::uint32_t kFlowId        = 0x301U;

    /* ============================================================
     * RX QUEUE TUNING (NO MAGIC NUMBERS)
     * ============================================================
     */
    static constexpr std::uint32_t kRxPollTimeoutMs = 10U;
    static constexpr std::uint32_t kTxTimeoutMs     = 10U;

    /* Accept-all filter: zero mask matches every standard CAN ID. */
    static constexpr std::uint32_t kAcceptAllId   = 0U;
    static constexpr std::uint32_t kAcceptAllMask = 0U;

    /**
     * @brief Internal RX dispatcher
     * @param id CAN ID
     * @param data pointer to payload
     * @param len payload length
     */
    static void DecodeRx(std::uint32_t id, const std::uint8_t* data, std::uint8_t len);
};

#endif // CANFD_HPP