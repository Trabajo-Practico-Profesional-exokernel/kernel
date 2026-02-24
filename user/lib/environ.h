#ifndef PUBLIC_ENVIRON_H
#define PUBLIC_ENVIRON_H

int load_env(void);
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

char ** get_environ_list(void);

#endif