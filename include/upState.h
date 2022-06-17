#ifndef UPSTATE_H_
#define UPSTATE_H_

#include "socks5.h"
#include "selector.h"
#include "adminFunctions.h"
#include "socks5Handler.h"

void up_read_close(struct socks5 * currClient);

void up_read_init(struct socks5 * currClient);
void userpass_process(struct userpass_st *up_s, bool * auth_valid);

void up_read(struct selector_key *key);
void up_write(struct selector_key *key);
bool validate_user_proxy(uint8_t *uid, uint8_t *pw);

#endif