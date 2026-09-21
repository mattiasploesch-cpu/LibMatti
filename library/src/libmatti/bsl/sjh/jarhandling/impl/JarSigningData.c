//
// Created by administrator on 09.09.26.
//

#include "JarSigningData.h"

#include "libmatti/cpw/modlauncher/LogManager.h"

// Java: static JarSigningData readJarSigningData(JarInputStream jis) - parses
// the META-INF/*.SF and META-INF/*.RSA signature entries.
// Skipped on purpose: the .so mods carry no Java signatures and a JCA crypto
// port brings nothing to the loader. unsigned jars behave identically.
static int warned;

LIBMATTI_JH_JarSigningData *LIBMATTI_JH_JarSigningData_Read(void)
{
    if (!warned)
    {
        warned = 1;
        LIBMATTI_ML_Logger *logger = LIBMATTI_ML_LogManager_GetLogger();
        LIBMATTI_ML_Logger_Debug(logger, NULL, "Jar signing verification skipped (jar signing data not read)");
    }
    return NULL; // Java: JarSigningData.EMPTY equivalent
}