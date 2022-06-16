#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

enum ssmed_type {
    SSEMD_GET       = 0x01,
    SSEMD_EDIT      = 0x02,
    SSEMD_RESPONSE  = 0xAA,
    SSEMD_ERROR     = 0xFF,
};

enum admin_state {
    read_status = 0,
    read_code,
    read_response_code,
    read_size1,
    read_size2,
    prepare_data,
    read_data,
    read_done,
    read_error_code,
    read_error,
    read_close,
};

typedef struct admin_parser{
    enum admin_state state;
    int size;
    char * data;
    int dataPointer;
    bool isList;
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

    SSEMD_ERROR_UNKNOWNTYPE     = 0xA0,
    SSEMD_ERROR_UNKNOWNCMD      = 0xA1,

    SSEMD_ERROR_NOSPACE         = 0xFA,
    SSEMD_ERROR_UNKNOWNERROR    = 0xFF,
};
enum ssmed_bool {
    SSEMD_TRUE  = 0x11,
    SSEMD_FALSE = 0x00,
};

#endif