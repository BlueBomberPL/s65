/*
 *  main.c
 *
 *  Entrance point for the program.
 *  Launches modules.
 * 
 */

 #include "reader.h"

 int main(int argc, char **argv)
 {
    s65_read_file("../res/file.hex", S65_READER_AUTO);

    printf("ok");
 }
