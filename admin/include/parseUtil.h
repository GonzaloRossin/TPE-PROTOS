#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

enum ssemd_type {
    SSEMD_GET       = 0x01,
    SSEMD_EDIT      = 0x02,
    SSEMD_RESPONSE  = 0xAA,
    SSEMD_ERROR     = 0xFF,
};

enum ssemd_cmd_get {
    SSEMD_HISTORIC_CONNECTIONS  = 0x01,
    SSEMD_CURRENT_CONNECTIONS   = 0x02,
    SSEMD_BYTES_TRANSFERRED     = 0x03,
    SSEMD_USER_LIST             = 0x04,
    SSEMD_DISSECTOR_STATUS      = 0x05,
    SSEMD_AUTH_STATUS           = 0x06,
    SSEMD_GET_BUFFER_SIZE       = 0x07,
    SSEMD_GET_TIMEOUT           = 0x08,

};
enum ssemd_cmd_EDIT {
    SSEMD_BUFFER_SIZE       = 0x01,
    SSEMD_CLIENT_TIMEOUT    = 0x02,
    SSEMD_DISSECTOR_ON      = 0x03,
    SSEMD_DISSECTOR_OFF     = 0x04,
    SSEMD_ADD_USER          = 0x05,
    SSEMD_REMOVE_USER       = 0x06,
    SSEMD_AUTH_ON           = 0x07,
    SSEMD_AUTH_OFF          = 0x08,
};

enum admin_state {
    read_status = 0,
    read_code,
    read_response_code,
    read_size1,
    read_size2,
    read_data,
    read_done,
    read_error_code,
    read_error,
    read_close,
};
enum ssemd_response_code {
    SSEMD_RESPONSE_OK       = 0x01,
    SSEMD_RESPONSE_LIST     = 0x02,
    SSEMD_RESPONSE_INT      = 0x03,
    SSEMD_RESPONSE_BOOL     = 0x04,
};
enum ssemd_error_code {
    SSEMD_ERROR_SMALLBUFFER     = 0x01,
    SSEMD_ERROR_BIGBUFFER       = 0x02,
    SSEMD_ERROR_SMALLTIMEOUT    = 0x03,
    SSEMD_ERROR_BIGTIMEOUT      = 0x04,
    SSEMD_ERROR_NOSPACEUSER     = 0x05,

    SSEMD_ERROR_UNKNOWNTYPE     = 0xA0,
    SSEMD_ERROR_UNKNOWNCMD      = 0xA1,
    SSEMD_ERROR_INCORRECT_TOKEN = 0xA2,

    SSEMD_ERROR_NOSPACE         = 0xFA,
    SSEMD_ERROR_UNKNOWNERROR    = 0xFF,
};
enum ssemd_bool {
    SSEMD_TRUE  = 0x11,
    SSEMD_FALSE = 0x00,
};

typedef struct admin_parser{
    enum ssemd_type type_sent;
    uint8_t cmd_sent;
    
    enum ssemd_type status;
    uint8_t response_code;
    uint8_t size1;
    uint8_t size2;
    int size; //helper for size
    uint8_t * data;

    enum admin_state state;
    int dataPointer;
    unsigned int number;
};


extern void admin_parser_init(struct admin_parser * adminParser);

extern enum admin_state admin_parser_feed(struct admin_parser * parser, const uint8_t byte);

extern bool admin_is_done(const enum admin_state state, bool * errored);
extern const char * admin_error_handler(const struct admin_parser * parser);

extern void admin_parser_close(struct admin_parser * parser);
extern enum admin_state admin_consume(buffer * buffer, struct admin_parser * parser, bool *errored);


void printInt(struct admin_parser * adminParser);
void printList(struct admin_parser * adminParser);
void printBool(struct admin_parser * adminParser);


#endif