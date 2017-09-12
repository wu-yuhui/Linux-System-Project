#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
void formPolybiusTable(unsigned short mode){
    if (key == NULL){
        printf("Normal Polybius\n");
        // Normal Polybius
        int row = mode & 0x00F0;
        int column = mode & 0x000F;

        // Insert polybius_alphabet into polybiuds table
        int counterRow = 0;
        int counterColumn = 0;
        const char* tmp_polybius_alphabet = polybius_alphabet;
        while (*tmp_polybius_alphabet != 0){
            *(polybius_table+ counterRow*column + counterColumn) = *tmp_polybius_alphabet;
            tmp_polybius_alphabet++;

            printf("Row: %d, Column %d \n", counterRow, counterColumn);

            counterColumn++;
            if (counterColumn == column){
                counterRow++;
                counterColumn = 0;
            }
        }
    }

    else {
        printf("Polybius with KEY!! \n");
        // Polybius with Key

        int row = mode & 0x00F0;
        int column = mode & 0x000F;

        // Insert polybius_alphabet into polybiuds table
        int counterRow = 0;
        int counterColumn = 0;
        const char* tmp_polybius_alphabet = polybius_alphabet;

        // Special Cases
        int keyLength = LenghtofString(key);
        counterColumn += keyLength % column;
        counterRow += keyLength / column;

        while (*tmp_polybius_alphabet != 0){
            if (/* belongs in KEY -> return number # */ CharInString(key, tmp_polybius_alphabet) ){
                int keyPosition = CharInString(key, tmp_polybius_alphabet) - 1;
                *(polybius_table+keyPosition) = *tmp_polybius_alphabet;
                tmp_polybius_alphabet++;
            }
            else {
                *(polybius_table+ counterRow*column + counterColumn) = *tmp_polybius_alphabet;
                tmp_polybius_alphabet++;

                counterColumn++;
                if (counterColumn == column){
                    counterRow++;
                    counterColumn = 0;
                }
            }

        }
    }

//    printf("\n\n polybius_table: %s \n", polybius_table);

    return;
}


int CharInString(const char* toSearch, const char* beSearched){
    int counterKey = 0;
    while (*(toSearch+counterKey) != 0){
        // If found, return position+1
        if (*(toSearch+counterKey) == *beSearched) return (counterKey+1);
        else counterKey++;
    }
    // Not found return 0;
    return 0;

}


int LenghtofString(const char* string){
    int length = 0;
    while (*(string+length) != 0) length++;
    return length;
}

int checkRepeatAndSubset(const char* theKey, const char* alphabet) {
        int keyLength = LenghtofString(theKey);
        int alphabetLength = LenghtofString(alphabet);

    // 1. Check if theKey has repeating characters
        for (int counter1 = 0; counter1 < keyLength; counter1++)
            for (int counter2 = counter1 + 1; counter2 < keyLength; counter2++)
                if (*(theKey+counter1) == *(theKey+counter2)) return 0;

    // 2. Check if characters of theKey are all in alphabet
        for (int counterKey = 0; counterKey < keyLength; counterKey++){
            int counterAlphabet = 0;
            while(*(theKey+counterKey) != *(alphabet+counterAlphabet)){
                counterAlphabet++;
                if (counterAlphabet == alphabetLength) return 0;
            }
        }
    // All pass -> valid
        return 1;
}

