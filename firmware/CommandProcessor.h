//
// Command processor
//
// Interprets and executes commands from the console
//

#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

extern void CommandProcessor_processCommand (
    const char* command);

#endif  // COMMANDPROCESSOR_H