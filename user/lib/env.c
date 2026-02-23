#include "environ.h"
#include "lib.h"
#include "string.h"
// #include "parsers/strutil.h"

static char **init_envp = NULL;

static char **environ = NULL;
static size_t env_size = 0;
static size_t env_capacity = 0;

static bool env_loaded = 0;

char ** get_environ_list(void){
    return environ;
}

void init_environ(char **envp)
{
    init_envp = envp;
    environ = envp;      // directly reference

    size_t count = 0;
    while (init_envp[count])
        count++;

    env_size = count;
    env_capacity = count+ 8;

}


int load_env(void)
{
    if (env_loaded)
        return 0;
    if (!init_envp){
    	printf("Error: Failed load env since not setted envp\n");
        return -1;   // or handle error    	
    }


    char **new_env = malloc(sizeof(char *) * (env_capacity + 1));
    if (!new_env)
        return -1;

    for (size_t i = 0; i < env_size; i++)
        new_env[i] = strdup(init_envp[i]);

    new_env[env_size] = NULL;

    environ = new_env;
    env_loaded = 1;
    return 0;
}


static int find_env_ind(const char *name)
{
    size_t len = strlen(name);

    for (size_t i = 0; environ[i]; i++)
    {
        if (strncmp(environ[i], name, len) == 0 &&
            environ[i][len] == '=')
            return i;
    }

    return -1;
}

char *getenv(const char *name)
{
    int index = find_env_ind(name);
    if (index < 0)
        return NULL;

    size_t len = strlen(name);
    return environ[index] + len + 1;
}


static int grow_environ(void)
{
    size_t new_capacity = env_capacity * 2;

    char **new_env = realloc(environ,
                             sizeof(char *) * (new_capacity + 1));
    if (!new_env)
        return -1;

    environ = new_env;
    env_capacity = new_capacity;
    return 0;
}


int setenv(const char *name, const char *value, int overwrite)
{
    if (!env_loaded){
        if (load_env() < 0){
        	return -1;
        }

    }

    int index = find_env_ind(name);

    if (index >= 0 && !overwrite)
        return 0;

    size_t len = strlen(name) + strlen(value) + 2;
    char *entry = malloc(len);
    if (!entry)
        return -1;

    snprintf(entry, len, "%s=%s", name, value);
    
    if (index >= 0)
    {
        free(environ[index]);
        environ[index] = entry;
        return 0;
    }

    if (env_size >= env_capacity)
    {
        if (grow_environ() < 0)
        {
            free(entry);
            return -1;
        }
    }

    environ[env_size++] = entry;
    environ[env_size] = NULL;

    return 0;
}

int unsetenv(const char *name)
{
    if (!env_loaded)
        return -1;

    int index = find_env_ind(name);
    if (index < 0)
        return 0;

    free(environ[index]);

    for (size_t i = index; i < env_size - 1; i++)
        environ[i] = environ[i + 1];

    env_size--;
    environ[env_size] = NULL;

    return 0;
}