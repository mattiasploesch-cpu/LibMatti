//
// Created by administrator on 19.09.26.
//

#include <stdio.h>

#include "libmatti/java/nio/file/Files.h"

int main(int argc, char* argv[])
{
    LIBMATTI_JNF_Files_CreateDirectories("./external_tests/io/out/test");
    LIBMATTI_JNF_Files_CreateFile("./external_tests/io/out/test/test.txt");
    fputs("Geht", fopen("./external_tests/io/out/test/test.txt", "w"));
    return 0;
}

