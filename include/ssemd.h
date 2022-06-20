#ifndef SSEMD_H_
#define SSEMD_H_

#include "buffer.h"
#include "hello.h"
#include "protocolParser.h"

typedef struct ssemd ssemd;

enum ssemd_state {
    SSEMD_READ_REQUEST = 0,
    SSEMD_WRITE_REQUEST,
    SSEMD_ERROR_STATE
};

enum ssmed_type {
    SSEMD_GET       = 0x01,
    SSEMD_EDIT      = 0x02,
    SSEMD_RESPONSE  = 0xAA,
    SSEMD_ERROR     = 0xFF,
};
enum ssmed_cmd_get {
    SSEMD_HISTORIC_CONNECTIONS  = 0x01,
    SSEMD_CURRENT_CONNECTIONS   = 0x02,
    SSEMD_BYTES_TRANSFERRED     = 0x03,
    SSEMD_USER_LIST             = 0x04,
    SSEMD_DISSECTOR_STATUS      = 0x05,
    SSEMD_AUTH_STATUS           = 0x06,
    SSEMD_GET_BUFFER_SIZE       = 0x07,
    SSEMD_GET_TIMEOUT           = 0x08,

};
enum ssmed_cmd_EDIT {
    SSEMD_BUFFER_SIZE       = 0x01,
    SSEMD_CLIENT_TIMEOUT    = 0x02,
    SSEMD_DISSECTOR_ON      = 0x03,
    SSEMD_DISSECTOR_OFF     = 0x04,
    SSEMD_ADD_USER          = 0x05,
    SSEMD_REMOVE_USER       = 0x06,
    SSEMD_AUTH_ON           = 0x07,
    SSEMD_AUTH_OFF          = 0x08,
};
enum ssmed_response_code {
    SSEMD_RESPONSE_OK       = 0x01,
    SSEMD_RESPONSE_LIST     = 0x02,
    SSEMD_RESPONSE_INT      = 0x03,
    SSEMD_RESPONSE_BOOL     = 0x04,
};
enum ssmed_error_code {
    SSEMD_ERROR_SMALLBUFFER     = 0x01,
    SSEMD_ERROR_BIGBUFFER       = 0x02,
    SSEMD_ERROR_SMALLTIMEOUT    = 0x03,
    SSEMD_ERROR_BIGTIMEOUT      = 0x04,
    SSEMD_ERROR_NOSPACEUSER     = 0x05,
    SSEMD_ERROR_REPEATEDUSER    = 0x06,
    SSEMD_ERROR_USERTOOBIG      = 0x07,
    SSEMD_ERROR_USERNOTFOUND    = 0x08,

    SSEMD_ERROR_UNKNOWNTYPE     = 0xA0,
    SSEMD_ERROR_UNKNOWNCMD      = 0xA1,
    SSEMD_ERROR_INCORRECT_TOKEN = 0xA2,
    SSEMD_ERROR_INCORRECTSIZE   = 0xA3,

    SSEMD_ERROR_NOSPACE         = 0xFA,
    SSEMD_ERROR_UNKNOWNERROR    = 0xFF,
};
enum ssmed_bool {
    SSEMD_TRUE  = 0x11,
    SSEMD_FALSE = 0x00,
};

struct ssemd_connection_state {
    int init;
    enum ssemd_state ssemd_state;
    void (*on_departure) (struct ssemd * currAdmin);
    void (*on_arrival) (struct ssemd * currAdmin);
};

typedef struct ssemd_response {
    uint8_t status;
    uint8_t code;
    uint8_t size1;
    uint8_t size2;
    uint8_t *data;

} ssemd_response;

struct admins_data
{
    struct ssemd * admins;
    int admins_size;
};

typedef struct ssemd
{
    int fd;

    char * adminAddr;

    char * admin_token;

    buffer * bufferRead;
    buffer * bufferWrite;

    protocol_parser * pr;
    ssemd_response * response;

    struct ssemd_connection_state connection_state;

    bool isAvailable;
} ssemd;

void
new_admin(struct ssemd * newAdmin, int adminSocket, int BUFFSIZE, char * adminAddr);

void removeAdmin(struct ssemd * admin);

#endif