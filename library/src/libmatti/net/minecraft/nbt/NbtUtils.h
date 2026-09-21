// Port of the SNBT writers: net.minecraft.nbt.StringTagVisitor (the plain toString
// form) and net.minecraft.nbt.TextComponentTagVisitor (the pretty printer the
// crash reports and /data output use; the chat-formatting colors collapse to
// plain text in the C port).

#ifndef MATTICRAFT_NET_MINECRAFT_NBT_NBTUTILS_H
#define MATTICRAFT_NET_MINECRAFT_NBT_NBTUTILS_H

#include "libmatti/net/minecraft/nbt/Tag.h"

// Java: StringTagVisitor - the single-line form; the caller frees the string.
// Compounds print their keys sorted (Entry.comparingByKey) and quote keys that are
// not [A-Za-z._]+[A-Za-z0-9._+-]* or true/false (case-insensitive).
char *LIBMATTI_MC_Nbt_StringTagVisitor_Visit(const LIBMATTI_MC_Nbt_Tag *tag);
// the CompoundTag.toString() alias used by the callers
char *LIBMATTI_MC_Nbt_StringTagVisitor_Print(const LIBMATTI_MC_Nbt_Tag *tag);

// Java: TextComponentTagVisitor(String indentation) + visit(tag) - the pretty form.
// indentation "" collapses to the single-line form with ", " separators; compounds
// and non-numeric lists wrap when indentation is set, 128-entry folding as in Java.
char *LIBMATTI_MC_Nbt_TextComponentTagVisitor_Visit(const char *indentation, const LIBMATTI_MC_Nbt_Tag *tag);

#endif //MATTICRAFT_NET_MINECRAFT_NBT_NBTUTILS_H
