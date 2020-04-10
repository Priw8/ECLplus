# ECLplus
ECLplus is a modification for Touhou 17 - Wily Beast and Weakest Creature. It's main goal is adding more functionalities for ECL scripts - this includes both custom instructions and custom variables.

## Usage
### Installation
Put `ECLPLUS.dll` and `ECLplusLoader.exe` in the folder where `th17.exe` is located, and launch the game by using the loader. If you wish to use thcrap, you can change `th17.exe` in `games.js` to `ECLplusLoader.exe` and it will just work. If everything went correctly, a console with a message from ECLplus will start alongside the main game window (small note: with thcrap console enabled, both ECLplus and thcrap will end up outputting to the same console, and the ECLplus message will get burried under a massive amount of thcrap logs).

### Using custom instructions
In your ECL source file, `#include` the `ECLplus.tecl` script from the `ECLinclude` directory in order to get instruction names and definitios of ECLplus. Then, you can use the new instructions freely.

### Publishing modifications that use ECLplus
Simply including the content of the [LICENSE.txt](LICENSE.txt) file in the mod with some way of telling that it refers to ECLplus is fine (for example, in a file called `LICENSE_ECLplus.txt`).

## Instruction list
**ins_2000 series: various things that don't fall into a specific category**
- `msgf(string format, ...)` - shows a printf-formatted message box. Example: `msgf("Hello from ECL! My id is %d", _SS ID)` (note that since this uses the `D` param type for extra parameters, prefixing with typecast is necessary as of current thecl version).  
- `printf(string format, ...)` - prints a printf-formatted string in the console, same concept as the instruction above.
- `cls()` - clears the console.
- `drawf(float x, float y, string format, ...)` - draws a printf-formatted string on the given coordinates in the game window (ECL coordinate system). Top-left corner of the string corresponds to the coordinates given. It will only be drawn for 1 frame, in order to keep it displayed all the time `drawf` needs to be in a loop.
- `drawColor(int c)` - sets color of strings drawn by `drawf`. Example: `drawColor(0xFFFF8800)` sets color to orange. Alpha is stored in the MSB, so that the order is ARGB; the example has an alpha value of `FF`, red value of `FF`, green value of `88` and blue value of `00`.
- `drawFont(int f)` - sets font of strings drawn by `drawf`.  The argument is the ID of an `ascii.anm` font.  Be aware that not all fonts support all printable ASCII characters, and the game may crash if the chosen font cannot render the given text.
- `drawAnchor(int h, int v)` - sets anchor point of strings drawn by `drawf`.  `h = 0/1/2` is center/left/right.  `v = 0/1/2` is center/top/bottom.  Default is `(1, 1)` (left/top). Globals `ANCHOR_CENTER`, `ANCHOR_LEFT`, `ANCHOR_RIGHT`, `ANCHOR_TOP` and `ANCHOR_BOTTOM` are provided for convenience.
- `drawShadow(int bool)` - enable or disable drop shadows for `drawf`.
- `drawReset()` - resets all modified attributes of `drawf` to their defaults.
- `playMusic(string name)` - replaces currently playing music with a new one. The name must be the same as in `thbgm.fmt` (e.g. `th17_06.wav`).
- `exit()` - unconditionally exits to the main menu (does not allow saving replays)
- `itemSpeed(float factor)` - set item falling speed factor, when the factor is smaller than `1.0f` the items automatically get the animation they have in LoLK. The game automatically increases the factor by `0.1f` every frame until it's back to `1.0f`.
- `spellSetCapture(int cap)` - change current spell's capture state, `0` will set bonus to "failed", any other value will allow capturing the spell. Does NOT set the ECL `CAPTURE` variable to 0.
- `spellSetBonus(int bonus)` - set both max spell bonus and current bonus of the current spell to the given value.
- `spellSetBonusNow(int bonus)` - set only the current bonus of the current spell. This means that the speed at which the bonus decreases will still be determined by the old max spell bonus value.

**ins_2100 series: player manipulation**  
- `playerPos(float x, float y)` - move the player to the given coordinates.
- `playerKill()` - immidiately kill the player.
- `playerBomb()` - immidiately make the player bomb (even if there are 0 bombs, though using this instruction does take away a bomb from the player).
- `playerSetLives(int lives)` - set the amount of lives to the given amount.
- `playerSetBombs(int bombs)` - set the amount of bombs to the given amount.
- `playerSetPower(int power)` - set player's power.
- `playerSetIframes(int frames)` - set player's invincibility frame count.
- `playerAllowShot(int state)` - disable/enable player's ability to shoot.
- `playerAllowBomb(int state)` - disable/enable player's ability to bomb.
- `playerSetHyperTimer(int time)` - manipulate the timer of an ongoing hyper.

