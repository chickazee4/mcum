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

int turns = -1;
uint64_t role = 0;
uint64_t power_role = 0;

uint64_t *blacklist = NULL;
int blacklist_count = 0;

User users[];

int mflag = 0,
    mfset = 0,
    aflag = 0,
    afset = 0;

char *
strip_quotes(char *orig)
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

char *
strip_url(char *orig)
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
void
strip_ws(char *orig, int len)
{
    for(int i = 0; i < len; i++){
        if(orig[i] == '\n' || orig[i] == '\t'){
            orig[i] = ' ';
        }
    }
}

void
split_text(char *orig, char **buf1, char **buf2)
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

char *
create_meme(char *text) {
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

void
on_message(struct discord *client, const struct discord_message *event) {
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
modal_error(char *msg, struct discord *client, struct discord_interaction *event)
{
    struct discord_interaction_response params = {
        .type = DISCORD_INTERACTION_MODAL,
        .data = &(struct discord_interaction_callback_data){
            .title = "Bozo",
            .content = msg
        }
    };
    discord_create_interaction_response(client, event->id, event->token, &params, NULL);
}

void
on_interaction(struct discord *client, const struct discord_interaction *event)
{
    if(event->type != DISCORD_INTERACTION_APPLICATION_COMMAND)
        return;
    if((strcmp(event->data->name, "boomerize") == 0) && 
            (role == 0 || includes_snowflake(event->member->roles->array, role, event->member->roles->size) != NULL) && 
            (includes_snowflake_ptr(blacklist, msgs.array[0].author->id, blacklist_count) == NULL)){
        struct discord_get_channel_messages msgparms = {
            .before = 0,
            .limit = 1
        };
        struct discord_messages msgs = { 0 };
        struct discord_ret_messages drm = {
            .sync = &msgs
        };

        discord_get_channel_messages(client, event->channel_id, &msgparms, &drm);
        if(msgs.array[0].author->bot){
            modal_error("Can't boomerize a bot post.", client, event);
        } else {
            char *url = create_meme(msgs.array[0].content);
            discord_messages_cleanup(&msgs);
            struct discord_interaction_response params = {
                    .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
                    .data = &(struct discord_interaction_callback_data){
                        .content = url
                    }
            };
            discord_create_interaction_response(client, event->id, event->token, &params, NULL);
        }
    }
    if((strcmp(event->data->name, "mcum-blacklist") == 0) && 
            (power_role == 0 || includes_snowflake(event->member->roles->array, power_role, event->member->roles->size) != NULL)){
        if (event->data->values->size == 1){
            uint64_t target_bl = 0;
            if((target_bl = atol(event->data->values->array[0])) != 0){
                append_snowflake(&blacklist, taget_bl, blacklist_count);
                blacklist_count++;
            }
        } else {
            modal_error("Wrong number of arguments.", client, event);
        }
    }
    if((strcmp(event->data->name, "mcum-whitelist") == 0) && 
            (power_role == 0 || includes_snowflake(event->member->roles->array, power_role, event->member->roles->size) != NULL)){
        if (event->data->values->size == 1){
            uint64_t target_wl = 0;
            if((target_wl = atol(event->data->values->array[0])) != 0){
                remove_snowflake(&blacklist, target_wl, &blacklist_count);
            }
        } else {
            modal_error("Wrong number of arguments.", client, event);
        }
    }
}

void
on_ready(struct discord *client, const struct discord_ready *event)
{
    log_info("Logged in as %s!", event->user->username);
    if(aflag == 0){
        struct discord_create_global_application_command boomerize = {
            .type = DISCORD_APPLICATION_CHAT_INPUT,
            .name = "boomerize",
            .description = "Boomerize the above post"
        };
        discord_create_global_application_command(client, event->application->id, &boomerize, NULL);

        struct discord_create_global_application_command blacklist = {
            .type = DISCORD_APPLICATION_CHAT_INPUT,
            .name = "mcum-blacklist",
            .description = "Blacklist this user from using mcum for this session"
        };
        discord_create_global_application_command(client, event->application->id, &blacklist, NULL);
        
        if(turns > -1) {
            struct discord_create_global_application_command giveturns = {
                .type = DISCORD_APPLICATION_CHAT_INPUT,
                .name = "mcum-give-turns",
                .description = "Give this user additional turns with mcum for the day (or take away, with a negative number)"
            };
            discord_create_global_application_command(client, event->application->id, &giveturns, NULL);
        }

        struct discord_create_global_application_command immunize = {
            .type = DISCORD_APPLICATION_CHAT_INPUT,
            .name = "mcum-immunize",
            .description = "Prevent this user from being boomerized"
        };
        discord_create_global_application_command(client, event->application->id, &immunize, NULL);

        struct discord_create_global_application_command whitelist = {
            .type = DISCORD_APPLICATION_CHAT_INPUT,
            .name = "mcum-whitelist",
            .description = "Remove this user from the mcum blacklist for this session"
        };
        discord_create_global_application_command(client, event->application->id, &whitelist, NULL);
    }
}

// yes, i am aware that concord offers an interface for this. no, i will not use it (too limited and we already have to link against json-c anyway so might as well make use of it). cope, seethe, mald, etc 
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
        if(mflag == 0 && target_id == 0){
            printf("And your target Discord user ID: ");
            scanf("%lu", &target_id);
        }
    } else if (bot_token == NULL || imgflip_username == NULL || bot_token == NULL || target_id == 0 || chance_denominator == UINT32_MAX){
        char read;
        char *line = NULL;
        char *contents = malloc(1);
        contents[0] = '\0';
        size_t len, curlen = 0;
        while ((read = getline(&line, &len, cfgfp)) != -1) {
            contents = realloc(contents, curlen + len);
            curlen += len;
            strcat(contents, line);
        }
        free(line);

        struct json_object *data = json_tokener_parse(contents);

        if(afset == 0 && mfset == 0){
            char *tmp;
            struct json_object *jobj_mode;
            if(json_object_object_get_ex(data, "mode", &jobj_mode)){
                tmp = json_object_to_json_string(jobj_mode);
                if(tmp == "manual"){
                    mflag = 1;
                } else if(tmp == "automatic"){
                    aflag = 1;
                } else {
                    perror("Invalid bot mode selected in config.json - check spelling. Continuing with both manual and automatic mode enabled.");
                }
            }
            afset = 1;
            mfset = 1;
        }
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
        if(mflag == 0 && target_id == 0){
            struct json_object *jobj_targetid;
            if (json_object_object_get_ex(data, "target_id", &jobj_targetid)){
                target_id = json_object_get_uint64(jobj_targetid);
            } else {
                printf("Target Discord user ID: ");
                scanf("%lu", &target_id);
            }
        }
        if(chance_denominator == UINT32_MAX){
            struct json_object *jobj_chance;
            if (json_object_object_get_ex(data, "chance", &jobj_chance)){
                chance_denominator = (unsigned int)json_object_get_int(jobj_chance);
            }
        }
        if(turns == -1){
            struct json_object *jobj_turns;
            if (json_object_object_get_ex(data, "turns", &jobj_turns)){
                chance_denominator = (unsigned int)json_object_get_int(jobj_turns);
            }
        }
        if(power_role == 0){
            struct json_object *jobj_powerrole;
            if (json_object_object_get_ex(data, "power_role", &jobj_powerrole)){
                power_role = json_object_get_uint64(jobj_powerrole);
            }
        }
        if(role == 0){
            struct json_object *jobj_role;
            if (json_object_object_get_ex(data, "role", &jobj_role)){
                role = json_object_get_uint64(jobj_role);
            }
        }
        struct json_object *json_blacklist;
        if (json_object_object_get_ex(data, "blacklist", &json_blacklist)){
            blacklist_count = json_object_array_length(json_blacklist);
            struct json_object *json_cur_bl;
            uint64_t cur_bl;
            for(int i = 0; i < blacklist_count; i++){
                json_cur_bl = json_object_array_get_idx(json_blacklist, i);
                cur_bl = json_object_get_uint64(json_cur_bl);
                append_snowflake(&blacklist, cur_bl);
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
                    case 'a':
                        aflag = 1;
                        afset = 1;
                        break;
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
                    case 'm':
                        mflag = 1;
                        mfset = 1;
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
                    case 'T':
                        if(i < argc - 1){
                            turns = (unsigned int)atoi(argv[i+1]);
                            i++;
                        }
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
    if(aflag && mflag) aflag = mflag = 0;
    load_cfg();
    if(chance_denominator == UINT32_MAX){
        chance_denominator = 1;
    }

    struct discord *client = discord_init(bot_token);
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_set_on_ready(client, &on_ready);
    if(mflag == 0)
        discord_set_on_message_create(client, &on_message);
    if(aflag == 0)
        discord_set_on_interaction_create(client, &on_interaction);
    discord_run(client);
}
