// Port of net.neoforged.bus.api.IEventClassChecker.

#include "libmatti/net/neoforged/bus/api/IEventClassChecker.h"

// Java: void check(Class<? extends Event> eventClass)
void LIBMATTI_BUS_IEventClassChecker_Check(const LIBMATTI_BUS_IEventClassChecker *checker,
                                           LIBMATTI_JL_Class *eventClass)
{
    checker->check(checker->self, eventClass);
}
