// Port of net.neoforged.fml.loading.modscan.Scanner.

#ifndef MATTICRAFT_FML_LOADING_MODSCAN_SCANNER_H
#define MATTICRAFT_FML_LOADING_MODSCAN_SCANNER_H

#include "libmatti/net/neoforged/fml/loading/moddiscovery/ModFile.h"
#include "libmatti/net/neoforged/neoforgespi/language/ModFileScanData.h"

typedef struct
{
    LIBMATTI_FML_ModFile *fileToScan;
} LIBMATTI_FML_Scanner;

// Java: public Scanner(ModFile fileToScan)
LIBMATTI_FML_Scanner *LIBMATTI_FML_Scanner_New(LIBMATTI_FML_ModFile *fileToScan);
void LIBMATTI_FML_Scanner_Free(LIBMATTI_FML_Scanner *scanner);

// Java: public ModFileScanData scan()
LIBMATTI_NEOFORGESPI_ModFileScanData *LIBMATTI_FML_Scanner_Scan(const LIBMATTI_FML_Scanner *scanner);

#endif //MATTICRAFT_FML_LOADING_MODSCAN_SCANNER_H
