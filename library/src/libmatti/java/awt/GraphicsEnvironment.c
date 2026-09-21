#include "libmatti/java/awt/GraphicsEnvironment.h"

#include "libmatti/java/lang/System.h"

#include <string.h>

// Java: private static Boolean headless - resolved once
static int headlessComputed = 0;
static int headlessValue = 0;

int LIBMATTI_JAWT_GraphicsEnvironment_IsHeadless(void)
{
    if (!headlessComputed)
    {
        headlessComputed = 1;

        // Java: String nm = System.getProperty("java.awt.headless")
        const char *property = LIBMATTI_JL_System_GetProperty("java.awt.headless");
        if (property != NULL)
        {
            // Java: headless = Boolean.valueOf(nm)
            headlessValue = strcmp(property, "true") == 0;
        }
        else
        {
            const char *osName = LIBMATTI_JL_System_GetProperty("os.name");
            if (osName != NULL && strstr(osName, "OS X") != NULL)
            {
                // Java: on macOS there is always a display
                headlessValue = 0;
            }
            else
            {
                // Java: getHeadlessPropertyFromEnv() - a display-less environment is headless
                headlessValue = LIBMATTI_JL_System_Getenv("DISPLAY") == NULL;
            }
        }
    }

    return headlessValue;
}
