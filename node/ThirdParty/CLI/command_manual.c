/*
 * command_manual.c
 *
 *  Created on: May 31, 2025
 *      Author: HLC
 */
#include <stddef.h>
#include <stdio.h>
#include "command_manual.h"

void command_manual_handler(char **argv, uint8_t argc)
{
    const cli_command_entry_t *entry = command_entry_talbe;
    printf("=== Manual: Command List ===\n");
    while (entry->command_info != NULL)
    {
        printf("-> %s: %s\n", entry->name, entry->command_info->help);
        entry++;
    }
}



