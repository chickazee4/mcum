#include <stdio.h>
#include <stdlib.h>

// todo, obviously

void
get_key(char **cp, char *prompt)
{
    *cp = malloc(1024);
    printf("Please enter your %s: ", prompt);
    scanf("%s", *cp);
}