**ins_2200 series: extended enemy intertaction**
A more in-depth explanation of the message system can be found [here](EnmMsg.md).
- `msgResetAll()` - fully resets the messages and frees any memory allocated for them.
- `msgReset(int channel)` - reset and remove the given channel.
- `msgSend(float a, float b, float c, float d, int channel)` - send message on the given channel.
- `msgReceive(intvar received, floatvar a, floatvar b, floatvar c, floatvar d, int channel)` - receive message from the given channel. The `a`-`d` variables will be set to the values from the message (if there are any messages to receive), `received` variable will be set to 1 if a message was received and to 0 otherwise.
- `msgPeek(intvar received, floatvar a, floatvar b, floatvar c, floatvar d, int channel)` - similar to `msgReceive`, except it doesn't actually receive the message (it stays on the list of messages in the channel). Can be used to check the message before actually deciding to receive it.
- `msgCheck(intvar receive, int channel)` - sets `receive` to 1 if there are any messages to be received in the given channel, 0 otherwise.
- `msgWait(int channel)` - wait until there is a message to be received in the given channel (actually an inline sub that uses `msgCheck` and not an instruction)  
  
More detailed explanation of enemy IDs vs enemy iterators can be found [here](EnmIdIter.md).  
In all following instructions, every time there is a variable parameter that gets set to some sort of return value, any non-variable parameter can be given to ignore the return value.
- `enmClosest(int &varId, float &varDist, float x, float y)` - sets `varId` to ID of the enemy that's closest to the given (`x`, `y`) coordinates, and `varDist` to the distance itself. Ignores enemies that are intangible (flag 32) or have no hurtbox (flag 1).
- `enmDamage(int id, int dmg [, int isBomb])` - unconditionally deal `dmg` damage to the enemy with the given ID. The third, optional parameter specifies whether the damage should be treated as bomb damage (that is, not get dealt when the enemy has bomb shield and get affected by the bomb damage multiplier).
- `enmDamageIter(int iter, int dmg, [, int isBomb])` - same as above, but uses an iterator instead of ID.
- `enmDamageRad(int &varCnt, float x, float y, float r, int maxCnt, int dmg [, int isBomb])` - deal `dmg` damage enemies within the circle with center at coordinates (`x`, `y`) and radius `r`. Maximum amount of enemies that can be damaged is determined by `maxCnt` (set to `-1` for unlimited amount), prioritizing damaging enemies closer to the center of the circle. The amount of the damaged enemies is written to `varCnt`. The optional `isBomb` works the same as in the previous instructions.
- `enmDamageRect(int &varCnt, float x, float y, float w, float h, int maxCnt, int dmg [, int isBomb])` - same as above, but damages a rect of width `w` and height `h` with center at coordinates (`x`, `y`) instead of a circle.
- `enmIterate(int &varIterator, int prevIterator)` - used to iterate over the enemy list. If `prevIterator` is 0, then `varIterator` is set the the iterator of the first enemy. If it's not 0, then it must be a valid iterator, and in this case `varIterator` will be set to iterator of the enemy that's after `prevIterator`.
- `enmIdIter(int &varId, int iter)` - sets `varId` to ID value of enemy that iterator `iter` refers to.
- `enmIterId(int &varIter, int id)` - sets `varIter` to the iterator that refers to enemy with the given ID.
- `enmFlag(int &varFlag, int id)` - sets `varFlag` to flags of enemy with ID `id`.
- `enmFlagIter(int &varFlag, int iter)` - sets `varFlag` to flags of enemy that iterator `iter` refers to.
- `enmLife(int &varLife, int id)` - sets `varLife` to current HP of enemy with ID `id`.
- `enmLifeIter(int &varLife, int iter)` - sets `varLife` to current HP of enemy that iterator `iter` refers to.
- `enmPosIter(float &varX, float &varY, int iter)` - sets `varX` and `varY` to current coordiantes of enemy that iterator `iter` refers to.
- `enmBombInvuln(float &varInvuln, int id)` - sets `varInvuln` to bomb damage multiplier (set by `ins_565`) of enemy with ID `id`.
- `enmBombInvulnIter(float &varInvuln, int iter)` - sets `varInvuln` to damage multiplier (set by `ins_565`) of enemy that iterator `iter` refers to.

## Variable list
- `GI4` up to `GI7` - additational global integer variables, use them for whatever you want.
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
- `IFRAMES` - int, the amount of iframes the player has. Not writable, use `playerSetIframes` instruction instead.
- `PLSTATE` - int, player state. The following constants from `ECLplus.tecl` contain possible values: `PL_NORMAL`, `PL_DYING`, `PL_DEAD` and `PL_RESPAWN`. Writable (but just because you can doesn't mean that you should).
- `HYPERTIMER` - int, timer of a hyper. Not writable, use `playerSetHyperTimer` instruction instead.
- `DIALOG` - int, 0 if no dialog is active, nonzero if there is dialog active. Not writable.
- `SPELLBONUS` - int, current spell bonus. Returns `0` if there is no spell active.
