# Priorities and enemy execution order in ECLplus

Priorities are an extremely powerful feature of ECLplus, but as we all know, with great power comes great responsibility.  In order to understand how they work, however, we must first understand how the game deals with the concept of frames.

## What is priority?

Touhou 17 has numerous global objects that each register `on_tick` functions to run on every frame.  For instance, the Player object registers an `on_tick` function that checks the `INPUT` variables and makes the character move, shoot, and trigger bombs.  The ShotType object registers an `on_tick` function that advances the state of an in-progress bomb.  The EnemyManager updates all enemies, including running their ECL.  And et cetera.

Every `on_tick` function has a **priority** associated with it; a nearly-unique number that generally indicates the order that it runs in.  For instance, the Player's `on_tick` function has priority 23, while EnemyManager's has priority 27, so the player is updated before all ECL scripts.  In other words it could be said that all enemies have priority 27.

The priority feature of ECLplus makes it possible to make **enemies that run on non-standard priorities.**  E.g. you could make an ECL script that runs after the player, but before all enemies; or one that runs after all items, but before all beast tokens.  You could even make enemies that run while the game is paused!... more on this later.

### `PRIORITY` and `PRIORITY_REL`

As stated, by default, all enemies run during the update func of priority 27.  This is equivalent to the ECLplus instruction

```
priority(PRIORITY_ENEMY, REL_DURING);
```

The second argument is called the "relative timing."  `REL_DURING` means it actually runs during that update function.  This makes perfect sense for the Enemy update function, but it does not make sense for any of the others.  Thus, for anything else, you must use one of the following:

* `priority(p, REL_BEFORE);` will run immediately before the update function of priority `p`.
* `priority(p, REL_AFTER);` will run immediately after the update function of priority `p`.

Update order is always sorted first by priority, so e.g. `priority(15, REL_AFTER)` will run before `priority(16, REL_BEFORE)`.

A full list of the priorities is available in `ECLplus.tecl`.

### UI priorities

The update function at priority 16 (`PRIORITY_UI_CUTOFF`) is extremely special.  This function checks whether the game is paused, and, if so, *it prevents all of the update functions after it from running.*  Thus, enemies with a priority of `(PRIORITY_UI_CUTOFF, REL_BEFORE)` or earlier will run even during the pause menu.  **Such priorities are called UI priorities.** This is great for e.g. enemies that use `drawf`.

```
void LevelDisplayEnemy() {
    priority(PRIORITY_ASCII, REL_AFTER);
    while (true) {
        drawf(0f, 0f, "%d", $GI4);
        wait(1);
    }
}
```

## Precise semantics of `priority` and the `PRIORITY` variables

When you call `priority`, the enemy will not *immediately* change into that priority.  Rather, it will begin using that priority on the next frame (i.e. after a `wait(1)`).  Similarly, the variables `PRIORITY` and `PRIORITY_REL` reflect the priority at which the enemy will run on the next frame, meaning they are immediately updated by `priority`.

By default, children inherit the priority of their parent at the time of their creation.  To clarify, they inherit `PRIORITY` and `PRIORITY_REL`, so that something like

```
priority(p, r);

// these children will default to priority(p, r) even though the parent may still
// technically be running from a different priority.
enmCreate("ChildA", 0f, 0f, 1, 0, 0);
enmCreate("ChildB", 0f, 0f, 1, 0, 0);
```

Enemies **never** run two frames of an ECL script in one frame of game time, even if an enemy changes priority to a higher number., it will not get to run an extra tick when the update functions of that priority run on this frame.

## Advice for using `priority`

* **Use priorities sparingly.**  Prefer to use the game's built-in mechanisms for ordering enemies whenever possible.  E.g. if you're simply trying to ensure that `AwesomePowerGaugeEnemy` runs after `AwesomePowerInputEnemy`, then have some parent enemy create them in that order.  Priorities are for accomplishing things that are otherwise impossible, e.g. making an `AwesomePlayerBulletEnemy` run before all of the enemies do.
* **UI enemies should not do anything that may have an impact on non-UI enemies,** or you will break replays.  Do not draw random numbers from the replay-saved RNG.  Do not do anything like kill the player or create items.
* **Do not ever change from a UI priority to a non-UI priority.**  Doing this has the potential to break replays.  *Even if the enemy does not interact with others or the game world in anyway whatsoever,* merely inserting them into the list associated with a non-UI priority may subtly change the behavior of other enemies at that priority, due to certain vanilla bugs in run order.
* Understand that **`priorities` are an experimental feature.**  There are things that we still do not understand about how the game works.  There may be bugs.

## Appendix: Enemy run order (and vanilla bugs!)

Knowing the run order of scripts in the vanilla game is useful background information that can help explain why the `priority` command works the way it does, and why some of the recommendations are the way they are.

Whenever an `enmCreate` instruction is encountered, the following occurs:

* The child is initialized, with some variables copied from the parent or taken from the `enmCreate` command.
* The child's ECL script is **immediately run for a single frame**, interrupting the parent's script.
* Once the child's script completes a frame, the child is inserted **at the end of the active enemies list.**
* There is a mechanism in place to prevent the child from running a second time on the first frame of its existence; when it was ran early, a flag was set.  Later on in this frame, when the game encounters this enemy while iterating over the list, it will see that flag, clear it, and skip the enemy.

Some notable aspects of this:

* If an enemy creates three children in the order A, B, and C, then those children will always run in the order A, B, C.
* *In most cases,* children will run after their parents in the run order.  **However,** if a parent creates a child during its very first frame, then the parent will not yet be in the list by the time the child gets added to the list, and therefore in this case the child will end up before the parent in the run order.
* There is a timing bug in how the game prevents enemies from running twice.  Specifically, if the parent is the *very last enemy in the enemy list,* then the game will fail to clear the aforementioned flag, and will end up skipping the child on the next frame. (causing it to "miss" one frame entirely).  ECLplus replicates this bug whenever priorities are not explicitly used.

