// Port of the io.netty surface the game uses: ByteBuf, EventLoopGroup stubs,
// Bootstrap/Channel for the client's network stack.
// MC's packets ride on top; the port wraps plain sockets.

#ifndef MATTICRAFT_NETTY_BOOTSTRAP_H
#define MATTICRAFT_NETTY_BOOTSTRAP_H

#include "libmatti/java/nio/ByteBuffer.h"

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Java: io.netty.buffer.ByteBuf - the port reuses java.nio.ByteBuffer semantics
// ---------------------------------------------------------------------------

typedef LIBMATTI_JN_ByteBuffer LIBMATTI_NT_ByteBuf;

// Java: Unpooled.buffer() / Unpooled.wrappedBuffer(bytes)
LIBMATTI_NT_ByteBuf *LIBMATTI_NT_Unpooled_Buffer(size_t initialCapacity);
LIBMATTI_NT_ByteBuf *LIBMATTI_NT_Unpooled_WrappedBuffer(unsigned char *array, size_t length);
// Java: public int readableBytes() / readByte() / writeByte(int)
int LIBMATTI_NT_ByteBuf_ReadableBytes(const LIBMATTI_NT_ByteBuf *buf);
int LIBMATTI_NT_ByteBuf_ReadByte(LIBMATTI_NT_ByteBuf *buf);
int LIBMATTI_NT_ByteBuf_WriteByte(LIBMATTI_NT_ByteBuf *buf, int value);
// Java: ByteBuf.writeVarInt / readVarInt - MC's VarInt on the netty pipeline
void LIBMATTI_NT_ByteBuf_WriteVarInt(LIBMATTI_NT_ByteBuf *buf, int value);
int LIBMATTI_NT_ByteBuf_ReadVarInt(LIBMATTI_NT_ByteBuf *buf, int *ok);

// ---------------------------------------------------------------------------
// Java: io.netty.bootstrap.Bootstrap + Channel - a single TCP client connection
// ---------------------------------------------------------------------------

typedef struct LIBMATTI_NT_Channel LIBMATTI_NT_Channel;

// Java: interface ChannelInboundHandler - channelRead(ChannelHandlerContext, Object)
typedef void (*LIBMATTI_NT_ChannelReadHandler)(void *self, LIBMATTI_NT_Channel *channel,
                                               LIBMATTI_NT_ByteBuf *message);
typedef void (*LIBMATTI_NT_ChannelInactiveHandler)(void *self, LIBMATTI_NT_Channel *channel);

typedef struct LIBMATTI_NT_Channel
{
    int fd; // the socket, -1 when closed
    LIBMATTI_NT_ChannelReadHandler readHandler;
    void *readHandlerSelf;
    LIBMATTI_NT_ChannelInactiveHandler inactiveHandler;
    void *inactiveHandlerSelf;
    int active;
} LIBMATTI_NT_Channel;

// Java: Bootstrap.connect(host, port) -> ChannelFuture; the port connects sync
// and returns the channel (NULL on failure)
LIBMATTI_NT_Channel *LIBMATTI_NT_Bootstrap_Connect(const char *host, int port);
void LIBMATTI_NT_Channel_Free(LIBMATTI_NT_Channel *channel);

// Java: channel.pipeline().addLast(handler)
void LIBMATTI_NT_Channel_AddLast(LIBMATTI_NT_Channel *channel, LIBMATTI_NT_ChannelReadHandler readHandler,
                                 void *readSelf, LIBMATTI_NT_ChannelInactiveHandler inactiveHandler, void *inactiveSelf);
// Java: channel.writeAndFlush(ByteBuf)
int LIBMATTI_NT_Channel_WriteAndFlush(LIBMATTI_NT_Channel *channel, LIBMATTI_NT_ByteBuf *buf);
// Java: channel.isActive()
int LIBMATTI_NT_Channel_IsActive(const LIBMATTI_NT_Channel *channel);
// Java: channel.close()
void LIBMATTI_NT_Channel_Close(LIBMATTI_NT_Channel *channel);
// Java: the event loop's read pass - drains the socket into the handler
void LIBMATTI_NT_Channel_FlushInbound(LIBMATTI_NT_Channel *channel);

#endif //MATTICRAFT_NETTY_BOOTSTRAP_H
