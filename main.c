/***************************************************************************//**
 *
 *  @file         main.c
 *
 *  @author       Stephen Brennan
 *
 *  @date         Thursday,  8 January 2015
 *
 *  @brief        LSH (Libstephen SHell)
 *
 *  ---------------------------------------------------------------------------
 *  DESIGN CONTRACT:
 *
 *  This program implements a minimal single-process interactive UNIX shell.
 *
 *  SYSTEM INVARIANTS:
 *  - All heap allocations have exactly one owner.
 *  - All owned allocations are freed exactly once.
 *  - Child processes perform no heap management.
 *  - The parent process always reaps children.
 *  - Shell must not crash on invalid user input.
 *
 *  OWNERSHIP MODEL:
 *  - lsh_read_line()   → returns owned heap buffer (caller must free)
 *  - lsh_split_line()  → returns owned token array (caller must free)
 *                        Tokens borrow memory from the line buffer.
 *  - All other functions borrow memory only.
 *
 *  PARSING CONTRACT:
 *  - Tokenization is destructive: strtok() modifies the input buffer.
 *  - Token lifetimes are coupled to the lifetime of the input buffer.
 *
 *******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Built-in shell commands.
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
 * Built-in dispatch tables.
 */
char *builtin_str[] = { "cd", "help", "exit" };

int (*builtin_func[])(char **) = { &lsh_cd, &lsh_help, &lsh_exit };

/*
 * Returns number of built-ins.
 * BORROWS: builtin_str
 */
int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
 * Built-in: cd
 * BORROWS: args
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) perror("lsh");
  }
  return 1;
}

/*
 * Built-in: help
 * BORROWS: args
 */
int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++)
    printf("  %s\n", builtin_str[i]);

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/*
 * Built-in: exit
 */
int lsh_exit(char **args)
{
  return 0;
}

/*
 * Launch external program.
 *
 * CONTRACT:
 *  - Child process must not return.
 *  - Child must not perform heap operations.
 *  - Parent process blocks until child terminates.
 *
 * BORROWS: args
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();

  if (pid == 0) {
    if (execvp(args[0], args) == -1)
      perror("lsh");
    exit(EXIT_FAILURE);   /* Child must terminate */
  }
  else if (pid < 0) {
    perror("lsh");
  }
  else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/*
 * Execute built-in or external program.
 * BORROWS: args
 */
int lsh_execute(char **args)
{
  int i;
  if (args[0] == NULL) return 1;

  for (i = 0; i < lsh_num_builtins(); i++)
    if (strcmp(args[0], builtin_str[i]) == 0)
      return (*builtin_func[i])(args);

  return lsh_launch(args);
}

/*
 * Read line from stdin.
 *
 * RETURNS:
 *  - Owned heap buffer.
 *  - Caller must free().
 */
char *lsh_read_line(void)
{
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE, position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) { fprintf(stderr, "lsh: allocation error\n"); exit(EXIT_FAILURE); }

  while (1) {
    c = getchar();
    if (c == EOF) exit(EXIT_SUCCESS);
    else if (c == '\n') { buffer[position] = '\0'; return buffer; }
    buffer[position++] = c;

    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) { fprintf(stderr, "lsh: allocation error\n"); exit(EXIT_FAILURE); }
    }
  }
}

/*
 * Tokenize input.
 *
 * DESTRUCTIVE PARSER:
 *  - Modifies input buffer.
 *  - Tokens borrow memory from input buffer.
 *
 * RETURNS:
 *  - Owned token array (caller must free).
 */
char **lsh_split_line(char *line)
{
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) { fprintf(stderr, "lsh: allocation error\n"); exit(EXIT_FAILURE); }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position++] = token;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) { fprintf(stderr, "lsh: allocation error\n"); exit(EXIT_FAILURE); }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}

/*
 * Main shell loop.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);
    free(line);
    free(args);
  } while (status);
}

/*
 * Entry point.
 */
int main(int argc, char **argv)
{
  lsh_loop();
  return EXIT_SUCCESS;
}
