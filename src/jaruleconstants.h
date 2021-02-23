/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * constants.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file constants.h
 * @brief Various constants.
 *
 * This file defines constants that are not expected to change. Constants that
 * are board specific should be placed in the system_config/ directory.
 */

#ifndef FIRMWARE_SRC_CONSTANTS_H_
#define FIRMWARE_SRC_CONSTANTS_H_

// *****************************************************************************
// USB specific constants
// *****************************************************************************

/**
 * @brief The maximum size of a USB packet to / from the bulk endpoint.
 *
 * 64 bytes is the highest value a full speed, bulk endpoint can use.
 */
#define USB_MAX_PACKET_SIZE 64u

/**
 * @brief The maximum transfer size of a Ja Rule USB command.
 *
 * This should be a multiple of USB_MAX_PACKET_SIZE.
 */
#define USB_READ_BUFFER_SIZE 576u

/**
 * @brief The polling interval for the bulk endpoint in milliseconds.
 *
 * 1ms is the shortest polling interval USB allows.
 */
#define USB_POLLING_INTERVAL 1u

// *****************************************************************************
// Network specific constants
// *****************************************************************************

/**
 * @brief The size of a MAC address
 */
enum { MAC_ADDRESS_SIZE = 6 };

// *****************************************************************************
// RDM specific constants
// *****************************************************************************

/**
 * @brief The RDM Start Code.
 */
#define RDM_START_CODE 0xccu

/**
 * @brief The RDM Sub-start Code.
 */
#define RDM_SUB_START_CODE 0x01u

// *****************************************************************************
// Protocol specific constants
// *****************************************************************************

/**
 * @brief The Ja Rule message commands.
 */
typedef enum {
  // Base commands
  /**
   * @brief Reset the device.
   * See @ref message-commands-reset.
   */
  COMMAND_RESET_DEVICE = 0x00,

  /**
   * @brief Change the operating mode of the device.
   * @sa @ref message-commands-setmode.
   */
  COMMAND_SET_MODE = 0x01,

  /**
   * @brief Get the hardware info for the device.
   * @sa @ref message-commands-gethardware.
   */
  COMMAND_GET_HARDWARE_INFO = 0x02,

  /**
   * @brief Run a loopback self test.
   * @sa @ref message-commands-selftest.
   */
  COMMAND_RUN_SELF_TEST = 0x03,

  // User Configuration
  /**
   * @brief Set the break time of the transceiver.
   * See @ref message-commands-setbreaktime
   */
  COMMAND_SET_BREAK_TIME = 0x10,

  /**
   * @brief Fetch the current transceiver break time.
   * See @ref message-commands-getbreaktime
   */
  COMMAND_GET_BREAK_TIME = 0x11,

  /**
   * @brief Set the mark-after-break time of the transceiver.
   * See @ref message-commands-setmarktime
   */
  COMMAND_SET_MARK_TIME = 0x12,

  /**
   * @brief Fetch the current transceiver mark-after-break time.
   * See @ref message-commands-getmarktime
   */
  COMMAND_GET_MARK_TIME = 0x13,

  // Advanced Configuration
  /**
   * @brief Set the RDM Broadcast timeout.
   * See @ref message-commands-setbcasttimeout.
   */
  COMMAND_SET_RDM_BROADCAST_TIMEOUT = 0x20,

  /**
   * @brief Get the RDM Broadcast timeout.
   * See @ref message-commands-getbcasttimeout.
   */
  COMMAND_GET_RDM_BROADCAST_TIMEOUT = 0x21,

  /**
   * @brief Set the RDM Response timeout.
   * See @ref message-commands-setresponsetimeout.
   */
  COMMAND_SET_RDM_RESPONSE_TIMEOUT = 0x22,

  /**
   * @brief Get the RDM Response timeout.
   * See @ref message-commands-getresponsetimeout.
   */
  COMMAND_GET_RDM_RESPONSE_TIMEOUT = 0x23,

  /**
   * @brief Set the RDM DUB Response limit.
   * See @ref message-commands-setdublimit.
   */
  COMMAND_SET_RDM_DUB_RESPONSE_LIMIT = 0x24,

  /**
   * @brief Get the RDM Response limit.
   * See @ref message-commands-getdublimit.
   */
  COMMAND_GET_RDM_DUB_RESPONSE_LIMIT = 0x25,

  /**
   * @brief Set the RDM responder delay.
   * See @ref message-commands-setresponderdelay.
   */
  COMMAND_SET_RDM_RESPONDER_DELAY = 0x26,

  /**
   * @brief Get the RDM responder delay.
   * See @ref message-commands-getresponderdelay.
   */
  COMMAND_GET_RDM_RESPONDER_DELAY = 0x27,

  /**
   * @brief Set the RDM responder jitter.
   * See @ref message-commands-setresponderjitter.
   */
  COMMAND_SET_RDM_RESPONDER_JITTER = 0x28,

  /**
   * @brief Get the RDM responder jitter.
   * See @ref message-commands-getresponderjitter.
   */
  COMMAND_GET_RDM_RESPONDER_JITTER = 0x29,

  // DMX
  TX_DMX = 0x30,  //!< Transmit a DMX frame. See @ref message-commands-txdmx.

  // RDM
  /**
   * @brief Send an RDM Discovery Unique Branch and wait for a response.
   * See @ref message-commands-txrdmdub.
   */
  COMMAND_RDM_DUB_REQUEST = 0x40,

  /**
   * @brief Send an RDM Get / Set command.
   * See @ref message-commands-txrdm
   */
  COMMAND_RDM_REQUEST = 0x41,

  /**
   * @brief Send a broadcast RDM command.
   * See @ref message-commands-txrdmbroadcast.
   */
  COMMAND_RDM_BROADCAST_REQUEST = 0x42,

  // Experimental / testing
  COMMAND_ECHO = 0xf0,  //!< Echo the data back. See @ref message-commands-echo
  GET_FLAGS = 0xf2,  //!< Get the flags state
} Command;

