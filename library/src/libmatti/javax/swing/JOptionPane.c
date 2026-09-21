#include "libmatti/javax/swing/JOptionPane.h"

#include "libmatti/java/io/PrintWriter.h"
#include "libmatti/java/lang/System.h"

// Java: shows a modal dialog on the event dispatch thread. The C port has no
// Swing toolkit, so the dialog content is written to the error stream.
void LIBMATTI_JS_JOptionPane_ShowMessageDialog(void *parentComponent, const char *message, const char *title,
                                               int messageType, LIBMATTI_JS_ImageIcon *icon)
{
    (void)parentComponent;
    (void)messageType;
    (void)icon;

    LIBMATTI_JI_PrintWriter *err = LIBMATTI_JL_System_Err();
    LIBMATTI_JI_PrintWriter_Println(err, title);
    LIBMATTI_JI_PrintWriter_Println(err, message);
    LIBMATTI_JI_PrintWriter_Flush(err);
}
