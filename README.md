# Compilation directions

Requires: [concord](https://github.com/Cogmasters/concord), [json-c](https://github.com/json-c/json-c), curl, CMake, Unix system with a modern libc (designed on FreeBSD, should work on Linux and probably macOS)

To compile, you need to create your own "mcum_configuration.h" file in the base source tree, containing information for your specific instance. You need defines for the following variables:

    #define CHANCE_DENOMINATOR [int]
    #define TARGET_USER_ID [ulong]
    #define IMGFLIP_USERNAME [char*]
    #define BOT_TOKEN [char*]

* CHANCE_DENOMINATOR determines how likely it will be that any given post by the target user is boomerized. A higher number means a lower chance (the exact probability is more or less equal to 1 / CHANCE_DENOMINATOR).
* TARGET_USER_ID is the user whose posts will be boomerized. This is the internal Discord ID, which does not change when a user changes their username, so it is more persistent. You can access IDs by turning on developer mode in Discord and viewing the target user's profile.
* IMGFLIP_USERNAME is the username that will be used for the Imgflip API (which is used to generate the memes).
* BOT_TOKEN is your bot's unique token, which you can find on its page in your Discord developer dashboard.

Then:

    mkdir build && cd build
    cmake ..
    make
    su -c "make install"
