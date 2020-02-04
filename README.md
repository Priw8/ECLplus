# ECLplus
ECLplus is a modification for Touhou 17 - Wily Beast and Weakest Creature. It's main goal is adding more functionalities for ECL scripts - this includes both custom instructions and custom variables.

## Usage
### Installation
Put `ECLPLUS.dll` and `ECLplusLoader.exe` in the folder where `th17.exe` is located, and launch the game by using the loader. If you wish to use thcrap, you can change `th17.exe` in `games.js` to `ECLplusLoader.exe` and it will just work. If everything went correctly, a console with a message from ECLplus will start alongside the main game window (small note: with thcrap console enabled, both ECLplus and thcrap will end up outputting to the same console, and the ECLplus message will get burried under a massive amount of thcrap logs).

### Using custom instructions
In your ECL source file, `#include` the `ECLplus.tecl` script from the `ECLinclude` directory in order to get instruction names and definitios of ECLplus. Then, you can use the new instructions freely.

## Instruction list
**ins_2000 series: various things that don't fall into a specific category**
- `msgf(string format, ...)` - shows a printf-formatted message box. Example: `msgf("Hello from ECL! My id is %d", _SS ID)` (note that since this uses the `D` param type for extra parameters, prefixing with typecast is necessary as of current thecl version).  
- `printf(string format, ...)` - prints a printf-formatted string in the console, same concept as the instruction above.
- `cls()` - clears the console.
- `drawf(string format, float x, float y, ...)` - draws a printf-formatted string on the given coordinates in the game window (ECL coordinate system). Top-left corner of the string corresponds to the coordinates given. It will only be drawn for 1 frame, in order to keep it displayed all the time `drawf` needs to be in a loop.
- `drawColor(int c)` - sets color of strings drawn by `drawf`. Example: `drawColor(0xFF00FF00)` sets color to green. The order is reversed (ABGR), so this has alpha value of `FF`, blue value of `00`, green value of `FF` and red value of `00`.
- `playMusic(string name)` - replaces currently playing music with a new one. The name must be the same as in `thbgm.fmt` (e.g. `th17_06.wav`).

**ins_2100 series: player manipulation**  
- `playerPos(float x, float y)` - move the player to the given coordinates.
- `playerKill()` - immidiately kill the player.
- `playerBomb()` - immidiately make the player bomb (even if there are 0 bombs, though using this instruction does take away a bomb from the player).
- `playerSetLives(int lives)` - set the amount of lives to the given amount.
- `playerSetBombs(int bombs)` - set the amount of bombs to the given amount.
- `playerSetPower(int power)` - set player's power.

**ins_2200 series: enemy message system**
A more in-depth explanation of the message system can be found later in the README.
- `msgResetAll()` - fully resets the messages and frees any memory allocated for them.
- `msgReset(int channel)` - reset and remove the given channel.
- `msgSend(float a, float b, float c, float d, int channel)` - send message on the given channel.
- `msgReceive(intvar received, floatvar a, floatvar b, floatvar c, floatvar d, int channel)` - receive message from the given channel. The `a`-`d` variables will be set to the values from the message (if there are any messages to receive), `received` variable will be set to 1 if a message was received and to 0 otherwise.
- `msgPeek(intvar received, floatvar a, floatvar b, floatvar c, floatvar d, int channel)` - similar to `msgReceive`, except it doesn't actually receive the message (it stays on the list of messages in the channel). Can be used to check the message before actually deciding to receive it.
- `msgCheck(intvar receive, int channel)` - sets `receive` to 1 if there are any messages to be received in the given channel, 0 otherwise.
- `msgWait(int channel)` - wait until there is a message to be received in the given channel (actually an inline sub that uses `msgCheck` and not an instruction)

## Variable list
- `INPUT` - int, player input bitmask (works with replays). Writing to it has no effect.
- `SCORE` - int, score of the player (not including the last digit). Writable.
- `HIGHSCORE` - int, highscore of the player (not including the last digit). Writable.
- `BOMBING` - int, 1 if player is bombing, 0 otherwise. Not writable.
- `LIVES` - int, current amount of lives. Not writable, use `playerSetLives` instruction instead.
- `BOMBS` - int, current amount of bombs. Not writable, use `playerSetBombs` instruction instead.
- `GRAZE` - int, current amount of graze the player has. Writable.
- `PIV` - int, current point item value. Note that this value is larger than the one displayed on the screen because of how it's stored internally. Writable.
- `CONTINUES` - int, the amount of continues used so far (number displayed as last digit of the score). Writable.
- `CREDITS` - int, the amount of credits left (how many times can the player continue). Writable.

## Enemy message system
The message system allows easy communication between enemies without needing to use global variables. There are 2 basic operations - sending a message in the given channel and receiving a message from the given channel. The channel can be any number, though if you want to communicate 2 enemies (parent and child) you can use `ID` variable in the child enemy and `LAST_ENM_ID` in the parent enemy. Make sure to properly initialize channels used, as they do not get reset automatically at any point (you could actually send a message at the end of stage 6, start extra stage and receive it there). This can be done with `msgResetAll` instruction which resets all channels, or the `msgReset` instruction which resets only the specified channel. I recommend calling `msgResetAll` at the beginning of every stage if you use messages, in order to avoid memory leaks. In case of spawning a child that will use messages, make sure to reset the channel from the child, as child ECL code actually executes immidiately after spawning (before the instruction after `enmCreate` executes).

### Example 1 - sending messages between 2 siblings
```c
void sender() {
    flagSet(32);
    msgReset(42069); // reset channel 42069
    while(1) {
        msgSend(RANDF, RANDF2, RANDRAD, PLAYER_X, 42069);
        wait(60);
    } 
}

void reader() {
    flagSet(32);
    while(1) {
        int received;
        float a, b, c, d;
        msgReceive(received, a, b, c, d, 42069);
        if (received)
            printf("I got a message! a=%f, b=%f, c=%f, d=%f\n", _ff a, _ff b, _ff c, _ff d);
        else
            printf("There were no messages!\n");
        wait(50);
    }
}

void main()
{
    cls();
    enmCreate("sender", 0f, 0f, 100, 100, 0);
    enmCreate("reader", 0f, 0f, 100, 100, 0);
    while(1)
        wait(1000);
}
```

### Example 2 - sending message from parent to child
```c
void reader() {
    msgReset(ID);
    msgWait(ID);
    printf("Hello\n");
}

void main() {
    cls();
    enmCreate("reader", 0f, 0f, 100, 100, 0);
    int id = LAST_ENM_ID;
    wait(360);
    msgSend(0f, 0f, 0f, 0f, id);
    while(1)
        wait(1000);
}
```
