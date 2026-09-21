#include "libmatti/org/lwjgl/util/tinyfd/TinyFileDialogs.h"

#include "libmatti/java/io/PrintWriter.h"
#include "libmatti/java/lang/System.h"

// Java: the native tinyfiledialogs message box. The C port has no desktop
// toolkit to draw it with, so the dialog content is written to the error
// stream and the requested default button is reported as pressed.
int LIBMATTI_LWJGL_TINYFD_TinyFileDialogs_TinyfdMessageBox(const char *title, const char *message,
                                                          const char *dialogType, const char *iconType,
                                                          int defaultButton)
{
    (void)dialogType;
    (void)iconType;
    (void)defaultButton;

    LIBMATTI_JI_PrintWriter *err = LIBMATTI_JL_System_Err();
    LIBMATTI_JI_PrintWriter_Println(err, title);
    LIBMATTI_JI_PrintWriter_Println(err, message);
    LIBMATTI_JI_PrintWriter_Flush(err);
    return 1;
}
