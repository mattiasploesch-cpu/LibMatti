// Port of cpw.mods.modlauncher.api.ServiceRunner.

#ifndef MATTICRAFT_MODLAUNCHER_SERVICERUNNER_H
#define MATTICRAFT_MODLAUNCHER_SERVICERUNNER_H

// Java: interface ServiceRunner extends Function<ClassLoader, Class<?>> {
//           Class<?> run(ClassLoader loader) throws Throwable; }
typedef struct LIBMATTI_MLA_ServiceRunner
{
    void *(*run)(struct LIBMATTI_MLA_ServiceRunner *self, void *classLoader);
} LIBMATTI_MLA_ServiceRunner;

// Java: Class<?> run(ClassLoader loader)
void *LIBMATTI_MLA_ServiceRunner_Run(LIBMATTI_MLA_ServiceRunner *runner, void *classLoader);

#endif //MATTICRAFT_MODLAUNCHER_SERVICERUNNER_H
