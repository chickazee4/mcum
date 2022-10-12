# What is mcum?

mcum is a Discord bot application that will automatically turn a user's posts into the type of meme that you would see on a Facebook page frequented by baby boomers. You'll need to create the Discord API application yourself (I don't share my instance), but mcum will provide the actual function for it - just get your bot token and configure your instance with it.

# Configuration

You do not need to create a config.json in order to use the program (see Usage for how you can enter your configuration otherwise), but it is recommended. The following fields are supported:

* bot_token - your Discord API bot token (not recommended to store this here because it is plaintext, but it's an option if you don't anticipate remote attacks)
* chance - the denominator of the likelihood (1/x) that mcum will create a meme out of a target user's post
* imgflip_username - username for the Imgflip account to use
* imgflip_password - password for the Imgflip account (also not recommended to store in this file for the same reason as the token)
* target_id - the Discord numeric user ID for the target user

# Usage

The most basic usage is:

    mcum

This is fine if you have a config.json in the location that you set as the default during compilation & are content with the settings therein or if you're okay with entering the program configuration interactively once it starts. You can also use the flags -cdiptu to pass arguments to the program if your configuration file exists but is in a non-default location or if you do not have one but do not want to use the interactive entry mode.

    -c [FILE]   Path to your config.json
    -d [INT]    The denominator of the likelihood (1/x) that mcum will create a meme out of the given user's post
    -i [ID]     The target user's Discord ID
    -p [PASSWD] The password for the Imgflip account to be used (not recommended to pass this as an argument due to console logging, but it's an option)
    -t [TOKEN]  The token for the Discord bot to operate (also not recommended to pass as an argument)
    -u [USERNM] The Imgflip username to use

Arguments passed will override those in config.json, if they exist.

The default for mcum is to automatically boomerize a specified target user's posts at your specified frequency, but it also creates a chat command that can be used to manually boomerize any post. You can change which of these is done with command line flags:

    -a          Automatic operation only: no chat command will be created
    -m          Manual operation only: no automatic targeting, users must reply to the target post with the command /boomerize

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
