#include "socks5.h"

static void on_hello_method(struct hello_parser * parser, const uint8_t method){
    uint8_t * selected = parser->data;

    if(SOCKS_HELLO_NO_AUTHENTICATION_REQUIRED == method){ //acá elijo la funcion no auth required, nosotros deberíamos poner la de password
        *selected = method;
    }
}

//lee e interpreta la trama "hello" que llega por fd
//return true ante un error
static bool read_hello(const int fd, const uint8_t * method) {
    //0. lectura del primer hello
    uint8_t buff[256+1+1];
    buffer buffer;
    buffer_init(&buffer, N(buff), buff);
    struct hello_parser hello_parser = {
        .data = (void *)method,
        .on_authentication_method = on_hello_method,
    };
    hello_parser_init(&hello_parser);
    bool error = false;
    size_t buffsize;
    ssize_t n;
    do {
        uint8_t * ptr = buffer_write_ptr(&buffer, &buffsize);
        n = recv(fd, ptr, buffsize, 0);
        if(n > 0){
            buffer_write_adv(&buffer, n);
            const enum hello_state st = hello_consume(&buffer, hello_parser, &error); //o es *error?
            if(hello_is_done(st, &error)) {
                break;
            }
        } else {
            break;
        }
    } while(true);

    if( ! hello_is_done(hello_parser.state, &error)){
        error = true;
    }
    hello_parser_close(&hello_parser);
    return error;
}

//static bool read_request(const int fd, struct request *request)

void //dudo que sea void
socks5_handle_connection(const int fd, const struct sockaddr *caddr){
    uint8_t method = SOCKS_HELLO_NO_ACCEPTABLE_METHODS;
    struct request request;
    struct sockaddr *originaddr = 0x00;
    socklen_t origin_addr_len = 0;
    int origin_domain;
    int originfd = -1;
    uint8_t buff[10];
    buffer b;
    buffer_init(&(b), N(buff), (buff));

    //0. lectura del hello enviado por el cliente
    if(read_hello(fd, &method)) {
        goto finally;
    }

    //1. envío de la respuesta
    const uint8_t r = (method == SOCKS_HELLO_NO_ACCEPTABLE_METHODS) ? 0xFF : 0x00;
    hello_marshall(&b, r);
    if(sock_blocking_write(fd, &b)) { //de netutils.h .c
        goto finally;
    }
    if(SOCKS_HELLO_NO_ACCEPTABLE_METHODS == method) {
        goto finalle;
    }

    //2. lectura del request
    enum socks_response_status status = status_general_SOCKS_server_failure; // talvez no sea failure
    if(read_request(fd, &erquest)) {
        status = status_general_SOCKS_server_failure;
    } else {
        //3. procesamiento
        switch (request.cmd)
            {
            case socks_req_cmd_connect: {
                bool error = false;
                status = cmd_resolve(&request, &originaddr, &origin_addr_len, &origin_domain);
                if(originaddr == NULL){
                    error = true;
                } else {
                    originfd = socket(origin_domain, SOCK_STREAM, 0);
                    if(originfd == -1){
                        error = true;
                    } else {
                        if(connect(originfd, originaddr, origin_domain) == -1){//hay más a la derecha que no se ve
                            status = errno_to_socks(errno);
                            errir = true;
                        } else {
                            status = status_succeeded;
                        }
                    }
                }
                if(error) {
                    if(originfd != -1) {
                        close(originfd);
                        originfd = -1;
                    }
                    close(fd);
                }
                break;
            }case socks_req_cmd_bind:
            case socks_req_cmd_associate:
            default:
            status = status_command_not_supported;
                break;
            }
    }
    log_request(status, caddr, originaddr);

    //4. envío de respuesta al request
    request_marshall(&b, status);
}
                //acá sigue pero ya no se ve, creo que mas adelante se ve mas pero bueno, cerca del final del recording