#include "utils/commands.h"
#include <stdio.h>
#include <string.h>

typedef struct irongit_command
{
  char *name;
  int (*handler)(int argc, char *argv[]);
}irongit_command;

irongit_command commands[] = {
  {"init", cmd_init},
  {"hash-object", cmd_hash_object},
  {"cat-file", cmd_cat_file},
  {"add", cmd_add},
  {"ls-tree", cmd_ls_tree},
  {"write-tree", cmd_write_tree}
};

#define DIM_COMMANDS sizeof(commands) / sizeof(irongit_command)

int main(int argc, char **argv)
{

  if (argc < 2) // If the user doesn't pass any command, print the help/error message
  {
    printf("Usage: irongit <command> [<args>]\n\nAvailable commands:\n");
    return 1;
  }
  const char *requested_command = argv[1];
  // Search for the command
  for (int i = 0; i < DIM_COMMANDS; i++)
  {
    if (strcmp(requested_command, commands[i].name) == 0)
    {
      return commands[i].handler(argc, argv); // if it finds the command executes it
    }
  }
  printf("irongit: '%s' is not an irongit command. See 'irongit --help'.\n", requested_command);
  return 1;
}

