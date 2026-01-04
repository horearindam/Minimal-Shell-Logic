#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Built-ins */
int blsh_cd(char **);
int blsh_help(char **);
int blsh_exit(char **);

char *builtin_str[] = { "cd", "help", "exit" };
int (*builtin_func[])(char **) = { blsh_cd, blsh_help, blsh_exit };

int blsh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

/* cd */
int blsh_cd(char **args) {
  if (!args[1]) fprintf(stderr, "blsh: cd needs a path\n");
  else if (chdir(args[1]) != 0) perror("blsh");
  return 1;
}

/* help */
int blsh_help(char **args) {
  puts("BarkBuff's LittleShell");
  for (int i = 0; i < blsh_num_builtins(); i++) puts(builtin_str[i]);
  return 1;
}

/* exit */
int blsh_exit(char **args) { return 0; }

/* exec external */
int blsh_launch(char **args) {
  pid_t pid = fork(); int status;

  if (pid == 0) {
    execvp(args[0], args);
    perror("blsh"); exit(EXIT_FAILURE);
  } else if (pid < 0) perror("blsh");
  else do waitpid(pid, &status, WUNTRACED);
       while (!WIFEXITED(status) && !WIFSIGNALED(status));
  return 1;
}

/* dispatch */
int blsh_execute(char **args) {
  if (!args[0]) return 1;
  for (int i = 0; i < blsh_num_builtins(); i++)
    if (!strcmp(args[0], builtin_str[i]))
      return (*builtin_func[i])(args);
  return blsh_launch(args);
}

/* read line – returns owned buffer */
char *blsh_read_line(void) {
  int size = 1024, pos = 0, c;
  char *buf = malloc(size);
  if (!buf) exit(1);

  while (1) {
    c = getchar();
    if (c == EOF) exit(0);
    if (c == '\n') { buf[pos] = 0; return buf; }
    buf[pos++] = c;
    if (pos >= size) buf = realloc(buf, size += 1024);
  }
}

/* destructive split – returns owned token array */
char **blsh_split(char *line) {
  int size = 64, pos = 0;
  char **t = malloc(size * sizeof(char *));
  char *tok = strtok(line, " \t\r\n\a");
  while (tok) {
    t[pos++] = tok;
    if (pos >= size) t = realloc(t, (size += 64) * sizeof(char *));
    tok = strtok(NULL, " \t\r\n\a");
  }
  t[pos] = NULL;
  return t;
}

/* main loop */
void blsh_loop(void) {
  char *line; char **args; int status;
  do {
    printf("> ");
    line = blsh_read_line();
    args = blsh_split(line);
    status = blsh_execute(args);
    free(line); free(args);
  } while (status);
}

int main() {
  blsh_loop();
}
