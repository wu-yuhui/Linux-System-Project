#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;

    mode = validargs(argc, argv);

    printf("The mode is 0x%04X \n", mode);

    debug("Mode: 0x%X", mode);

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    else if (mode & 0x4000) {
        // Fractionated Morse Cipher
        printf("Fractionated Morse Cipher\n");
    }
    else if (mode & 0x0088) {
        // Polybius Cipher
        printf("Polybius Cipher\n");
    }
    else if (mode == 0){
        // Usage error
        printf("It's an Error\n");
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */