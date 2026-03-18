#include "utils/commands.h"
#include <stdio.h>
#include <string.h>

typedef struct irongit_command{
  char *name;
  int (*handler)(int argc, char *argv[], char *flag);
}irongit_command;

irongit_command commands[] = {
  {"init", cmd_init},
  {"hash-object", cmd_hash_object},
  // {"cat-file", cmd_cat_file},  TODO
};

int main(int argc, char **argv) {
  // If the user doesn't pass any command, print the help/error message
  if (argc < 2) {
    printf("Usage: irongit <command> [<args>]\n\nAvailable commands:\n");
    return 1;
  }

  const char *requested_command = argv[1];

  // Search for the command
  for (int i = 0; i < 2; i++) {
    if (strcmp(requested_command, commands[i].name) == 0) {
        if(argc > 3){
            return commands[i].handler(argc, argv, argv[2]);  // ex: irongit hash_object -w file.txt
        }
        return commands[i].handler(argc, argv, NULL); // if it finds the command executes it
    }
  }
  printf("irongit: '%s' is not an irongit command. See 'irongit --help'.\n", requested_command);
  return 1;
}
