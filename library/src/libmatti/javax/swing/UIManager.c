// Port of javax.swing.UIManager.
// Java loads and installs a LookAndFeel class; the port keeps the same lifecycle with the
// cross-platform Metal defaults installed (getSystemLookAndFeelClassName reports the platform
// class name exactly like Java, the installation itself has no Swing toolkit behind it).

#include "libmatti/javax/swing/UIManager.h"

#include "libmatti/java/lang/System.h"

#include <stdlib.h>
#include <string.h>

// Java: the installed look and feel (UIManager.getLookAndFeel())
static char *installedLookAndFeel = NULL;

// Java: the system look and feel of the platform (GTK on Linux desktops,
// Windows, Aqua on macOS). Without a Swing toolkit the platform look and feel
// cannot be loaded, so the cross-platform Metal one is reported.
const char *LIBMATTI_JS_UIManager_GetSystemLookAndFeelClassName(void)
{
    const char *osName = LIBMATTI_JL_System_GetProperty("os.name");
    if (osName != NULL && strstr(osName, "Windows") != NULL)
        return "javax.swing.plaf.windows.WindowsLookAndFeel";
    if (osName != NULL && strstr(osName, "OS X") != NULL)
        return "com.apple.laf.AquaLookAndFeel";
    return "javax.swing.plaf.metal.MetalLookAndFeel";
}

// Java: public static void setLookAndFeel(String className)
int LIBMATTI_JS_UIManager_SetLookAndFeel(const char *className)
{
    if (className == NULL) return 0;

    // Java: the class is loaded and instantiated; an unknown class name throws
    // ClassNotFoundException. The port only knows the Metal and platform class names.
    const char *known[] = {
        "javax.swing.plaf.metal.MetalLookAndFeel",
        "javax.swing.plaf.windows.WindowsLookAndFeel",
        "com.apple.laf.AquaLookAndFeel",
        "javax.swing.plaf.nimbus.NimbusLookAndFeel",
        NULL};

    int known_name = 0;
    for (int i = 0; known[i] != NULL; i++)
        if (strcmp(className, known[i]) == 0)
        {
            known_name = 1;
            break;
        }
    if (!known_name) return 0;

    // Java: the look and feel is installed as the current one
    free(installedLookAndFeel);
    installedLookAndFeel = strdup(className);
    return 1;
}
