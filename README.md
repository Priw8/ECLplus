# ECLplus
ECLplus is a modification for Touhou 17 - Wily Beast and Weakest Creature. It's main goal is adding more functionalities for ECL scripts - this includes both custom instructions and custom variables.

## Usage
### Installation
Put `ECLPLUS.dll` and `ECLplusLoader.exe` in the folder where `th17.exe` is located, and launch the game by using the loader. If you wish to use thcrap, you can change `th17.exe` in `games.js` to `ECLplusLoader.exe` and it will just work. If everything went correctly, a console with a message from ECLplus will start alongside the main game window (small note: with thcrap console enabled, both ECLplus and thcrap will end up outputting to the same console, and the ECLplus message will get burried under a massive amount of thcrap logs).

### Using custom instructions
In your ECL source file, `#include` the `ECLplus.tecl` script from the `ECLinclude` directory in order to get instruction names and definitios of ECLplus. Then, you can use the new instructions freely.

## Instruction list
**ins_2000 series: general output/debug utilities**
- `msgf(string format, ...)` - shows a printf-formatted message box. Example: `msgf("Hello from ECL! My id is %d", _SS ID)` (note that since this uses the `D` param type for extra parameters, prefixing with typecast is necessary as of current thecl version).  
- `printf(string format, ...)` - prints a printf-formatted string in the console, same concept as the instruction above.
- `cls()` - clears the console.
- `drawf(string format, float x, float y, ...)` - draws a printf-formatted string on the given coordinates in the game window (ECL coordinate system). Top-left corner of the string corresponds to the coordinates given. It will only be drawn for 1 frame, in order to keep it displayed all the time `drawf` needs to be in a loop.
- `drawColor(int c)` - sets color of strings drawn by `drawf`. Example: `drawColor(0xFF00FF00)` sets color to green. The order is reversed (ABGR), so this has alpha value of `FF`, blue value of `00`, green value of `FF` and red value of `00`.

**ins_2100 series: player manipulation**  
- `playerPos(float x, float y)` - move the player to the given coordinates.
- `playerKill()` - immidiately kill the player.
- `playerBomb()` - immidiately make the player bomb (even if there are 0 bombs, though using this instruction does take away a bomb from the player).
- `playerSetLives(int lives)` - set the amount of lives to the given amount.
- `playerSetBombs(int bombs)` - set the amount of bombs to the given amount.

## Variable list
- `INPUT` - int, player input bitmask (works with replays). Writing to it has no effect.
- `SCORE` - int, score of the player (not including the last digit). Writable.
- `HIGHSCORE` - int, highscore of the player (not including the last digit). Writable.
- `BOMBING` - int, 1 if player is bombing, 0 otherwise. Not writable.
- `LIVES` - int, current amount of lives. Not writable, use `playerSetLives` instruction instead.
- `BOMBS` - int, current amount of bombs. Not writable, use `playerSetBombs` instruction instead.
