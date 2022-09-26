#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>

#include "imgflip.h"
#include "mcum_configuration.h"

char *imgflip_password;
char *imgflip_ids[] = { KERMIT_TEA, CONDESCENDING_WONKA, GRUMPY_CAT, BIKER_GUY, NUCLEAR_FAMILY, JOHN_WAYNE_1, JOHN_WAYNE_2, MOTORCYCLE, YOUNG_SAM_ELLIOTT, OLD_SAM_ELLIOTT_1, OLD_SAM_ELLIOTT_2, PUNISHER, CHUNKY_FARMER, MINIONS_1, RONALD_REAGAN_1, RONALD_REAGAN_2, FLAMING_SKULL };

Imgflip_response structify(void *data)
{
    Imgflip_response ret = {};
    struct json_object *dataj = json_tokener_parse(data);
    struct json_object *success;
    json_object_object_get_ex(dataj, "success", &success);
    char *success_str = json_object_to_json_string(success);
    printf("%s\n", success_str);
    if (strcmp(success_str, "true") == 0)
        ret.success = 1;
    else
        ret.success = 0;
    if (success == 0) {
        struct json_object *error_msg;
        if (json_object_object_get_ex(dataj, "error_message", &error_msg)) {
            ret.error_message = json_object_to_json_string(error_msg);
        } else {
            perror("Fatal error in resolving JSON: image retrieve failed but no error message");
            exit(1);
        }
    } else {
        struct json_object *subdata;
        if (json_object_object_get_ex(dataj, "data", &subdata)){
            struct json_object *url;
            if (json_object_object_get_ex(subdata, "url", &url)){
                ret.url = json_object_to_json_string(url);
            } else {
                perror("Fatal error in resolving JSON: could not find URL token");
                exit(2);
            }
            struct json_object *page_url;
            if (json_object_object_get_ex(subdata, "page_url", &page_url)){
                ret.page_url = json_object_to_json_string(page_url);
            }
        } else {
            perror("Fatal error in resolving JSON: success response but failed to acquire data");
            exit(3);
        }
    }
    return ret;
}

size_t callback(char *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    get_request *req = (get_request *) data;

    printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + 2048);
        req->buflen += 2048;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    printf("%s", req->buffer);

    return realsize;
}

Imgflip_response request_meme(Imgflip_request *im)
{
    CURLcode ret;
    curl_mime *mime1 = NULL;
    curl_mimepart *part1;
    get_request req = {.buffer = NULL, .len = 0, .buflen = 0};

    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.imgflip.com/caption_image");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    mime1 = curl_mime_init(hnd);
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, im->username, CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "username");
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, im->password, CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "password");
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, im->text0, CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "text0");
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, im->text1, CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "text1");
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, im->template_id, CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "template_id");
    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, mime1);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.85.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_DEFAULT_PROTOCOL, "https");
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "length: 20");
    headers = curl_slist_append(headers, "numbers: true");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    req.buffer = malloc(2048);
    req.buflen = 2048;
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&req);

    ret = curl_easy_perform(hnd);

    Imgflip_response irp = structify(req.buffer);
    free(req.buffer);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_mime_free(mime1);
    mime1 = NULL;

    return irp;
}

Imgflip_request create_random_request(char *text0, char *text1)
{
    int rollmax = sizeof(imgflip_ids) / sizeof(char *);
    srand(time(0));
    int roll = rand() % rollmax;
    Imgflip_request irq = { .template_id=imgflip_ids[roll], .username=IMGFLIP_USERNAME, .password=imgflip_password, .text0=text0, .text1=text1 };
    return irq;
}
