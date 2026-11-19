//
// Demo launch setup for the matticraft target.
//

#ifndef MATTICRAFT_DEMO_SERVICE_H
#define MATTICRAFT_DEMO_SERVICE_H

// Registers the demo providers in the ServiceLoader registry. C has no
// META-INF/services, so the host has to declare them; must run before the
// launcher reads the registry (i.e. before start_from_bsl).
void demo_register_services(void);

#endif //MATTICRAFT_DEMO_SERVICE_H
