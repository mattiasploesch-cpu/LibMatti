#ifndef MATTICRAFT_LWJGL_TINYFD_TINYFILEDIALOGS_H
#define MATTICRAFT_LWJGL_TINYFD_TINYFILEDIALOGS_H

// Java: public class TinyFileDialogs
// Java: public static native int tinyfd_messageBox(String aTitle, String aMessage, String aDialogType,
//                                                  String aIconType, int aDefaultButton)
int LIBMATTI_LWJGL_TINYFD_TinyFileDialogs_TinyfdMessageBox(const char *title, const char *message,
                                                          const char *dialogType, const char *iconType,
                                                          int defaultButton);

#endif //MATTICRAFT_LWJGL_TINYFD_TINYFILEDIALOGS_H
