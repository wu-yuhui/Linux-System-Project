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
        printf("Fractionated Morse Cipher\n");
        // Fractionated Morse Cipher
    }
    else if (mode & 0x0088) {
        printf("Polybius Cipher\n");
        // Polybius Cipher
        formPolybiusTable(mode);

        printf("Enter Characters:");

        if (mode & 0x2000){
            // Decrypt
            char cin;
            while ((cin = getchar()) != EOF){
                if (cin == ' ') printf(" ");
                else if (cin == '\n') printf("\n");
                else{
                    int rowNum = cin - '0';
                    int columnNum = getchar() - '0';
                    int position = rowNum * (mode & 0x000F) + columnNum;
                    //printf("%d %d %X \n", rowNum, columnNum, position);
                    char cout = PolybiusDecrypt(position);
                    printf("%c",cout);
                }
            }
        }
        else {
            // Encrypt
            char cin;
            while ((cin = getchar()) != EOF){
                if (cin == ' ') printf(" ");
                else if (cin == '\n') printf("\n");
                else{
                    int position = PolybiusEncrypt(cin);
                    printf("%X%X", position / (mode & 0x000F), position % (mode & 0x000F));
                }
            }
        }

    }
    else if (mode == 0){
        printf("It's an Error\n");
        // Usage error
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */