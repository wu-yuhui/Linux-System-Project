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


void InitialMorseStorage(){
    // Initial Polybius table to store 1-step Morse for FM use.
    for (int counterClear = 0; counterClear < 257; counterClear++){
        *(polybius_table+counterClear) = 0;
//        printf("Initial Poly for Morse:%d\n", counterClear);
    }
}

void PrintMorseStorage(){
//    printf("Morse Storage: ");
    for (int counterClear = 0; counterClear < 257; counterClear++){
//        printf("PrintMorseStorage %d\n", counterClear);
        if (*(polybius_table+counterClear) != 0)
            printf("%c", *(polybius_table+counterClear));
    }
    printf("\n");
}


int MorseEncrypt(char c, int* whiteSpace){
    // 1. If whiteSpace = 1,skip
    // 2. If not, add "xx", whiteSpace++
//    printf("In Morse Encrypt \n");
    int insertPosition = LenghtofString(polybius_table);

//    printf("insertPosition: %d\n", insertPosition);
//    PrintMorseStorage();
//    printf("SWGEGEGGEGE\n");

    if (c == ' ' ){
        if (*whiteSpace == 1)   return 1;
        else {
            *(polybius_table+insertPosition) = 'x';
            *whiteSpace = 1;
        }
    }
    else if (c == '\n') {
        *whiteSpace = 0;
        *(polybius_table+insertPosition) = 'x';
//        PrintMorseStorage();
        FM_Encrypt();
        InitialMorseStorage();
        return 1;
    }
    else {
        // Find if in the Morse Table and not NULL
        char* charAddr = &c;
        int morsePosition = CharInString(polybius_alphabet, charAddr) - 1;

        // If NULL return 0 for FAILURE
        if (**(morse_table+morsePosition) == 0) return 0;

        int morseCodeLength = 0;
        while (*(*(morse_table+morsePosition)+morseCodeLength) != 0) {
            *(polybius_table+insertPosition+morseCodeLength) = *(*(morse_table+morsePosition)+morseCodeLength);
            morseCodeLength++;
        }
        *(polybius_table+insertPosition+morseCodeLength) = 'x';
        *whiteSpace = 0;
        return 1;
    }
    return 1;

}

void FM_Encrypt(){

//    printf("FM_Encrypt\n");

    for (int counterFMEnc = 0; counterFMEnc < LenghtofString(polybius_table)-2; counterFMEnc+=3){
        int keyPosition = TotalValueofFMKey((polybius_table+counterFMEnc));
        printf( "%c", *(fm_key+keyPosition));
    }
    printf("\n");
}


int FindMorseOriginal(int historyLength, int nextXLength){

//    printf("FindMorseOriginal\n");
//    printf("historyLength: %d, nextXLength: %d\n", historyLength, nextXLength);

    int fmSectionLength = nextXLength-1;

    int counterNo = 0;
    while (*(morse_table+counterNo) != 0){
        // if length equals
        if (LenghtofString(*(morse_table+counterNo)) == fmSectionLength){
            int counterSection = 0;
            // String char-by-char comparison
            while ( *(*(morse_table+counterNo)+counterSection) == *(polybius_table+historyLength+1+counterSection) ){
//                printf("%c", *(polybius_table+historyLength+1+counterSection));
                counterSection++;
                if(counterSection == fmSectionLength) return counterNo;
            }
        }
        counterNo++;
    }
    return 0;

}


void MorseDeCrypt(){
    // Find code in morse_table and  polybius alphabet
//    printf("MorseDeCrypt\n");
    //Find next x;
    int historyLength = -1;
    int nextXLength = 1;  // nextXLength far, but word length is (nextXLength-1)

    int polybius_table_Length = LenghtofString(polybius_table);
    if ( *(polybius_table+polybius_table_Length-1) != 'x'){

        while (historyLength+nextXLength < polybius_table_Length){

            // Not x terminated
            while (*(polybius_table+historyLength+nextXLength) != 'x'){
                // if last number
                if (*(polybius_table+historyLength+nextXLength) == 0) break;
                nextXLength++;
            }
            // The No.1 Word case
            // if (historyLength == 0) historyLength = -1;
            if (nextXLength == 1)   printf("%c", ' ');
            else {
                int morsePosition = FindMorseOriginal(historyLength, nextXLength);
//                printf("\nMorse Position= %d\n", morsePosition);
                printf("%c", *(polybius_alphabet+morsePosition));
            }
            historyLength += nextXLength;
            nextXLength = 1;
        }

    }
    else {
        while (historyLength+nextXLength < polybius_table_Length){
            // x terminated
            while (*(polybius_table+historyLength+nextXLength) != 'x'){
                nextXLength++;
            }
            // The No.1 Word case
            // if (historyLength == 0) historyLength = -1;
            if (nextXLength == 1)   printf("%c", ' ');
            else {
                int morsePosition = FindMorseOriginal(historyLength, nextXLength);
//                printf("\nMorse Position= %d\n", morsePosition);
                printf("%c", *(polybius_alphabet+morsePosition));
            }

            historyLength += nextXLength;
            nextXLength = 1;
        }
    }

    // Use section length to find letter
}



