// Port of net.neoforged.bus.IWrapperListener.
// Java: public interface IWrapperListener { EventListener getWithoutCheck(); }
// The C port marks a wrapper with the getWithoutCheck function pointer on EventListener itself
// (ListenerList tests that pointer instead of performing an instanceof), so the interface carries
// no struct of its own.

#ifndef MATTICRAFT_BUS_IWRAPPERLISTENER_H
#define MATTICRAFT_BUS_IWRAPPERLISTENER_H

#include "libmatti/net/neoforged/bus/api/EventListener.h"

#endif //MATTICRAFT_BUS_IWRAPPERLISTENER_H
