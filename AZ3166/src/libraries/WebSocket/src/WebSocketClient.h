// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "mbed.h"
#include "SystemWiFi.h"
#include "http_common.h"
#include "http_parsed_url.h"
#include "nsapi_types.h"

#define _WS_DEBUG

#ifdef _WS_DEBUG
#define INFO_FORMAT(format, args...) do {char buf[250];sprintf(buf, format, args);printf(buf);} while(0);
#define ERROR_FORMAT(format, args...) do {char buf[250];sprintf(buf, format, args);printf(buf);} while(0);

#define INFO(x) do {  } while(0);
#define ERROR(x) do {  } while(0);
#else
#define INFO_FORMAT(format, args...) do {} while(0);
#define ERROR_FORMAT(format, args...) do {} while(0);

#define INFO(x) do {  } while(0);
#define ERROR(x) do {  } while(0);
#endif

// Amount of time (in ms) a user may be connected before getting disconnected
// for timing out (i.e. not sending any data to the server).
#define TIMEOUT_IN_MS 10000

typedef enum
{
    WS_Message_Text = 0,        /* The message is clear text. */
    WS_Message_Binary           /* The message is in binary format. */
} WS_Message_Type;

typedef enum
{
    WS_OPCODE_CONT = 0x00,          /* Denotes a continuation frame */
    WS_OPCODE_TEXT = 0x01,          /* Denotes a text frame */
    WS_OPCODE_BINARY = 0x02,        /* Denotes a binary frame */
    WS_OPCODE_CLOSE = 0x08,         /* Denotes a connection close */
    WS_OPCODE_PING = 0x09,          /* Denotes a ping */
    WS_OPCODE_PONG = 0x0A,          /* Denotes a pong */
    WS_FINAL_BIT = 0x80             /* Denotes a final message frame */
};

typedef enum
{
    WS_NORMAL = 1000,               /* Normal closure */
    WS_GOING_AWAY = 1001,           /* An endpoint is "going away" */
    WS_PROTOCOL_ERROR = 1002,       /* A protocol error occurred. */
    WS_UNSOPPORTED_DATA = 1003,     /* An endpoint has received a type of data it cannot accept. */
    WS_NO_STATUS = 1005,            /* A dummy value to indicate that no status code was received. */
    WS_ABNORMAL_CLOSE = 1006,        /* A dummy value to indicate that the connection was closed abnormally. */
    WS_INVALID_PAYLOAD = 1007,      /* An endpoint received message data inconsistent with its type. */
    WS_POLICY_VIOLATION = 1008,     /* An endpoint received a message that violated its policy. */
    WS_MESSAGE_TOO_BIG = 1009       /* An endpoint received a message too large to process. */
} WSCloseStatusCode;

typedef struct
{
    int length;
    bool isEndOfMessage;
    WS_Message_Type messageType;
} WebSocketReceiveResult;

class WebSocketClient
{
    public:
        /**
        * Constructor
        *
        * @param url The Websocket url in the form "ws://ip_domain[:port]/path" (by default: port = 80)
        */
        WebSocketClient(char * url);

        /**
        * Destructor
        * 
        */
        ~WebSocketClient();

        /**
        * Connect to the websocket url
        *
        *@return true if the connection is established, false otherwise
        */
        bool connect();

        /**
        * Check if the WebSocket connection is available
        *
        * @return true if the connection is available
        */
        bool connected();

        /**
        * Send a string according to the websocket format (see rfc 6455)
        *
        * @param data           message data to be sent.
        * @param size           length of message payload in bytes.
        * @param messageType    data message type, can be WS_Message_Text or WS_Message_Binary
        * @param isFinal        Flag indicates whether this is a final data frame.
                                By default it is true. For a message with a big payload you may need to send 
                                it with smaller pieces and mark the final piece with this flag.
        *
        * @returns the number of bytes sent, or negative number on error
        */
        int send(const char * data, long size, WS_Message_Type messageType = WS_Message_Text, bool isFinal = true);

        /**
        * Send a ping message according to the websocket format (see rfc 6455)
        *
        * @param application data in ping frame
        *
        * @returns the number of bytes sent, or negative number on error
        */
        int sendPing(char * str, int size);

        /**
        * Read a websocket message
        *
        * @param msgBuffer  pointer to the string to be read (null if drop frame)
        * @param size       Size of the buffer in bytes
        *
        * @return A WebSocketReceiveResult object contains the information of the received message.
        */
        WebSocketReceiveResult* receive(char * msgBuffer, int size);

        /**
        * Close the websocket connection
        *
        * @return true if the connection has been closed, false otherwise
        */
        bool close();

        /*
        * Accessor: get path from the websocket url
        *
        * @return path
        */
        const char* getPath();

    private:
        bool doHandshake();
        int sendLength(long len, char * msg);
        int sendMask(char * msg);
        int readChar(char * pC, bool block = true);

        int read(char * buf, int len, int min_len = -1);
        int write(const char * buf, int len);

        TCPSocket * _tcpSocket;
        ParsedUrl * _parsedUrl;
        uint16_t _port;
        WS_Message_Type _messageType;
        bool _firstFrame;
};

#endif
