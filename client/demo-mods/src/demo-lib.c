//
// Demo mod com.demo.lib.
// The manifest lives in the ELF section ".matti_manifest" and is read by the
// bootstrap launcher exactly like the manifest of a jar.
//
#include <stddef.h>

__attribute__((used, section(".matti_manifest")))
const char LIBMATTI_MOD_MANIFEST[] =
    "Manifest-Version: 1.0\n"
    "Automatic-Module-Name: com.demo.lib\n"
    "\n";

// mod payload (the native equivalent of the jar's classes)
int demo_lib_version(void)
{
    return 100;
}
