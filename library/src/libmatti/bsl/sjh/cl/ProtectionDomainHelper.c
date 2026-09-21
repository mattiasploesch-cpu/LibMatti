#include "ProtectionDomainHelper.h"

#include <stdlib.h>
#include <string.h>

struct LIBMATTI_CL_CodeSource
{
    LIBMATTI_JN_URI *url;
    void **signers;
};

struct LIBMATTI_CL_ProtectionDomain
{
    LIBMATTI_CL_CodeSource *codeSource;
};

// Java: Map<URL, CodeSource> csCache - computeIfAbsent
static struct
{
    LIBMATTI_CL_CodeSource **values;
    size_t count;
} csCache = {0};

// Java: Map<CodeSource, ProtectionDomain> pdCache - computeIfAbsent
static struct
{
    LIBMATTI_CL_ProtectionDomain **values;
    size_t count;
} pdCache = {0};

LIBMATTI_CL_CodeSource *LIBMATTI_CL_ProtectionDomainHelper_CreateCodeSource(LIBMATTI_JN_URI *url, void **signers)
{
    // Java: synchronized (csCache) { return csCache.computeIfAbsent(url, u -> new CodeSource(url, signers)); }
    const char *urlStr = LIBMATTI_JN_URI_ToString(url);
    for (size_t i = 0; i < csCache.count; i++)
    {
        if (strcmp(LIBMATTI_JN_URI_ToString(csCache.values[i]->url), urlStr) == 0) return csCache.values[i];
    }

    LIBMATTI_CL_CodeSource *codeSource = calloc(1, sizeof(LIBMATTI_CL_CodeSource));
    codeSource->url = url;
    codeSource->signers = signers;
    csCache.values = realloc(csCache.values, sizeof(LIBMATTI_CL_CodeSource *) * (csCache.count + 1));
    csCache.values[csCache.count] = codeSource;
    csCache.count++;
    return codeSource;
}

LIBMATTI_CL_ProtectionDomain *LIBMATTI_CL_ProtectionDomainHelper_CreateProtectionDomain(LIBMATTI_CL_CodeSource *codeSource)
{
    // Java: synchronized (pdCache) { return pdCache.computeIfAbsent(codeSource, cs -> {
    //          Permissions perms = new Permissions(); perms.add(new AllPermission());
    //          return new ProtectionDomain(codeSource, perms, cl, null); }); }
    for (size_t i = 0; i < pdCache.count; i++)
    {
        if (pdCache.values[i]->codeSource == codeSource) return pdCache.values[i];
    }

    LIBMATTI_CL_ProtectionDomain *domain = calloc(1, sizeof(LIBMATTI_CL_ProtectionDomain));
    domain->codeSource = codeSource;
    pdCache.values = realloc(pdCache.values, sizeof(LIBMATTI_CL_ProtectionDomain *) * (pdCache.count + 1));
    pdCache.values[pdCache.count] = domain;
    pdCache.count++;
    return domain;
}

void LIBMATTI_CL_ProtectionDomainHelper_TrySetPackageModule(void *pkg, void *module)
{
    // Java: PKG_MODULE_HANDLE.set(pkg, module) - JVM VarHandle reflection, no C equivalent
    (void)pkg;
    (void)module;
}

void *LIBMATTI_CL_ProtectionDomainHelper_TryDefinePackage(const char *name, const LIBMATTI_JU_Manifest *man,
                                                         LIBMATTI_CL_TrustedEntries trustedEntries,
                                                         LIBMATTI_CL_DefinePackage definePackage,
                                                         LIBMATTI_CL_GetDefinedPackage getDefinedPackage)
{
    // Java: final var pname = name.substring(0, name.lastIndexOf('.'));
    const char *lastDot = strrchr(name, '.');
    if (lastDot == NULL) return NULL; // Java: IllegalArgumentException
    size_t pnameLen = (size_t)(lastDot - name);
    char *pname = strndup(name, pnameLen);

    if (!getDefinedPackage(pname))
    {
        // Java: synchronized (classLoader) { if (getDefinedPackage(pname) != null) return ...; }
        if (getDefinedPackage(pname))
        {
            free(pname);
            return NULL; // Java: returns the existing package
        }

        // Java: String path = pname.replace('.', '/').concat("/");
        size_t pathLen = pnameLen + 1;
        char *path = malloc(pathLen + 1);
        for (size_t i = 0; i < pnameLen; i++) path[i] = pname[i] == '.' ? '/' : pname[i];
        path[pnameLen] = '/';
        path[pathLen] = '\0';

        const char *specTitle = NULL, *specVersion = NULL, *specVendor = NULL;
        const char *implTitle = NULL, *implVersion = NULL, *implVendor = NULL;

        if (man != NULL)
        {
            // Java: Attributes attr = trustedEntries.apply(path);
            const LIBMATTI_JU_Attributes *attr = trustedEntries(path);
            if (attr != NULL)
            {
                specTitle = LIBMATTI_JU_Attributes_GetValue(attr, "Specification-Title");
                specVersion = LIBMATTI_JU_Attributes_GetValue(attr, "Specification-Version");
                specVendor = LIBMATTI_JU_Attributes_GetValue(attr, "Specification-Vendor");
                implTitle = LIBMATTI_JU_Attributes_GetValue(attr, "Implementation-Title");
                implVersion = LIBMATTI_JU_Attributes_GetValue(attr, "Implementation-Version");
                implVendor = LIBMATTI_JU_Attributes_GetValue(attr, "Implementation-Vendor");
            }
            // Java: attr = man.getMainAttributes();
            if (specTitle == NULL) specTitle = LIBMATTI_JU_Manifest_GetMainValue(man, "Specification-Title");
            if (specVersion == NULL) specVersion = LIBMATTI_JU_Manifest_GetMainValue(man, "Specification-Version");
            if (specVendor == NULL) specVendor = LIBMATTI_JU_Manifest_GetMainValue(man, "Specification-Vendor");
            if (implTitle == NULL) implTitle = LIBMATTI_JU_Manifest_GetMainValue(man, "Implementation-Title");
            if (implVersion == NULL) implVersion = LIBMATTI_JU_Manifest_GetMainValue(man, "Implementation-Version");
            if (implVendor == NULL) implVendor = LIBMATTI_JU_Manifest_GetMainValue(man, "Implementation-Vendor");
        }

        // Java: definePackage.apply(new String[]{pname, specTitle, specVersion, specVendor, implTitle, implVersion, implVendor});
        const char *args[7] = {pname, specTitle, specVersion, specVendor, implTitle, implVersion, implVendor};
        void *result = definePackage(args);
        free(path);
        free(pname);
        return result;
    }

    free(pname);
    return NULL; // Java: returns classLoader.getDefinedPackage(pname)
}