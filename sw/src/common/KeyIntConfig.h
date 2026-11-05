/**
 * @file KeyIntConfig.h
 * @brief Shared message-queue configuration for the dsky app and the
 *        dskyKeyInt keypad daemon.
 *
 * Both processes must agree on the SysV message queue paths and IDs used
 * to exchange keypad events (OUT) and control commands (IN). This header
 * is a plain, toolchain-independent definition: it is included directly
 * from source files and is NOT generated per build environment.
 *
 * Note: intentionally named KeyIntConfig.h (not config.h) so it does not
 * shadow the app's CMake-generated config.h for sources in this folder.
 */

#ifndef KEYINT_CONFIG_H
#define KEYINT_CONFIG_H

// Message queue for OUT messages (keypad events, daemon -> app)
#define KEY_INT_MSG_QUEUE_FILE "/var/local/dskykeyint"
#define KEY_INT_MSG_QUEUE_ID 10

// Message queue for IN commands (control commands, app -> daemon)
#define CMD_MSG_QUEUE_FILE "/var/local/dskycmd"
#define CMD_MSG_QUEUE_ID 12

#endif // KEYINT_CONFIG_H
