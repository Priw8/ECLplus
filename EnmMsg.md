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
