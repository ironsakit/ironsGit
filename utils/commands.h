#ifndef COMMANDS_H
#define COMMANDS_H

int cmd_hash_object(int argc, char *argv[]);
int cmd_init(int argc, char *argv[]);
int cmd_cat_file(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_write_tree(int argc, char *argv[]);
int cmd_ls_tree(int argc, char *argv[]);

#endif
