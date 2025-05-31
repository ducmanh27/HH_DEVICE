/*
 * cli_command.h
 *
 *  Created on: May 30, 2025
 *      Author: HLC
 */

#ifndef CLI_CLI_COMMAND_H_
#define CLI_CLI_COMMAND_H_

#include <stdint.h>
#include <string.h>
#include "cli_types.h"
void cli_command_excute(char *uart_buff, uint8_t lenght);
#endif /* CLI_CLI_COMMAND_H_ */
