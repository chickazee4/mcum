#ifndef IMGFLIP_H
#define IMGFLIP_H

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
#define RONALD_REAGAN_1 "413685043"
#define RONALD_REAGAN_2 "413690640"
#define FLAMING_SKULL "413686331"

extern char *imgflip_password;

typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

typedef struct imgflip_request {
    char *template_id;
    char *username;
    char *password;
    char *text0;
    char *text1;
} Imgflip_request;

typedef struct imgflip_response {
    int success; // 0 or 1
    char *error_message;
    char *page_url;
    char *url;
} Imgflip_response;

Imgflip_response request_meme(Imgflip_request *im);
Imgflip_request create_random_request(char *text0, char *text1);

#endif
