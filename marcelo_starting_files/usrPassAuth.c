#include "include/usrPassAuth.h"

static struct user_pass user_pass_table[MAX_USER_PASS];
static int user_pass_c;

int create_table(char *filename, auth_level level)
{
    uint8_t line[514] = {0x00}, *uid_stored, *pw_stored, *uid, *pw;
    int ulen, plen;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return -1;
    }
    while (fgets((char *)line, sizeof(line), file) && user_pass_c < MAX_USER_PASS)
    {

        //Spliting the string to separate user from passwd, get the uid first
        uid_stored = (uint8_t *)strtok((char *)line, " ");
        pw_stored = (uint8_t *)strtok(NULL, "\n");

        ulen = strlen((const char *)uid_stored);
        plen = strlen((const char *)pw_stored);

        uid = calloc(1 + ulen, sizeof(uint8_t));
        if (uid == NULL)
            return -1;

        pw = calloc(1 + plen, sizeof(uint8_t));
        if (pw == NULL)
            return -1;

        memset(uid, 0, 1 + ulen);
        memset(pw, 0, 1 + plen);
        memcpy(uid, uid_stored, ulen);
        memcpy(pw, pw_stored, plen);
        uid[ulen] = 0x00;
        pw[plen] = 0x00;

        user_pass_table[user_pass_c].user = uid;
        user_pass_table[user_pass_c].password = pw;
        user_pass_table[user_pass_c].level = level;
        user_pass_c++;
    }
    fclose(file);

    return 0;
}

int create_usr_table(){
    return create_table(".")
}