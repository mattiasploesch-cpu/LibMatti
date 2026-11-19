





#ifndef MATTICRAFT_MODULEREADER_H
#define MATTICRAFT_MODULEREADER_H

// Java: interface ModuleReader
// C has no interfaces; implementations (e.g. LIBMATTI_CL_JarModuleReader)
// provide the methods:
//   void *find(const char *name);  // Java: Optional<URI> find(String name)
//   void *open(const char *name);  // Java: Optional<InputStream> open(String name)
//   void *list(void);              // Java: Stream<String> list()
//   void close(void);              // Java: void close()
typedef struct LIBMATTI_JL_ModuleReader LIBMATTI_JL_ModuleReader;

#endif
