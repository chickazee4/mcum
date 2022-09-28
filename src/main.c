#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <concord/discord.h>
#include <concord/log.h>
#include <json-c/json.h>

#include "config.h"
#include "mcum.h"

char *config_file = CONFIG_FILE;
char *bot_token = NULL;
uint64_t target_id = 0;
char *imgflip_username = NULL;
char *imgflip_password = NULL;
unsigned int chance_denominator = UINT32_MAX;

void on_ready(struct discord *client, const struct discord_ready *event) {
    log_info("Logged in as %s!", event->user->username);
}

char *strip_quotes(char *orig)
{
    int len = strlen(orig);
    char *new = malloc(len * sizeof(char));
    int j = 0;
    for(int i = 0; i < len; i++){
        if (orig[i] != '\"' && orig[i] != '\\'){
            new[j] = orig[i];
            j++;
        }
    }
    new[j] = '\0';
    return new;
}

char *strip_url(char *orig)
{
    int len = strlen(orig);
    char *new = malloc(len * sizeof(char));
    int j = 0;
    for(int i = 0; i < len; i++){
        if (orig[i] != '\\' && orig[i] != '\"'){
            new[j] = orig[i];
            j++;
        }
    }
    new[j] = '\0';
    return new;
}

// remove extraneous whitespace
void strip_ws(char *orig, int len)
{
    for(int i = 0; i < len; i++){
        if(orig[i] == '\n' || orig[i] == '\t'){
            orig[i] = ' ';
        }
    }
}

void split_text(char *orig, char **buf1, char **buf2)
{
    int maxlen = strlen(orig);
    strip_ws(orig, maxlen);
    char *token;
    char *out = strdup(orig);
    int wc = 0;
    // pass 1
    while ((token = strsep(&out, " "))){
        wc++;
    }
    free(out);
    if (wc <= 1){
        *buf1 = orig;
        *buf2 = " ";
    } else {
        *buf1 = malloc(maxlen);
        *buf1[0] = '\0';
        *buf2 = malloc(maxlen);
        *buf2[0]= '\0';
        char *out2 = strdup(orig);
        int i = 0;
        // pass 2
        while ((token = strsep(&out2, " "))){
            if (i < wc / 2){
                strcat(*buf1, token);
                strcat(*buf1, " ");
            } else {
                strcat(*buf2, token);
                strcat(*buf2, " ");
            }
            i++;
        }
        free(out2);
    }
}

char *create_meme(char *text) {
    char *buf1;
    char *buf2;
    split_text(text, &buf1, &buf2);
    Imgflip_request im = create_random_request(buf1, buf2);
    Imgflip_response ir = request_meme(&im);
    if (ir.success == 0){
        perror(ir.error_message);
        exit(1);
    } else {
        char *stripped_url = strip_url(ir.url);
        log_info("Generated meme with URL %s", stripped_url);
        return stripped_url;
    }
}

void on_message(struct discord *client, const struct discord_message *event) {
    if (event->author->id == target_id){
        srand(time(0));
        if ((rand() % chance_denominator) == 0) {
                char *url = create_meme(event->content);
                struct discord_create_message params = { .content = url };
                discord_create_message(client, event->channel_id, &params, NULL);
        } else {
            log_info("Received post by target user, but dice roll unsuccessful.");
        }
    }
}

