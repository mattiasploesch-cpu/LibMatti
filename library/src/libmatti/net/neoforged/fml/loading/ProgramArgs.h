// Port of net.neoforged.fml.loading.ProgramArgs.

#ifndef MATTICRAFT_FML_PROGRAMARGS_H
#define MATTICRAFT_FML_PROGRAMARGS_H

#include <stddef.h>

// Java: public final class ProgramArgs
typedef struct LIBMATTI_FML_ProgramArgs LIBMATTI_FML_ProgramArgs;

// Java: public static ProgramArgs from(String... args)
LIBMATTI_FML_ProgramArgs *LIBMATTI_FML_ProgramArgs_From(int argc, char *argv[]);
void LIBMATTI_FML_ProgramArgs_Free(LIBMATTI_FML_ProgramArgs *programArgs);

// Java: public void addRaw(String arg)
void LIBMATTI_FML_ProgramArgs_AddRaw(LIBMATTI_FML_ProgramArgs *programArgs, const char *arg);
// Java: public void addArg(boolean split, String raw, String value)
void LIBMATTI_FML_ProgramArgs_AddArg(LIBMATTI_FML_ProgramArgs *programArgs, int split, const char *raw,
                                     const char *value);
// Java: public String[] getArguments()
char **LIBMATTI_FML_ProgramArgs_GetArguments(const LIBMATTI_FML_ProgramArgs *programArgs, size_t *count);
// Java: public boolean hasValue(String key)
int LIBMATTI_FML_ProgramArgs_HasValue(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key);
// Java: public String get(String key)
const char *LIBMATTI_FML_ProgramArgs_Get(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key);
// Java: public String getOrDefault(String key, String value)
const char *LIBMATTI_FML_ProgramArgs_GetOrDefault(const LIBMATTI_FML_ProgramArgs *programArgs, const char *key,
                                                  const char *value);
// Java: public void put(String key, String value)
void LIBMATTI_FML_ProgramArgs_Put(LIBMATTI_FML_ProgramArgs *programArgs, const char *key, const char *value);
// Java: public void putLazy(String key, String value)
void LIBMATTI_FML_ProgramArgs_PutLazy(LIBMATTI_FML_ProgramArgs *programArgs, const char *key, const char *value);
// Java: public String remove(String key) - the returned value is a new string the caller frees
char *LIBMATTI_FML_ProgramArgs_Remove(LIBMATTI_FML_ProgramArgs *programArgs, const char *key);

#endif //MATTICRAFT_FML_PROGRAMARGS_H
