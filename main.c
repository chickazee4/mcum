#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <concord/discord.h>
#include <concord/log.h>

#include "mcum_configuration.h"
#include "imgflip.h"

void on_ready(struct discord *client, const struct discord_ready *event) {
    log_info("Logged in as %s!", event->user->username);
}

char *strip_url(char *orig)
{
    int len = strlen(orig);
    char *new = malloc(strlen(orig) * sizeof(char));
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
void strip(char *orig, int len)
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
    strip(orig, maxlen);
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
        int len1 = strlen(*buf1);
        int len2 = strlen(*buf2);
        free(out2);
    }
    /*
    int *divider_indices = malloc(sizeof(int));
    int indexct = 0;

    for(int i = 0; i++; i < maxlen){
        switch (orig[i]){
            case ',':
            case ':':
            case ';':
            case '.':
            case '?':
            case '!':
                if(i < maxlen - 1) {
                    indexct++;
                    divider_indices = realloc(divider_indices, sizeof(int) * indexct);
                    divider_indices[indexct - 1] = i;
                }
                break;
            default:
                break;
        }
    }
    if (indexct <= 1) {
        buf1 = orig;
        buf2 = "";
    } else {
        int middle = maxlen / 2;
        int lowest = 0, distance;
        // find the index closest to the middle of the post
        for(int j = 0; j++; j < indexct){
            distance = abs(middle - divider_indices[j]);
            if(distance < lowest)
                lowest = divider_indices[j];
        }

        buf1 = malloc(lowest + 1);
        buf2 = malloc(maxlen - lowest - 2);
        strncpy(buf1, orig, lowest + 1);
        strncpy(buf2, orig + lowest + 2, maxlen - lowest - 2);
    }
    */
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
        printf("%s\n", stripped_url);
        return stripped_url;
    }
}

void on_message(struct discord *client, const struct discord_message *event) {
    printf("Message read with contents %s from author id %lu\n", event->content, event->author->id);
    if (event->author->id == TARGET_USER_ID){
        srand(time(0));
        if ((rand() % CHANCE_DENOMINATOR) == 0) {
                char *url = create_meme(event->content);
                struct discord_create_message params = { .content = url };
                discord_create_message(client, event->channel_id, &params, NULL);
        }
    }
}

int main(void) {
    printf("Please enter Imgflip password: ");

    imgflip_password = malloc(256);
    scanf("%s", imgflip_password);

    struct discord *client = discord_init(BOT_TOKEN);
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_set_on_ready(client, &on_ready);
    discord_set_on_message_create(client, &on_message);
    discord_run(client);
}