void
load_cfg()
{
    FILE *cfgfp;
    cfgfp = fopen(config_file, "r");
    if (!cfgfp && (bot_token == NULL || imgflip_username == NULL || bot_token == NULL || target_id == 0)){
        printf("No config file found; you will need to enter your credentials manually.\n");
        if(bot_token == NULL) get_key(&bot_token, "Discord bot token");
        if(imgflip_username == NULL) get_key(&imgflip_username, "Imgflip username");
        if(imgflip_password == NULL) get_key(&imgflip_password, "Imgflip password");
        if(target_id == 0){
            printf("And your target Discord user ID: ");
            scanf("%lu", &target_id);
        }
    } else if (bot_token == NULL || imgflip_username == NULL || bot_token == NULL || target_id == 0 || chance_denominator == UINT32_MAX){
        char read;
        char *line;
        char *contents = malloc(1);
        contents[0] = '\0';
        size_t len, curlen = 0;
        while ((read = getline(&line , &len, cfgfp)) != -1) {
            contents = realloc(contents, curlen + len);
            curlen += len;
            strcat(contents, line);
        }
        free(line);

        struct json_object *data = json_tokener_parse(contents);

        if(bot_token == NULL){
            char *tmp;
            struct json_object *jobj_token;
            if (json_object_object_get_ex(data, "bot_token", &jobj_token)){
                tmp = json_object_to_json_string(jobj_token);
                bot_token = strip_quotes(tmp);
                free(tmp);
            } else {
                get_key(&bot_token, "Discord bot token");
            }
        }
        if(imgflip_username == NULL){
            char *tmp;
            struct json_object *jobj_username;
            if (json_object_object_get_ex(data, "imgflip_username", &jobj_username)){
                tmp = json_object_to_json_string(jobj_username);
                imgflip_username = strip_quotes(tmp);
                free(tmp);
            } else {
                get_key(&imgflip_username, "Imgflip username");
            }
        }
        if(imgflip_password == NULL){
            char *tmp;
            struct json_object *jobj_password;
            if (json_object_object_get_ex(data, "imgflip_password", &jobj_password)){
                tmp = json_object_to_json_string(jobj_password);
                imgflip_password = strip_quotes(tmp);
                free(tmp);
            } else {
                get_key(&imgflip_password, "Imgflip password");
            }
        }
        if(target_id == 0){
            struct json_object *jobj_targetid;
            char *tmp1, *tmp2;
            if (json_object_object_get_ex(data, "target_id", &jobj_targetid)){
                tmp1 = json_object_to_json_string(jobj_targetid);
                tmp2 = strip_quotes(tmp1);
                free(tmp1);
                if((target_id = (uint64_t)atol(tmp2)) <= 0){
                    printf("Target Discord user ID: ");
                    scanf("%lu", &target_id);
                }
            } else {
                printf("Target Discord user ID: ");
                scanf("%lu", &target_id);
            }
        }
        if(chance_denominator == UINT32_MAX){
            char *tmp1, *tmp2;
            unsigned int i;
            struct json_object *jobj_chance;
            if (json_object_object_get_ex(data, "chance", &jobj_chance)){
                tmp1 = json_object_to_json_string(jobj_chance);
                tmp2 = strip_quotes(tmp1);
                free(tmp1);
                if((i = (unsigned int)atoi(tmp2)) > 0)
                    chance_denominator = i;
                free(tmp2);
            }
        }
    }
}

int
main(int argc, char **argv) {
    if(argc > 0){
        for(int i = 0; i < argc; i++){
            if(argv[i][0] == '-'){
                switch(argv[i][1]){
                    case 'c':
                        if(i < argc - 1){
                            i++;
                            config_file = argv[i];
                        }
                        break;
                    case 'd':
                        if(i < argc - 1){
                            i++;
                            chance_denominator = ((unsigned int)atoi(argv[i]));
                        }
                        break;
                    case 'i':
                        if(i < argc - 1){
                            i++;
                            if(json_parse_uint64(argv[i], &target_id) != 0){
                                perror("Invalid user ID");
                                exit(4);
                            }
                        }
                        break;
                    case 'p':
                        if(i < argc - 1){
                            i++;
                            imgflip_password = argv[i];
                        }
                        break;
                    case 't':
                        if(i < argc - 1){
                            i++;
                            bot_token = argv[i];
                        }
                        break;
                    case 'u':
                        if(i < argc - 1){
                            i++;
                            imgflip_username = argv[i];
                        }
                        break;
                    default:
                        printf("Invalid argument %s, ignoring.\n", argv[i]);
                        break;
                }
            }
        }
    }
    load_cfg();
    if(chance_denominator == UINT32_MAX){
        chance_denominator = 1;
    }

    struct discord *client = discord_init(bot_token);
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_set_on_ready(client, &on_ready);
    discord_set_on_message_create(client, &on_message);
    discord_run(client);
}
