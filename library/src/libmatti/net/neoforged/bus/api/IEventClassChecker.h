// Port of net.neoforged.bus.api.IEventClassChecker.

#ifndef MATTICRAFT_BUS_API_IEVENTCLASSCHECKER_H
#define MATTICRAFT_BUS_API_IEVENTCLASSCHECKER_H

#include "libmatti/java/lang/Class.h"

// Java: @FunctionalInterface public interface IEventClassChecker
typedef struct LIBMATTI_BUS_IEventClassChecker LIBMATTI_BUS_IEventClassChecker;

struct LIBMATTI_BUS_IEventClassChecker
{
    void *self;

    // Java: void check(Class<? extends Event> eventClass) throws IllegalArgumentException
    void (*check)(void *self, LIBMATTI_JL_Class *eventClass);
};

// Java: void check(Class<? extends Event> eventClass)
void LIBMATTI_BUS_IEventClassChecker_Check(const LIBMATTI_BUS_IEventClassChecker *checker,
                                           LIBMATTI_JL_Class *eventClass);

#endif //MATTICRAFT_BUS_API_IEVENTCLASSCHECKER_H
