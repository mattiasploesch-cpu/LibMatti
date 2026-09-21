// Port of net.neoforged.fml.loading.progress.ProgressMeter.

#ifndef MATTICRAFT_FML_PROGRESS_PROGRESSMETER_H
#define MATTICRAFT_FML_PROGRESS_PROGRESSMETER_H

#include "libmatti/net/neoforged/fml/loading/progress/Message.h"

// Java: public final class ProgressMeter
typedef struct LIBMATTI_FML_ProgressMeter LIBMATTI_FML_ProgressMeter;

// Java: public ProgressMeter(String name, int steps, int current, Message label)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_ProgressMeter_New(const char *name, int steps, int current,
                                                           LIBMATTI_FML_Message *label);
void LIBMATTI_FML_ProgressMeter_Free(LIBMATTI_FML_ProgressMeter *progressMeter);

// Java: public String name()
const char *LIBMATTI_FML_ProgressMeter_Name(const LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public int steps()
int LIBMATTI_FML_ProgressMeter_Steps(const LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public int current()
int LIBMATTI_FML_ProgressMeter_Current(const LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public Message label()
LIBMATTI_FML_Message *LIBMATTI_FML_ProgressMeter_Label(const LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public void increment()
void LIBMATTI_FML_ProgressMeter_Increment(LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public void complete()
void LIBMATTI_FML_ProgressMeter_Complete(LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public float progress()
float LIBMATTI_FML_ProgressMeter_Progress(const LIBMATTI_FML_ProgressMeter *progressMeter);
// Java: public void setAbsolute(int absolute)
void LIBMATTI_FML_ProgressMeter_SetAbsolute(LIBMATTI_FML_ProgressMeter *progressMeter, int absolute);
// Java: public void label(String message) - C names the setter after the Java method
void LIBMATTI_FML_ProgressMeter_SetLabel(LIBMATTI_FML_ProgressMeter *progressMeter, const char *message);

#endif //MATTICRAFT_FML_PROGRESS_PROGRESSMETER_H