void FM_Decrypt(char c){
    // Find Insertion Point
    int insertPosition = LenghtofString(polybius_table);

    if (c == '\n'){
//        PrintMorseStorage();
        MorseDeCrypt();
        InitialMorseStorage();
        printf("\n");

    }
    else {

        // Which key
        char* charAddr = &c;
        int position = CharInString(fm_key, charAddr) - 1;

        *(polybius_table+insertPosition) = PositionofFMKey((position/9)%3);
        *(polybius_table+insertPosition+1) = PositionofFMKey((position/3)%3);
        *(polybius_table+insertPosition+2) = PositionofFMKey(position%3);

    }

}


int TotalValueofFMKey(const char* tmp_key){
    return ( ValueofFMKey(*tmp_key)*9 + ValueofFMKey(*(tmp_key+1))*3 + ValueofFMKey(*(tmp_key+2)) );
}

int ValueofFMKey(char c){
    if (c == '.') return 0;
    else if (c == '-') return 1;
    else if (c == 'x') return 2;
    else return 0;
}

char PositionofFMKey(int i){
    if (i == 0) return '.';
    else if (i == 1) return '-';
    else if (i == 2) return 'x';
    else return 0;
}


void formFMTable(){
    if (key == NULL){
        int lenghtofFMAlphabet = LenghtofString(fm_alphabet);
        for (int counterFMTable = 0; counterFMTable < lenghtofFMAlphabet; counterFMTable++)
            *(fm_key+counterFMTable) = *(fm_alphabet+counterFMTable);
    }
    else{
        // Have Key
        const char* tmp_fm_alphabet = fm_alphabet;
        int keyLength = LenghtofString(key);
        int counterFMTable = 0;

        while (*tmp_fm_alphabet != 0){

            if (/* belongs in KEY -> return number # */ CharInString(key, tmp_fm_alphabet) ){
                int keyPosition = CharInString(key, tmp_fm_alphabet) - 1;
                *(fm_key+keyPosition) = *tmp_fm_alphabet;
                tmp_fm_alphabet++;
            }
            else {
                *(fm_key+ keyLength + counterFMTable) = *tmp_fm_alphabet;
                tmp_fm_alphabet++;

                counterFMTable++;
            }
        }
/*
    printf("FM KEEEEEEEEY: ");
    for (int counterClear = 0; counterClear < 27; counterClear++){
//        printf("PrintMorseStorage %d\n", counterClear);
        if (*(fm_key+counterClear) != 0)
            printf("%c", *(fm_key+counterClear));
        }
        printf("\n");
*/
    }
}




void formPolybiusTable(unsigned short mode){
    if (key == NULL){
        //printf("Normal Polybius\n");
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

            //printf("Row: %d, Column %d \n", counterRow, counterColumn);

            counterColumn++;
            if (counterColumn == column){
                counterRow++;
                counterColumn = 0;
            }
        }
    }

    else {
        //printf("Polybius with KEY!! \n");
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
    //printf(" polybius_table: %s \n", polybius_table);
    return;
}


int PolybiusEncrypt(char c){
    char* charAddr = &c;
    int position = CharInString(polybius_table, charAddr) - 1;
//    printf("Postion: %d ", position);
    return position;
}

char PolybiusDecrypt(int position){
//    printf("In decrypt\n");
    char c = *(polybius_table+position);
    //printf("c' %c\n", c);
    return c;
}

int hexChartoInt(char c){
    if (c >= 'a' && c <= 'f')   return (10+c-'a');
    else if (c >= 'A' && c <= 'F')  return (10+c-'A');
    else    return (c-'0');

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
//            printf ("-p  ");
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
//                            printf("-k ");
                            counter++;
                            /*  Case -k:Parse KEY and Check repeat or subset

                            */
                            key = *(argv+previousArguments+counter);
                            // Check if Key is eligible
                            if(checkRepeatAndSubset(key, polybius_alphabet) == 0) return 0x0000;

                            counter++;
                        }
                        else if (*(*(argv+previousArguments+counter)+1) == 'r' && *(*(argv+previousArguments+counter)+2) == 0){
//                            printf("-r ");
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
//                            printf("-c ");
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
//            printf ("-f  ");
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
//                            printf("-k ");
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
