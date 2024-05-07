# Introduction
This repository contains my recreation of Tracer's abilities from Overwatch (2), (by Blizzard Entertainment). This project was created strictly for educational purposes while I learn Unreal Engine 5. 

***THIS IS A WORK IN PROGRESS!*** I still have plans for things to add/ improve, such as adding UI elements showing cooldowns, allowing multiple charges of abilities, etc. 

Included is a basic gameplay framework for starting/ stopping actions, modifying a character's health, among other things. Most importantly, implementations of the abilities "blink", "recall", and "pulse bomb" are included. More details on these are provided below.

This project was intended purely to showcase my implementations of these abilities. As such, most elements of the "game" are very basic or not implemented. For example, there is no real game mode or player death, so health simply gets capped at 0. These areas may or may not get fleshed out more in the future.

# Blink
## Description
Blink allows the player to teleport a short distance in the direction of their most recent movement input. The player can teleport through other players, but terrain (such as walls) blocks the move- placing the player up against the terrain after the teleport.

Blink has 3 starting charges and 3 max charges, each with a 10 second cooldown. If multiple charges have been consumed, one charge will be restored every 10 seconds.

Blink is bound to `Left Shift` and `Right Click` (i.e. either key can be used).

## Implementation
My implementation starts by sweeping a capsule (of the same dimensions as the player's character) in the direction of the player's last movement input. 
- If no movement input is provided (i.e. blink is the only key being pressed), the character's forward vector is used.
- If this sweep does **not** result in a hit (with a `WorldStatic` or `WorldDynamic` object), the player will be teleported by the `MaxBlinkDistance` in the direction of their input. 
- If the sweep does result in a hit, the `HitResult.ImpactPoint` is taken as a starting point. From here, the player's capsule's radius is added in the opposite direction of the sweep (i.e., from the impact point to the player's starting location). The result of this is used as the destination to teleport the player to. This is done to ensure the player is teleported up against the wall, rather than relying on UE5's `AActor::TeleportTo(...)` to resolve the collision, which can result in the teleport failing.
- The player's **horizontal** speed is also preserved across the blink.
	- I.e. if the player is moving at 500 ups (units per second) to their left, then they immediately press right and blink, they will be teleported to the right, and will be moving at 500 ups to their right.

# Recall
## Description
Recall is the most complicated of the abilities. It rewinds the player to their location a certain number of seconds ago (by default, 3 seconds). This rewind process takes an amount of time, during which:
- The user is both invisible and has no collision
- The user can't perform any other inputs
- The user is shown a rough rewind animation through where they were/ where they were looking during the time that is being rewound (i.e. the 3 seconds before the ability was used)

At the end of the ability, the player's health is set to its maximum value from any point in time during the duration that was rewound. 

Recall has 1 starting charge and 1 max charge, on a 15 second cooldown.

Recall is bound to `E`.

## Implementation
I created an `FRecallData` struct that stores the data for any given point in time. This includes the character's location, rotation (specifically, their `APawn::GetControlRotation()`) and health. 

A `TArray<FRecallData>` is used to store the recall data for the entire duration that can be rewound. This array is treated somewhat like a queue, where elements at the front (lower index) are the most recent entries, while elements at the back (higher index) are the oldest entries.

When the ability is initialised, a looping timer (called the `PushRecallData` timer) is started that frequently pushes a new entry into the array, using the character's current location/ rotation/ health. On initialisation, a second, non-looping, timer is started (called the `MaxQueueSize` timer) that lasts for the maximum recall length (i.e. the amount of time that will be rewound, not the amount of time that it takes for the rewind to complete). When this timer ends, a boolean flag is set that will cause the oldest entry to be popped off the array each time a new entry is about to be pushed into the array. This ensures that the array only holds data from the relevant time period. 

When the ability is started, the following things happen:
- The player's character is set to be invisible, and their collision is disabled
- The player's controller has its input disabled
- Both the `PushRecallData` and `MaxQueueSize` timers are cleared (to prevent data changing during the recall process)
- The duration for each segment is calculated based on the number of elements in the array and the `ActiveDuration` of the ability
	- "Segment" refers to the time taken to rewind the character from the position/rotation from one entry in the array to the next
	- `ActiveDuration` is the time it takes for the ability to complete (from the activation button being pressed, until the player regains control after being rewound)
- A new looping timer (called the `RecallSegment` timer) is started, which  updates the character's position periodically
	- The function called by this timer uses the following information:
		- The position and rotation of the character at the start of the current segment
		- The position and rotation of the character at the end of the current segment
		- The time that this segment started at
		- The duration of this segment (which is the same for all segments during a given use of the ability)
	- Additionally, this function gets the current game time from the world timer manager
	- This information is used to calculate an approximate position and rotation for the character at this update interval using lerp functions
	- If enough time has passed such that the current segment has ended, information for the next segment is obtained from the array, and the timer is reset with this new information
		- At the start of each new segment, that array entry's `Health` value is checked. If it is larger than the max value that has been previously seen (during the current ability usage), it will be recorded.
	- Once the end of the array has been reached, the ability is stopped

When the action ends, the following events occur:
- The player's health is increased to match the largest health value of any recall segment (this includes their health at the time that they started the ability)
- The `PushRecallData` and `MaxQueueSize` timers are started again
- The recall data array is emptied
- The flag is reset to indicate that entries should not be popped off the array (until the `MaxQueueSize` timer ends again)
- The player's character is set to be visible again, and their collision is re-enabled
- The player's controller input is re-enabled
- A data entry is immediately pushed into the array
	- This prevents any issues that would occur if the cooldown were lower than the rate at which new entries are pushed into the queue, causing a potentially empty array

# Pulse Bomb
## Description
The player can throw a pulse bomb. This bomb explodes after a short period of time, dealing damage in a radius around it. The closer the bomb is to a target, the more damage it will deal to that target.

The bomb can stick to surfaces and players. Once a bomb is stuck to an object/player, that bomb can't be un-stuck, or stick to anything else. When a bomb sticks to a player, it turns invisible.

Pulse Bomb has 0 starting charges and 1 max charge, on a 20 second cooldown. 

Pulse Bomb is bound to `Q`

## Implementation
The pulse bomb is implemented in two parts- the projectile and the ability. The projectile is an `AActor` that is responsible for properties like the radius of the explosion, its damage, and its movement. The action is simply a means to spawn the projectile. 

### Projectile
#### Explosion
When it's spawned, the projectile starts a timer. When this timer finishes, the projectile "explodes". When the explosion occurs:
- The projectile gets an array of all `ECC_Pawn` actors within a set radius from its location (known as `MinDamage_Range`, as this is the range at which the minimum damage will be applied) 
	- The projectile checks each of these actors to see if they have a health component 
	- All of these actors that do have a health component are potentially damaged by the projectile 
		- A path trace is used to check if there is a `WorldStatic` or `WorldDynamic` object between the Actor and the bomb
			- If there is, no damage is dealt.
		- Assuming there is no object blocking the damage:
			- If an actor is within a small radius of the bomb (`MaxDamage_Range`), maximum damage will be dealt
			- If an actor is outside the `MaxDamage_Range` (but still inside the explosion's radius, `MinDamage_Range`), some amount of damage falloff will apply 
				- This damage is calculated using a lerp function that takes the bomb's minimum and maximum damages, its minimum and maximum ranges, and the distance of the actor from the bomb.
				- The actual formula used is: `FMath::Lerp(MaxDamage, MinDamage, ( (Distance - MaxDamage_Range) / (MinDamage_Range - MaxDamage_Range) ));`
					- Note: The case where `MinDamage_Range - MaxDamage_Range == 0` is not possible due to other checks in the function. 

#### Stick
The projectile also contains a sphere component. This is used to detect collisions, which is used for the "stick" mechanic. When an overlap occurs:
- The projectile's velocity is set to 0
- The projectile's gravity is set to 0

If the actor that caused the overlap is an `APawn`:
- The bomb is set to be invisible
- The bomb is attached to that pawn

**Note:**
- This implementation assumes `WorldStatic` and `WorldDynamic` objects don't move
- This implementation assumes that the stuck `APawn` won't de-spawn or die
	- If death is later implemented, some handling would likely need to be done to drop the bomb in place, and re-enable the ability for it to stick to a new actor.

### Action/ Ability
The pulse bomb action is relatively straight forward. It simply spawns the projectile at the location of, and with the same rotation as, the user's `GetActorEyesViewPoint(...)`. The user's pawn is marked as the projectile's `Instigator`. This is used to prevent the user from being able to stick themself with their own pulse bomb.

# Debug Functionality
Some basic debugging functionality has been included to assist with development/ to showcase functionality.

## Blink Debug CVar
The blink ability contains a boolean cvar that can be toggled with `t.BlinkDebug true`/ `t.BlinkDebug false` in the in-game console. 

If set to true, the following debug shapes will be draw when the ability is used:
- A green sphere at the blink's start location
- A blue sphere at the end point of the sweep
	- This will be the point obtained by adding the maximum blink distance, in the direction of the user's last movement input, to the blink's start location
- If the sweep resulted in a hit, a red sphere will be shown at the impact point
- A capsule will be shown at the location the player was teleported to
	- It will be red if the sweep resulted in a hit, and white otherwise

## Pulse Bomb Debug CVar
The pulse bomb projectile contains a boolean cvar that can be toggled with `t.PulseBombDebug true`/ `t.PulseBombDebug false` in the in-game console. 

If set to true, the following will occur:
- When the bomb explodes, a red sphere will be shown at the location of the bomb, with a radius matching that of the explosion
- When the bomb overlaps with an actor, a sphere will be shown at this location
	- It will be yellow if the actor it overlapped with was a pawn, and blue otherwise

## `HealSelf` Command
The playable character has access to a console command that can be accessed by typing `HealSelf`, or `HealSelf x`, where `x` is the amount you want to heal your character by (e.g. `HealSelf 10`), in the in-game console. If no value is provided, the default of 100 will be used.

As the name suggests, this command is used to heal your character. If a non-positive value is entered, this command will have no effect. This command can only be used to heal up to your character's (or more specifically, your character's health component's) `HealthMax` and no higher. 

## Recall Stats
The recall ability has 2 `CYCLE_STAT`s that can be used to analyse the time taken by the `UpdateActorTransform` function. These can be accessed by typing `Stat Tracer` in the in game console.

`UpdateActorTransform` refers to the entire `UpdateActorTransform()` function, while `UpdateActorTransform.NewSegment` refers only to section within this function related to setting up a new segment. 