unsigned short validargs(int argc, char **argv) {
/*
    printf("argv = %p\n", argv);                            // Pointer to the first word of input.
    printf("argv+1 = %p\n", argv+1);                        // Pointer to the second word of input. seperated by white space,
    printf("argv+2 = %p\n", argv+2);                        // Pointer to the third word of input. seperated by white space,
    printf("*(argv+1) = %s\n", *(argv+1));                  // Pointer to the first letter of the second word
    printf("*(*(argv+1)) = %d\n", *(*(argv+1)));            // ASCII value of the first letter of the second word
    printf("*(*(argv+1)+1) = %d\n", *(*(argv+1)+1));        // ASCII value of the second letter of the second word
    printf("*(*(argv+1)+2) = %d\n", *(*(argv+1)+2));
    printf("*(*(argv+1)+3) = %d\n", *(*(argv+1)+3));
    printf("*(*(argv+1)+4) = %d\n", *(*(argv+1)+4));        // If exceed boundaries,this will obtain the next string component of the input
    printf("*(*(argv+1)+5) = %d\n", *(*(argv+1)+5));
    printf("argc = %d\n", argc);
*/

    int validNum = 0x0000;

    if (argc <= 1){                     // if no flags
//        USAGE(*argv, EXIT_FAILURE);
        printf("NO FLAG\n");
        return 0x0000;
    }
    else{
        if (*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'h' && *(*(argv+1)+2) == 0){        // if argv[1] == -h

            printf ("-h\n");
            return 0x8000;
        }
        else if (*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'p' && *(*(argv+1)+2) == 0){        // if argv[1] == -p
            printf ("-p  ");
            validNum += 0x0000;

            // Encrypt or Decrypt
            if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'e' && *(*(argv+2)+2) == 0)            // if argv[2] == -e
                validNum += 0x0000;
            else if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'd' && *(*(argv+2)+2) == 0)       // if argv[2] == -d
                validNum += 0x2000;
            else return 0x0000;

            // Row and Column initials are ten
            validNum += 0x00aa;
            int rowLength = 10;
            int columnLength = 10;

            // Optional Arguments
            if (argc > 3){
                int previousArguments = 3;
                int counter = 0;
                while (counter < argc-previousArguments){
                    // Recusively parse all left arguments
                    if (*(*(argv+previousArguments+counter)) == '-'){
                        if (*(*(argv+previousArguments+counter)+1) == 'k' && *(*(argv+previousArguments+counter)+2) == 0){
                            printf("-k ");
                            counter++;
                            /*  Case -k:Parse KEY and Check repeat or subset

                            */
                            key = *(argv+previousArguments+counter);
                            // Check if Key is eligible
                            if(checkRepeatAndSubset(key, polybius_alphabet) == 0) return 0x0000;

                            counter++;
                        }
                        else if (*(*(argv+previousArguments+counter)+1) == 'r' && *(*(argv+previousArguments+counter)+2) == 0){
                            printf("-r ");
                            counter++;
                            /*   Case -c:Check Number Valid  */
                            // Check Validation of Number
                            // 9
                            if (*(*(argv+previousArguments+counter)+1) == 0 && *(*(argv+previousArguments+counter)) == '9'){
                                validNum += 0x0090;
                                rowLength = 9;
                            }
                            // 10-15
                            else if (*(*(argv+previousArguments+counter)+2) == 0 && *(*(argv+previousArguments+counter)) == '1' \
                                     && *(*(argv+previousArguments+counter)+1) >= 48 && *(*(argv+previousArguments+counter)+1) <= 53) {
                                rowLength = *(*(argv+previousArguments+counter)+1) - 38;
                                // row -> bit 5-8
                                int prepValidNum = rowLength * 16;
                                validNum += prepValidNum;
                            }
                            else return 0x0000;

                            // Subtract default & Plus counter
                            validNum -= 0x00a0;
                            counter++;

                        }
                        else if (*(*(argv+previousArguments+counter)+1) == 'c' && *(*(argv+previousArguments+counter)+2) == 0){
                            printf("-c ");
                            counter++;
                            /*   Case -c:Check Number Valid  */
                            // Check Validation of Number
                            //9
                            if (*(*(argv+previousArguments+counter)+1) == 0 && *(*(argv+previousArguments+counter)) == '9'){
                                validNum += 0x0009;
                                columnLength = 9;
                            }
                            // 10-15
                            else if (*(*(argv+previousArguments+counter)+2) == 0 && *(*(argv+previousArguments+counter)) == '1' \
                                     && *(*(argv+previousArguments+counter)+1) >= 48 && *(*(argv+previousArguments+counter)+1) <= 53) {
                                columnLength = *(*(argv+previousArguments+counter)+1) - 38;
                                // column -> bit 1-4
                                int prepValidNum = columnLength;
                                validNum += prepValidNum;
                            }
                            else return 0x0000;

                            // Subtract default & Plus counter
                            validNum -= 0x000a;
                            counter++;
                        }
                        else return 0x0000;

                    }
                    else return 0x0000;
                }
            }

            // Invalid for (rows * columns) < length of polybius_alphabet
            int alphabetLength = 0;
            while (*(polybius_alphabet+alphabetLength) != 0) alphabetLength++;
            if (rowLength * columnLength < alphabetLength)  return 0x0000;

            // If all valid, return value of "mode"
            return validNum;
        }
        else if (*(*(argv+1)) == '-' && *(*(argv+1)+1) == 'f' && *(*(argv+1)+2) == 0){        // if argv[1] == -f
            printf ("-f  ");
            validNum += 0x4000;

            // Encrypt or Decrypt
            if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'e' && *(*(argv+2)+2) == 0)            // if argv[2] == -e
                validNum += 0x0000;
            else if (*(*(argv+2)) == '-' && *(*(argv+2)+1) == 'd' && *(*(argv+2)+2) == 0)       // if argv[2] == -d
                validNum += 0x2000;
            else return 0x0000;

            // Optional Arguments
            if (argc > 3){
                int previousArguments = 3;
                int counter = 0;
                while (counter < argc-previousArguments){
                    // Recusively parse all left arguments
                    if (*(*(argv+previousArguments+counter)) == '-'){
                        if (*(*(argv+previousArguments+counter)+1) == 'k' && *(*(argv+previousArguments+counter)+2) == 0){
                            printf("-k ");
                            counter++;
                            /*  Case -k:Parse KEY and Check repeat or subset

                            */
                            key = *(argv+previousArguments+counter);
                            // Check if Key is eligible
                            if(checkRepeatAndSubset(key, fm_alphabet) == 0) return 0x0000;

                            counter++;
                        }
                        else return 0x0000;
                    }
                    else return 0x0000;
                }
            }

            return validNum;
        }
        else{                               // not correct flag
//            USAGE(*argv, EXIT_FAILURE);
            printf ("USELESS\n");
            return 0x0000;
        }
    }
    return 0x8000;
}
