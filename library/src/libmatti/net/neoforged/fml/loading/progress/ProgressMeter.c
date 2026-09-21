#include "libmatti/net/neoforged/fml/loading/progress/ProgressMeter.h"

#include "libmatti/net/neoforged/fml/loading/progress/StartupNotificationManager.h"

#include <stdlib.h>
#include <string.h>

// Java: public final class ProgressMeter
struct LIBMATTI_FML_ProgressMeter
{
    char *name;
    int steps;
    // Java: AtomicInteger current
    int current;
    LIBMATTI_FML_Message *label;
};

// Java: public ProgressMeter(String name, int steps, int current, Message label)
LIBMATTI_FML_ProgressMeter *LIBMATTI_FML_ProgressMeter_New(const char *name, int steps, int current,
                                                           LIBMATTI_FML_Message *label)
{
    LIBMATTI_FML_ProgressMeter *progressMeter = calloc(1, sizeof(LIBMATTI_FML_ProgressMeter));
    progressMeter->name = name != NULL ? strdup(name) : NULL;
    progressMeter->steps = steps;
    progressMeter->current = current;
    progressMeter->label = label;
    return progressMeter;
}

void LIBMATTI_FML_ProgressMeter_Free(LIBMATTI_FML_ProgressMeter *progressMeter)
{
    if (progressMeter == NULL) return;

    free(progressMeter->name);
    LIBMATTI_FML_Message_Free(progressMeter->label);
    free(progressMeter);
}

// Java: public String name()
const char *LIBMATTI_FML_ProgressMeter_Name(const LIBMATTI_FML_ProgressMeter *progressMeter)
{
    return progressMeter->name;
}

// Java: public int steps()
int LIBMATTI_FML_ProgressMeter_Steps(const LIBMATTI_FML_ProgressMeter *progressMeter)
{
    return progressMeter->steps;
}

// Java: public int current()
int LIBMATTI_FML_ProgressMeter_Current(const LIBMATTI_FML_ProgressMeter *progressMeter)
{
    return progressMeter->current;
}

// Java: public Message label()
LIBMATTI_FML_Message *LIBMATTI_FML_ProgressMeter_Label(const LIBMATTI_FML_ProgressMeter *progressMeter)
{
    return progressMeter->label;
}

// Java: public void increment()
void LIBMATTI_FML_ProgressMeter_Increment(LIBMATTI_FML_ProgressMeter *progressMeter)
{
    progressMeter->current++;
}

// Java: public void complete()
void LIBMATTI_FML_ProgressMeter_Complete(LIBMATTI_FML_ProgressMeter *progressMeter)
{
    LIBMATTI_FML_StartupNotificationManager_PopBar(progressMeter);
}

// Java: public float progress()
float LIBMATTI_FML_ProgressMeter_Progress(const LIBMATTI_FML_ProgressMeter *progressMeter)
{
    return (float) progressMeter->current / (float) progressMeter->steps;
}

// Java: public void setAbsolute(int absolute)
void LIBMATTI_FML_ProgressMeter_SetAbsolute(LIBMATTI_FML_ProgressMeter *progressMeter, int absolute)
{
    progressMeter->current = absolute;
}

// Java: public void label(String message)
void LIBMATTI_FML_ProgressMeter_SetLabel(LIBMATTI_FML_ProgressMeter *progressMeter, const char *message)
{
    LIBMATTI_FML_Message_Free(progressMeter->label);
    progressMeter->label = LIBMATTI_FML_Message_New(message, LIBMATTI_FML_Message_MessageType_ML);
}
