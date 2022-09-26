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

// remove extraneous whitespace
void strip(char *orig, int len)
{
    for(int i = 0; i < len; i++){
        if(orig[i] == '\n' || orig[i] == '\t'){
            orig[i] = ' ';
        }
    }
}

void split_text(char *orig, char *buf1, char *buf2)
{
    int maxlen = strlen(orig);
    strip(orig, maxlen);
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

char *create_meme(char *text) {
    char *buf1;
    char *buf2;
    split_text(text, buf1, buf2);
    Imgflip_request im = create_random_request(buf1, buf2);
    Imgflip_response ir = request_meme(&im);
    if (ir.success == 0){
        perror(ir.error_message);
        exit(1);
    } else {
        return ir.url;
    }
}

void on_message(struct discord *client, const struct discord_message *event) {
    if (event->author->id == TARGET_USER_ID){
        srand(time(0));
        if ((rand() % CHANCE_DENOMINATOR) == 0) {
            if (event->attachments == NULL && event->embeds == NULL){
                char *url = create_meme(event->content);
                struct discord_create_message params = { .content = url };
                discord_create_message(client, event->channel_id, &params, NULL);
            }
        }
    }
}

int main(void) {
    printf("Please enter Imgflip password: \n");

    imgflip_password = malloc(256);
    scanf("%s", imgflip_password);

    struct discord *client = discord_init(BOT_TOKEN);
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_set_on_ready(client, &on_ready);
    discord_set_on_message_create(client, &on_message);
    discord_run(client);
}
