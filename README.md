# What is mcum?

mcum is a Discord bot application that will automatically turn a user's posts into the type of meme that you would see on a Facebook page frequented by baby boomers. You'll need to create the Discord API application yourself (I don't share my instance), but mcum will provide the actual function for it - just get your bot token and configure your instance with it.

# Configuration

You do not need to create a config.json in order to use the program (see Usage for how you can enter your configuration otherwise), but it is recommended. The following fields are supported:

* blacklist - an *array* consisting of user IDs who will not be allowed to use mcum. This can be overridden while mcum is active using the Discord chat command /whitelist.
* bot_token - your Discord API bot token (not recommended to store this here because it is plaintext, but it's an option if you don't anticipate remote attacks)
* chance - the denominator of the likelihood (1/x) that mcum will create a meme out of a target user's post
* imgflip_username - username for the Imgflip account to use
* imgflip_password - password for the Imgflip account (also not recommended to store in this file for the same reason as the token)
* immunized - an *array* consisting of user IDs whose posts cannot be boomerized. This automatically includes bots.
* mode - the mode for mcum to operate in by default (must be either `"manual"` or `"automatic"`; if you want to use both, which is the default behavior, do not add a field to the config.json). 
* power_role - numeric ID of role needed to blacklist, whitelist, immunize, or give additional turns to a user 
* role - numeric ID of role needed to boomerize posts (if not set, available to any user with command permission)
* target_id - the Discord numeric user ID for the target user in automatic mode. Ignored in manual mode.
* turns - the number of times a user can manually invoke mcum per 24-hour period (period defined per user, so every user must wait 24 hours from the first time they invoke mcum within a cycle once using up all turns). Ignored if automatic-only mode is on. If omitted or set to -1, users have infinite turns by default.

A minimal config.json, necessary to make mcum run without using any flags or entering any information from the command line, looks like this following:

    {
        "bot_token": "YOUR_BOT_TOKEN_HERE",
        "imgflip_username": "YOUR_IMGFLIP_USERNAME",
        "imgflip_password": "YOUR_IMGFLIP_PASSWORD",
        "target_id": "YOUR_TARGET_ID"
    }

Alternatively, if you don't use autotargeting:

    {
        "bot_token": "YOUR_BOT_TOKEN_HERE",
        "imgflip_username": "YOUR_IMGFLIP_USERNAME",
        "imgflip_password": "YOUR_IMGFLIP_PASSWORD",
        "mode": "manual"
    }    

# Usage

The most basic usage is:

    mcum

This is fine if you have a config.json in the location that you set as the default during compilation & are content with the settings therein or if you're okay with entering the program configuration interactively once it starts. You can also use the flags -cdiptu to pass arguments to the program if your configuration file exists but is in a non-default location or if you do not have one but do not want to use the interactive entry mode.

    -c [FILE]   Path to your config.json
    -d [INT]    The denominator of the likelihood (1/x) that mcum will create a meme out of the given user's post
    -i [ID]     The target user's Discord ID
    -p [PASSWD] The password for the Imgflip account to be used (not recommended to pass this as an argument due to console logging, but it's an option)
    -t [TOKEN]  The token for the Discord bot to operate (also not recommended to pass as an argument)
    -T [TURNS]  Default number of turns that a user gets in manual mode
    -u [USERNM] The Imgflip username to use

Arguments passed will override those in config.json, if they exist.

The default for mcum is to automatically boomerize a specified target user's posts at your specified frequency as well as creating a chat command that can be used to manually boomerize any post. You can change which of these is done with command line flags:

    -a          Automatic operation only: no chat command will be created
    -m          Manual operation only: no automatic targeting, users must use the command /boomerize and the last message in that channel will be converted

These also override the setting chosen in `config.json`. If you specify the '-m' flag and/or set the bot mode to `manual` in config.json (without overriding it with an -a), you will not be prompted for a target user ID.

## Discord commands

Commands are only created if running in default (combined) mode or manual mode. All permission-related commands only last for that session of mcum; persistent changes should be made to `config.json`.

* /boomerize - boomerize the post immediately preceding command. Does not take arguments.
* /mcum-blacklist user_id - temporarily, while mcum is running, blacklist a user (they will not be able to invoke mcum). To semi-permanently blacklist a user, use the config.json. config.json blacklists can still be overridden using /whitelist. Must use user ID, not name.
* /mcum-give-turns user_id how_many - give additional turns to a user (must use user ID).
* /mcum-immunize user_id - prevent a user from being boomerized
* /mcum-whitelist user_id - temporarily, while mcum is running, whitelist a user who is on the blacklist. Must use user ID, not name.

# Comparison of mcum configuration actions

Inheritance pattern: Discord commands override command line flags override config.json entries. Only config.json entries are persistent across sessions. The path to the config.json can also be hardcoded at compile time, which is, of course, also persistent. It is the only variable that supports this.

If the table says a variable is relevant in "automatic" or "manual" mode, that also includes default (both) mode.

| Action/variable          | config.json          | Command line flag | Discord command     | Mandatory?      | Type                                     | Modes     | Default value                        |
---------------------------|----------------------|-------------------|---------------------|-----------------|------------------------------------------|-----------|--------------------------------------|
| Discord bot token        | bot_token            | -t [...]          | -                   | Yes             | string                                   | All       | [empty]                              |
| Imgflip username         | imgflip_username     | -u [...]          | -                   | Yes             | string                                   | All       | [empty]                              |
| Imgflip password         | imgflip_password     | -p [...]          | -                   | Yes             | string                                   | All       | [empty]                              |
| Path to config.json      | -                    | -c [...]          | -                   | No              | string                                   | All       | $INSTALL_PREFIX/etc/mcum/config.json |
| Run mode                 | mode                 | -a, -m            | -                   | No              | enum ("manual"/"automatic")              | (affects) | [both]                               |
| Target user ID           | target_id            | -i [...]          | -                   | In auto mode    | uint64 (Discord ID)                      | Automatic | [empty]                              |
| 1/chance of effect       | chance               | -d [...]          | -                   | No              | uint32                                   | Automatic | 1                                    |               
| Default # daily turns    | turns                | -T [...]          | -                   | No              | uint32                                   | Manual    | -1 (=infinity)                       |
| Role needed to boomerize | role                 | -                 | -                   | No              | uint64                                   | Manual    | [empty] (no role necessary)          |
| Role for bot managers    | power_role           | -                 | -                   | No              | uint64                                   | Manual    | [empty] (no role necessary)          |
| Give extra turns         | -                    | -                 | /mcum-give-turns    | No              | uint64 (to whom) uint32 (how many)       | Manual    | -                                    |
| Immunize user(s)         | immunized            | -                 | /mcum-immunize      | No              | uint64[] (config.json); uint64 (Discord) | Manual    | [empty]                              |
| Blacklist user(s)        | blacklist            | -                 | /mcum-blacklist     | No              | uint64[] (config.json); uint64 (Discord) | Manual    | [empty]                              |
| Whitelist user(s)        | -                    | -                 | /mcum-whitelist     | No              | uint64                                   | Manual    | -                                    |       

# Compilation directions

Requires: [concord](https://github.com/Cogmasters/concord), [json-c](https://github.com/json-c/json-c), curl, CMake, Unix system with a modern libc (tested on FreeBSD and Linux, probably also works on macOS, maybe with minimal changes)

    git clone https://www.github.com/chickazee4/mcum.git
    cd mcum
    mkdir build && cd build
    cmake ../src
    make
    su -c "make install"

If you want to change the default configuration file location from $PREFIX/etc/mcum/config.json, you will need to pass it as an argument to CMake:

    cmake ../src -DCFGFILE="/path/to/config"

Note that you can still change the configuration file path when initializing the bot.
