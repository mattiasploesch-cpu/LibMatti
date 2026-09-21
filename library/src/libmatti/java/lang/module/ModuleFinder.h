//
// Created by administrator on 09.09.26.
//
// Port of java.lang.module.ModuleFinder (interface).

#ifndef MATTICRAFT_MODULEFINDER_H
#define MATTICRAFT_MODULEFINDER_H

// Java: interface ModuleFinder
// C has no interfaces; implementations (e.g. LIBMATTI_CL_JarModuleFinder)
// provide the methods:
//   find(name)     // Java: Optional<ModuleReference> find(String name)
//   findAll()      // Java: Set<ModuleReference> findAll()
typedef struct LIBMATTI_JL_ModuleFinder LIBMATTI_JL_ModuleFinder;

#endif //MATTICRAFT_MODULEFINDER_H