/**
 * @brief JaRule command return codes.
 */
typedef enum {
  RC_OK = 0,  //!< The command completed successfully.
  RC_UNKNOWN = 1,  //!< Unknown command
  /**
   * @brief The command could not be completed due to a full memory buffer
   */
  RC_BUFFER_FULL = 2,
  RC_BAD_PARAM = 3,  //!< The command was malformed.
  RC_TX_ERROR = 4,  //!< There was an error during transceiver transmit.
  RC_RDM_TIMEOUT = 5,  //!< No RDM response was received.

  /**
   * @brief Data was received in response to a broadcast RDM command.
   *
   * This usually indicates a broken responder.
   */
  RC_RDM_BCAST_RESPONSE = 6,
  RC_RDM_INVALID_RESPONSE = 7,  //!< An invalid RDM response was received.
  RC_INVALID_MODE = 8,  //!< The command is invalid in the current mode.

  RC_TEST_FAILED = 9,  //!< The self test failed
  RC_CANCELLED = 10  //!< The request was preempted or cancelled
} ReturnCode;

/**
 * @brief The Start of Message identifier.
 */
#define START_OF_MESSAGE_ID 0x5au

/**
 * @brief The End of Message Identifier.
 */
#define END_OF_MESSAGE_ID 0xa5u

/**
 * @brief The maximum payload size in a message.
 */
#define PAYLOAD_SIZE 513u

/**
 * @brief The minimum size of a valid request
 */
#define MINIMUM_REQUEST_SIZE 7u

/**
 * @brief The minimum size of a valid response
 */
#define MINIMUM_RESPONSE_SIZE 9u

/**
 * @brief The break time in microseconds.
 */
#define DEFAULT_BREAK_TIME 176u

/**
 * @brief The mark time in microseconds.
 */
#define DEFAULT_MARK_TIME 12u

/**
 * @brief The time to listen for a response after sending an RDM broadcast.
 *
 * Measured in 10ths of a millisecond. This can be 0, since we don't expect
 * responses from broadcast messages, however by waiting we can detect bad
 * responders, so we set this the same as DEFAULT_RDM_RESPONSE_TIMEOUT.
 */
#define DEFAULT_RDM_BROADCAST_TIMEOUT 28u

/**
 * @brief The default RDM response timeout for a controller.
 *
 * Measured is in 10ths of a millisecond, from Line 1 & 3, Table 3-2, E1.20.
 * Responders have 2ms (see Table 3-4), and then in line proxies can introduce
 * up to 704uS of delay. This rounds to 2.8 ms.
 */
#define DEFAULT_RDM_RESPONSE_TIMEOUT 28u

/**
 * @brief The default maximum time an RDM DUB response can take.
 *
 * Measured in in 10ths of a microsecond. From Line 3, Table 3-3, E1.20.
 */
#define DEFAULT_RDM_DUB_RESPONSE_LIMIT 29000u

/**
 * @brief The default time to wait before sending an RDM response.
 * @sa Transceiver_SetRDMResponderDelay.
 *
 * Measured in in 10ths of a microsecond. From Table 3-4, E1.20.
 */
#define DEFAULT_RDM_RESPONDER_DELAY 1760u

#endif  // FIRMWARE_SRC_CONSTANTS_H_

/**
 * @}
 */
