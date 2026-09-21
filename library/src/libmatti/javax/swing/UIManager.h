#ifndef MATTICRAFT_JAVAX_SWING_UIMANAGER_H
#define MATTICRAFT_JAVAX_SWING_UIMANAGER_H

// Java: public class UIManager
// Java: public static String getSystemLookAndFeelClassName()
const char *LIBMATTI_JS_UIManager_GetSystemLookAndFeelClassName(void);

// Java: public static void setLookAndFeel(String className) throws ClassNotFoundException,
//       InstantiationException, IllegalAccessException, UnsupportedLookAndFeelException
// Returns 0 when the look and feel could not be installed (the callers catch and ignore).
int LIBMATTI_JS_UIManager_SetLookAndFeel(const char *className);

#endif //MATTICRAFT_JAVAX_SWING_UIMANAGER_H
