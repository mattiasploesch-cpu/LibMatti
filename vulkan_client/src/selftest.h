//
// The selftest phase of the matticraft client: the tests of libmatti live in
// the client, so one binary runs the game, the launcher and every harness.
//
// Java runs its tests as JUnit inside the same process; the C port runs them
// here, before the launcher starts. A harness is the main() of its own binary,
// so the phase re-executes the freshly built test binaries of the same build
// tree and reports their exit codes.
//

#ifndef MATTICRAFT_SELFTEST_H
#define MATTICRAFT_SELFTEST_H

// Runs every harness (modscan, soscan, mixinfacade, m4proof). Returns the
// number of failed harnesses; 0 means everything passed.
int selftest_run_all(void);

#endif //MATTICRAFT_SELFTEST_H
