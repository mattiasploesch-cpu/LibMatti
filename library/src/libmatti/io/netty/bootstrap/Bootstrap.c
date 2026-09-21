// Port of the io.netty surface: ByteBuf over the ByteBuffer port and a plain
// TCP channel for Bootstrap.connect.

#include "libmatti/io/netty/bootstrap/Bootstrap.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// ---------------------------------------------------------------------------
// Unpooled / ByteBuf
// ---------------------------------------------------------------------------

LIBMATTI_NT_ByteBuf *LIBMATTI_NT_Unpooled_Buffer(size_t initialCapacity)
{
    return LIBMATTI_JN_ByteBuffer_Allocate(initialCapacity > 0 ? initialCapacity : 256);
}

LIBMATTI_NT_ByteBuf *LIBMATTI_NT_Unpooled_WrappedBuffer(unsigned char *array, size_t length)
{
    return LIBMATTI_JN_ByteBuffer_Wrap(array, length);
}

int LIBMATTI_NT_ByteBuf_ReadableBytes(const LIBMATTI_NT_ByteBuf *buf)
{
    return (int) LIBMATTI_JN_ByteBuffer_Remaining(buf);
}

int LIBMATTI_NT_ByteBuf_ReadByte(LIBMATTI_NT_ByteBuf *buf)
{
    return LIBMATTI_JN_ByteBuffer_Get(buf);
}

int LIBMATTI_NT_ByteBuf_WriteByte(LIBMATTI_NT_ByteBuf *buf, int value)
{
    return LIBMATTI_JN_ByteBuffer_Put(buf, (unsigned char) value);
}

// Java: MC's VarInt (ByteBufManaged writeVarInt)
void LIBMATTI_NT_ByteBuf_WriteVarInt(LIBMATTI_NT_ByteBuf *buf, int value)
{
    while ((value & ~0x7F) != 0)
    {
        LIBMATTI_NT_ByteBuf_WriteByte(buf, (value & 0x7F) | 0x80);
        value >>= 7;
    }
    LIBMATTI_NT_ByteBuf_WriteByte(buf, value);
}

int LIBMATTI_NT_ByteBuf_ReadVarInt(LIBMATTI_NT_ByteBuf *buf, int *ok)
{
    int value = 0;
    int shift = 0;
    *ok = 0;

    while (1)
    {
        if (!LIBMATTI_JN_ByteBuffer_HasRemaining(buf) || shift >= 32)
            return 0;
        unsigned char byte = (unsigned char) LIBMATTI_JN_ByteBuffer_Get(buf);
        value |= (byte & 0x7F) << shift;
        if (!(byte & 0x80))
            break;
        shift += 7;
    }
    *ok = 1;
    return value;
}

// ---------------------------------------------------------------------------
// Bootstrap / Channel
// ---------------------------------------------------------------------------

LIBMATTI_NT_Channel *LIBMATTI_NT_Bootstrap_Connect(const char *host, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return NULL;

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons((uint16_t) port);
    // Java: InetAddress.getByName - the port accepts dotted quads (a resolver
    // is the host's job)
    if (inet_pton(AF_INET, host, &address.sin_addr) != 1)
    {
        close(fd);
        return NULL;
    }

    if (connect(fd, (struct sockaddr *) &address, sizeof(address)) != 0)
    {
        close(fd);
        return NULL;
    }

    int nodelay = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    LIBMATTI_NT_Channel *channel = calloc(1, sizeof(LIBMATTI_NT_Channel));
    channel->fd = fd;
    channel->active = 1;
    return channel;
}

void LIBMATTI_NT_Channel_Free(LIBMATTI_NT_Channel *channel)
{
    if (channel == NULL)
        return;
    LIBMATTI_NT_Channel_Close(channel);
    free(channel);
}

void LIBMATTI_NT_Channel_AddLast(LIBMATTI_NT_Channel *channel, LIBMATTI_NT_ChannelReadHandler readHandler,
                                 void *readSelf, LIBMATTI_NT_ChannelInactiveHandler inactiveHandler,
                                 void *inactiveSelf)
{
    channel->readHandler = readHandler;
    channel->readHandlerSelf = readSelf;
    channel->inactiveHandler = inactiveHandler;
    channel->inactiveHandlerSelf = inactiveSelf;
}

int LIBMATTI_NT_Channel_WriteAndFlush(LIBMATTI_NT_Channel *channel, LIBMATTI_NT_ByteBuf *buf)
{
    if (!channel->active)
        return 0;
    size_t length = LIBMATTI_JN_ByteBuffer_Position(buf);
    unsigned char *array = LIBMATTI_JN_ByteBuffer_Array(buf);
    // Java: writeAndFlush - one send
    ssize_t written = send(channel->fd, array, length, 0);
    return written == (ssize_t) length;
}

int LIBMATTI_NT_Channel_IsActive(const LIBMATTI_NT_Channel *channel)
{
    return channel->active;
}

void LIBMATTI_NT_Channel_Close(LIBMATTI_NT_Channel *channel)
{
    if (!channel->active)
        return;
    channel->active = 0;
    close(channel->fd);
    channel->fd = -1;
    if (channel->inactiveHandler != NULL)
        channel->inactiveHandler(channel->inactiveHandlerSelf, channel);
}

// Java: the event loop's read pass
void LIBMATTI_NT_Channel_FlushInbound(LIBMATTI_NT_Channel *channel)
{
    if (!channel->active || channel->readHandler == NULL)
        return;

    unsigned char buffer[8192];
    ssize_t received;
    while ((received = recv(channel->fd, buffer, sizeof(buffer), MSG_DONTWAIT)) > 0)
    {
        LIBMATTI_NT_ByteBuf *message = LIBMATTI_NT_Unpooled_WrappedBuffer(buffer, (size_t) received);
        channel->readHandler(channel->readHandlerSelf, channel, message);
        LIBMATTI_JN_ByteBuffer_Free(message);
        if (received < (ssize_t) sizeof(buffer))
            break;
    }
    if (received == 0)
        LIBMATTI_NT_Channel_Close(channel);
}
