#ifndef IMGFLIP_H
#define IMGFLIP_H
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define KERMIT_TEA "16464531"
#define CONDESCENDING_WONKA "61582"
#define GRUMPY_CAT "405658"
#define BIKER_GUY "413682496"
#define NUCLEAR_FAMILY "413682600"
#define JOHN_WAYNE_1 "413683279"
#define JOHN_WAYNE_2 "413684645"
#define MOTORCYCLE "413683520"
#define YOUNG_SAM_ELLIOTT "413683826"
#define OLD_SAM_ELLIOTT_1 "413683972"
#define OLD_SAM_ELLIOTT_2 "413685815"
#define PUNISHER "413684154"
#define CHUNKY_FARMER "413684277"
#define MINIONS_1 "413684447"
#define MINIONS_2 "414928448"
#define RONALD_REAGAN_1 "413685043"
#define RONALD_REAGAN_2 "413690640"
#define FLAMING_SKULL "413686331"
#define KNITTING "415073804"
#define CLINT_EASTWOOD_1 "415074557"
#define CLINT_EASTWOOD_2 "415074830"
#define CLINT_EASTWOOD_3 "415075058"
#define ELVIS "415075882"

extern char *config_file;
extern char *imgflip_password;
extern char *imgflip_username;
extern unsigned int chance_denominator;
extern char *bot_token;
extern uint64_t target_id;

typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

typedef struct {
    char *template_id;
    char *username;
    char *password;
    char *text0;
    char *text1;
} Imgflip_request;

typedef struct {
    int success; // 0 or 1
    char *error_message;
    char *page_url;
    char *url;
} Imgflip_response;

typedef struct {
    int id;
    int turns_taken;
    int turns_allowed;
    time_t first_turn;
    int immune;
} User;

Imgflip_response request_meme(Imgflip_request *);
Imgflip_request create_random_request(char *, char *);

void get_key(char **, char *);

uint64_t *  includes_snowflake       (  uint64_t[],   uint64_t,  int  );
uint64_t *  includes_snowflake_ptr   (  uint64_t **,  uint64_t,  int  );
uint64_t ** append_snowflake         (  uint64_t **,  uint64_t,  int  );
uint64_t ** remove_snowflake         (  uint64_t **,  uint64_t,  *int );

#endif
