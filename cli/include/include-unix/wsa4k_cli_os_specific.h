#ifndef WSA4K_CLI_IS_SPECIFIC_H
#define WSA4K_CLI_IS_SPECIFIC_H

typedef struct timespec TIME_HOLDER;

#define getch() getchar()

// Have not yet test this function
int kbhit(void);

#endif