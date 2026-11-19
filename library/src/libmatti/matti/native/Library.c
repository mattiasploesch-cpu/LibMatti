#if !defined(_GNU_SOURCE) && !defined(_WIN32)
#define _GNU_SOURCE
#endif

#include "libmatti/matti/native/Library.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#if defined(__linux__) || defined(__FreeBSD__)
#include <elf.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#endif

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif






static void *platform_open(const char *path)
{
#if defined(_WIN32)
    return (void *) LoadLibraryA(path);
#else
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

static int platform_directory_separator(char character)
{
#if defined(_WIN32)
    return character == ';';
#else
    return character == ':';
#endif
}



static int module_directory(char *buffer, size_t size)
{
#if defined(_WIN32)
    DWORD written = GetModuleFileNameA(NULL, buffer, (DWORD) size);
    if (written == 0 || written >= size)
        return 0;
#elif defined(__APPLE__)
    uint32_t length = (uint32_t) size;
    if (_NSGetExecutablePath(buffer, &length) != 0)
        return 0;
#elif defined(__FreeBSD__)
    int name[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    size_t length = size;
    if (sysctl(name, 4, buffer, &length, NULL, 0) != 0 || length == 0)
        return 0;
    buffer[length - 1] = '\0';

#else
    ssize_t written = readlink("/proc/self/exe", buffer, size - 1);
    if (written <= 0)
        return 0;
    buffer[written] = '\0';
#endif
    char *slash = strrchr(buffer, '/');
#if defined(_WIN32)
    char *backslash = strrchr(buffer, '\\');
    if (backslash != NULL && (slash == NULL || backslash > slash))
        slash = backslash;
#endif
    if (slash != NULL)
        *slash = '\0';
    return 1;
}

static void *open_in_directory(const char *directory, const char *name)
{
    char path[4096];
    int written = snprintf(path, sizeof(path), "%s/%s", directory, name);
    if (written <= 0 || (size_t) written >= sizeof(path))
        return NULL;
    return platform_open(path);
}

static void *try_directories(const char *const *names, const char *directories)
{
    char *copy = strdup(directories);
    if (copy == NULL)
        return NULL;

    void *handle = NULL;
    char *cursor = copy;
    while (cursor != NULL && *cursor != '\0' && handle == NULL)
    {
        char *next = cursor;
        while (*next != '\0' && !platform_directory_separator(*next))
            next++;
        if (*next != '\0')
            *next++ = '\0';
        else
            next = NULL;

        for (int i = 0; names[i] != NULL && handle == NULL; i++)
            handle = open_in_directory(cursor, names[i]);
        cursor = next;
    }

    free(copy);
    return handle;
}

static void *try_module_directories(const char *const *names)
{
    char directory[4096];
    if (!module_directory(directory, sizeof(directory)))
        return NULL;

    for (int i = 0; names[i] != NULL; i++)
    {
        void *handle = open_in_directory(directory, names[i]);
        if (handle != NULL)
            return handle;
    }




    static const char *const relative[] = {"../lib", "../Frameworks", "../lib64", NULL};
    for (int index = 0; relative[index] != NULL; index++)
    {
        char candidate[4096];
        int written = snprintf(candidate, sizeof(candidate), "%s/%s", directory, relative[index]);
        if (written <= 0 || (size_t) written >= sizeof(candidate))
            continue;
        for (int i = 0; names[i] != NULL; i++)
        {
            void *handle = open_in_directory(candidate, names[i]);
            if (handle != NULL)
                return handle;
        }
    }
    return NULL;
}

#if defined(_WIN32)
static const char *const default_directories[] = {NULL};
#elif defined(__APPLE__)
static const char *const default_directories[] = {
    "/opt/homebrew/lib",
    "/opt/local/lib",
    "/usr/local/lib",
    "/usr/lib",
    NULL};
#else




static const char *const default_directories[] = {
    "/run/current-system/sw/lib",
    "/nix/var/nix/profiles/default/lib",
    "/nix/profile/lib",
    "/usr/local/lib",
    "/usr/lib",
    NULL};
#endif

void *LIBMATTI_NATIVE_OpenLibrary(const char *const *names)
{
    if (names == NULL)
        return NULL;




    for (int i = 0; names[i] != NULL; i++)
    {
        void *handle = platform_open(names[i]);
        if (handle != NULL)
            return handle;
    }

    void *handle = try_module_directories(names);
    if (handle != NULL)
        return handle;



    const char *override_path = getenv("MATTI_LIBRARY_PATH");
    if (override_path != NULL)
    {
        handle = try_directories(names, override_path);
        if (handle != NULL)
            return handle;
    }

#if !defined(_WIN32)



    const char *profiles = getenv("NIX_PROFILES");
    if (profiles != NULL)
    {
        char *copy = strdup(profiles);
        if (copy != NULL)
        {
            char *saveptr = NULL;
            for (char *directory = strtok_r(copy, " \t", &saveptr);
                 directory != NULL && handle == NULL;
                 directory = strtok_r(NULL, " \t", &saveptr))
            {
                for (int i = 0; names[i] != NULL && handle == NULL; i++)
                    handle = open_in_directory(directory, names[i]);
            }
            free(copy);
        }
        if (handle != NULL)
            return handle;
    }
#endif

    for (int directory = 0; default_directories[directory] != NULL; directory++)
    {
        for (int i = 0; names[i] != NULL; i++)
        {
            handle = open_in_directory(default_directories[directory], names[i]);
            if (handle != NULL)
                return handle;
        }
    }



    const char *home = getenv("HOME");
#if defined(_WIN32)
    if (home == NULL)
        home = getenv("USERPROFILE");
#endif
    if (home != NULL)
    {
        static const char *const home_profiles[] = {
            "/.nix-profile/lib",
            "/.local/state/nix/profile/lib",
            NULL};
        for (int profile = 0; home_profiles[profile] != NULL; profile++)
        {
            char directory[4096];
            int written = snprintf(directory, sizeof(directory), "%s%s", home, home_profiles[profile]);
            if (written <= 0 || (size_t) written >= sizeof(directory))
                continue;
            for (int i = 0; names[i] != NULL; i++)
            {
                handle = open_in_directory(directory, names[i]);
                if (handle != NULL)
                    return handle;
            }
        }
    }

    return NULL;
}










#if defined(__linux__) || defined(__FreeBSD__)
static int read_section_from_file(const char *path, const char *section_name, unsigned char **out_data,
                                  size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return 0;

    Elf64_Ehdr header;
    int found = 0;
    if (fread(&header, sizeof(header), 1, file) == 1 && header.e_ident[EI_CLASS] == ELFCLASS64 &&
        header.e_shoff != 0 && header.e_shnum != 0 && header.e_shstrndx < header.e_shnum)
    {
        Elf64_Shdr *sections = calloc(header.e_shnum, sizeof(*sections));
        if (sections != NULL &&
            fseek(file, (long) header.e_shoff, SEEK_SET) == 0 &&
            fread(sections, sizeof(*sections), header.e_shnum, file) == header.e_shnum)
        {
            Elf64_Shdr *name_header = &sections[header.e_shstrndx];
            char *name_table = name_header->sh_size != 0 ? malloc((size_t) name_header->sh_size + 1) : NULL;
            if (name_table != NULL &&
                fseek(file, (long) name_header->sh_offset, SEEK_SET) == 0 &&
                fread(name_table, 1, (size_t) name_header->sh_size, file) == name_header->sh_size)
            {
                name_table[name_header->sh_size] = '\0';
                for (uint16_t i = 0; i < header.e_shnum; i++)
                {
                    if (strcmp(name_table + sections[i].sh_name, section_name) != 0)
                        continue;
                    if (sections[i].sh_size == 0)
                        break;

                    unsigned char *data = malloc((size_t) sections[i].sh_size);
                    if (data != NULL &&
                        fseek(file, (long) sections[i].sh_offset, SEEK_SET) == 0 &&
                        fread(data, 1, (size_t) sections[i].sh_size, file) == sections[i].sh_size)
                    {
                        *out_data = data;
                        *out_size = (size_t) sections[i].sh_size;
                        found = 1;
                    }
                    else
                    {
                        free(data);
                    }
                    break;
                }
            }
            free(name_table);
        }
        free(sections);
    }
    fclose(file);
    return found;
}
#elif defined(_WIN32)
static int read_section_from_file(const char *path, const char *section_name, unsigned char **out_data,
                                  size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return 0;

    IMAGE_DOS_HEADER dos;
    if (fread(&dos, sizeof(dos), 1, file) != 1 || dos.e_magic != IMAGE_DOS_SIGNATURE)
    {
        fclose(file);
        return 0;
    }
    if (fseek(file, dos.e_lfanew, SEEK_SET) != 0)
    {
        fclose(file);
        return 0;
    }

    DWORD signature = 0;
    IMAGE_FILE_HEADER file_header;
    if (fread(&signature, sizeof(signature), 1, file) != 1 || signature != IMAGE_NT_SIGNATURE ||
        fread(&file_header, sizeof(file_header), 1, file) != 1)
    {
        fclose(file);
        return 0;
    }



    if (fseek(file, file_header.SizeOfOptionalHeader, SEEK_CUR) != 0)
    {
        fclose(file);
        return 0;
    }

    int found = 0;
    for (WORD index = 0; index < file_header.NumberOfSections && !found; index++)
    {
        IMAGE_SECTION_HEADER section;
        if (fread(&section, sizeof(section), 1, file) != 1)
            break;
        char name[9];
        memcpy(name, section.Name, 8);
        name[8] = '\0';
        if (strcmp(name, section_name) != 0 || section.SizeOfRawData == 0)
            continue;

        unsigned char *data = malloc(section.SizeOfRawData);
        if (data == NULL)
            continue;
        if (fseek(file, (long) section.PointerToRawData, SEEK_SET) == 0 &&
            fread(data, 1, section.SizeOfRawData, file) == section.SizeOfRawData)
        {
            *out_data = data;
            *out_size = section.SizeOfRawData;
            found = 1;
        }
        else
        {
            free(data);
        }
    }
    fclose(file);
    return found;
}
#else
static int read_section_from_file(const char *path, const char *section_name, unsigned char **out_data,
                                  size_t *out_size)
{
    (void) path;
    (void) section_name;
    (void) out_data;
    (void) out_size;
    return 0;
}
#endif





static int read_self_section(const char *section_name, unsigned char **out_data, size_t *out_size)
{
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#if !defined(__APPLE__)
    Dl_info info;
    if (dladdr((void *) (size_t) &read_self_section, &info) != 0 && info.dli_fname != NULL &&
        info.dli_fname[0] != '\0')
    {
        if (read_section_from_file(info.dli_fname, section_name, out_data, out_size))
            return 1;
    }
#endif
#endif

    char path[4096];
    if (!module_directory(path, sizeof(path)))
        return 0;
#if defined(_WIN32)
    char executable[4096];
    DWORD written = GetModuleFileNameA(NULL, executable, (DWORD) sizeof(executable));
    if (written == 0 || written >= sizeof(executable))
        return 0;
    return read_section_from_file(executable, section_name, out_data, out_size);
#elif defined(__APPLE__)
    (void) path;
    return 0;
#else
    char executable[4096];
    int written = snprintf(executable, sizeof(executable), "%s", path);
    (void) written;


#if defined(__FreeBSD__)
    int name[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    size_t length = sizeof(executable);
    if (sysctl(name, 4, executable, &length, NULL, 0) != 0)
        return 0;
#else
    ssize_t length = readlink("/proc/self/exe", executable, sizeof(executable) - 1);
    if (length <= 0)
        return 0;
    executable[length] = '\0';
#endif
    return read_section_from_file(executable, section_name, out_data, out_size);
#endif
}

int LIBMATTI_NATIVE_ReadEmbeddedSection(const char *section_name, unsigned char **out_data,
                                        size_t *out_size)
{
    if (section_name == NULL || out_data == NULL || out_size == NULL)
        return 0;
    return read_self_section(section_name, out_data, out_size);
}

void *LIBMATTI_NATIVE_OpenEmbeddedLibrary(const char *section_name)
{
    unsigned char *data = NULL;
    size_t size = 0;
    if (section_name == NULL || !LIBMATTI_NATIVE_ReadEmbeddedSection(section_name, &data, &size))
        return NULL;

#if defined(_WIN32)
    char temporary_path[4096];
    char temporary_file[4096];
    DWORD length = GetTempPathA((DWORD) sizeof(temporary_path), temporary_path);
    if (length == 0 || length >= sizeof(temporary_path) ||
        GetTempFileNameA(temporary_path, "matti", 0, temporary_file) == 0)
    {
        free(data);
        return NULL;
    }

    void *handle = NULL;
    FILE *file = fopen(temporary_file, "wb");
    if (file != NULL)
    {
        if (fwrite(data, 1, size, file) == size)
        {
            fclose(file);
            handle = platform_open(temporary_file);
        }
        else
        {
            fclose(file);
        }
    }


    DeleteFileA(temporary_file);
    free(data);
    return handle;
#else
    const char *tmp = getenv("TMPDIR");
    if (tmp == NULL || tmp[0] == '\0')
        tmp = "/tmp";

    char path[4096];
    int written = snprintf(path, sizeof(path), "%s/matti-native-XXXXXX", tmp);
    void *handle = NULL;
    if (written > 0 && (size_t) written < sizeof(path))
    {
        int fd = mkstemp(path);
        if (fd >= 0)
        {
            size_t offset = 0;
            int ok = 1;
            while (offset < size)
            {
                ssize_t count = write(fd, data + offset, size - offset);
                if (count <= 0)
                {
                    ok = 0;
                    break;
                }
                offset += (size_t) count;
            }
            close(fd);
            if (ok)
                handle = platform_open(path);


            unlink(path);
        }
    }

    free(data);
    return handle;
#endif
}
