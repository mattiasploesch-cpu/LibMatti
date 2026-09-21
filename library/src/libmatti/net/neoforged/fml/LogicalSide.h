// Port of net.neoforged.fml.LogicalSide.

#ifndef MATTICRAFT_FML_LOGICALSIDE_H
#define MATTICRAFT_FML_LOGICALSIDE_H

// Java: public enum LogicalSide { CLIENT, SERVER }
typedef enum
{
    LIBMATTI_FML_LogicalSide_CLIENT,
    LIBMATTI_FML_LogicalSide_SERVER
} LIBMATTI_FML_LogicalSide;

// Java: public boolean isServer()
int LIBMATTI_FML_LogicalSide_IsServer(LIBMATTI_FML_LogicalSide side);
// Java: public boolean isClient()
int LIBMATTI_FML_LogicalSide_IsClient(LIBMATTI_FML_LogicalSide side);

#endif //MATTICRAFT_FML_LOGICALSIDE_H
