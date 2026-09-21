#ifndef MATTICRAFT_JAVAX_SWING_JOPTIONPANE_H
#define MATTICRAFT_JAVAX_SWING_JOPTIONPANE_H

#include "libmatti/javax/swing/ImageIcon.h"

// Java: public static final int ERROR_MESSAGE = 0
#define LIBMATTI_JS_JOptionPane_ERROR_MESSAGE 0

// Java: public static void showMessageDialog(Component parentComponent, Object message, String title,
//                                            int messageType, Icon icon)
void LIBMATTI_JS_JOptionPane_ShowMessageDialog(void *parentComponent, const char *message, const char *title,
                                               int messageType, LIBMATTI_JS_ImageIcon *icon);

#endif //MATTICRAFT_JAVAX_SWING_JOPTIONPANE_H
