# Introduction
This repository contains my recreation of various Arena-FPS (AFPS) systems inspired heavily by Quake Champions, (by id Software). This project was created strictly for educational purposes while I learn Unreal Engine 5.

Included is a basic gameplay framework including weapons, health and armour. This project is still in its very early stages, and is therefore a Work in Progress.

# Weapons
Arena shooters rely on the use of many different types of weapons, and fast-paced switching between them. These weapons can be classed into two types: "**hitscan**" and "**projectile**". Hitscan weapons typically deal damage via a line-trace, while projectile weapons spawn a projectile that has some travel time, and deals damage either on impact and/or via splash (area-of-effect/explosion) damage.

Currently I have implemented 3 of the main weapons, and a system for managing and switching between them. More weapons will likely be added at a later date.

## Weapon Container Component
The `WeaponContainerComponent` is an `ActorComponent` used to manage a player's owned weapons. It is responsible for instantiating and tracking an actor's weapons, as well as handling weapon-swap inputs and forwarding the shooting input to the equipped weapon.

## Hitscan Trace Channels
Hitscan weapons can be one of two types:
- Penetrating
- Non-penetrating

In AFPS games, there is typically no "wallbanging", where weapons can shoot through cover/walls. As such, this penetration refers to whether or not the weapon can shoot through multiple enemies (dealing "collateral damage"), or if the first enemy it hits should be the only enemy to be hit.

To streamline these common use cases, I have created two trace channels: `Penetrating-HitScan` and `Non-penetrating-HitScan`. Both will `Block` by default (preventing wallbangs), however `Penetrating-HitScan` overlaps with `Pawns`, allowing for multiple enemies to be hit.

## Weapon Swapping
Each weapon has a dedicated keybind to swap to it. 

When a weapon-swap key is pressed (while a swap is not already in progress), two things can occur:
- If the keybind is for the weapon that is currently equipped, the input will be ignored
- If the keybind is for any other weapon (that the player has) a swap will start

Swapping weapons occurs in two main stages:
- Unequip the equipped weapon
- Equip the new weapon

### Unequipping the Old Weapon
When a new swap starts, the equipped weapon must be unequipped. The amount of time this takes is defined by `AWeaponContainerComponent::WeaponUnequipDelay`. 

If, at the time that the weapon-swap key is pressed, the `EquippedWeapon` is firing, `StopFire()` will be called on that weapon. Before the unequip can start, you must first wait for the `EquippedWeapon`'s `FireDelay` to complete (if it was firing). This prevents the player from firing a slow-fire-rate weapon, then immediately swapping to another weapon.

During the unequip time, any weapon-swap key inputs are valid (including to the weapon that is being unequipped). These inputs will simply update the `WeaponToSwapTo` that will begin equipping after the unequip delay.

### Equipping the New Weapon
Once the previous weapon was unequipped, the new weapon (determined by the most recently pressed weapon-swap key) can be equipped. The amount of time this takes is defined by `AWeaponContainerComponent::WeaponEquipDelay`. 

While a weapon is being equipped, all weapon-swap key inputs are ignored.

If the player is holding the trigger when the equip ends, the new weapon will start immediately firing.

## Example Weapons
### Lightning Gun
The lightning gun is a high-fire-rate, low-damage (per hit), non-penetrating hitscan weapon with a short-medium range. 

### Rail Gun
The lightning gun is a slow-fire-rate, high-damage (per hit), penetrating hitscan weapon with a long range. 

### Rocket Launcher
The rocket launcher is a projectile weapon with a slow projectile travel-speed and no bullet-drop. It deals explosive damage in a radius around it, dealing more damage the closer to the point of detonation the target is. 

Note: the current implementations for `AProjectile_Rocket::CalculateDamage()` and `AProjectile_Rocket::IsDamagePathBlocked()` are quick prototypes. A more correct solution will likely be implemented in the future. 

# Stack (Health + Armour)
"Stack" is a term used by AFPS players to refer to the combination of `Health` and/or `Armour`. 

## Types of Stack
### Health vs Armour
Health and armour are both non-negative integer values. The only differences between them are:
- Which `StackPickups` can heal them (e.g. `HealthBubbles` heal `Health`, while `LightArmours` heal `Armour`)
- Their use when calculating damage (see `"Applying Damage"` below)
- When `Health` reaches `0`, the player dies (not yet implemented); though when `Armour` reaches `0`, nothing happens

### Base-Stack vs Over-Stack
Both `Health` and `Armour` have two key types:
- `BaseHealth`/`BaseArmour`
- `OverHealth`/`OverArmour`

`BaseHealth`/`BaseArmour` is the most common type of stack, and (in most cases) is the kind that the player has the most of. Both can be healed by all healing pickups (of the appropriate `Health`/`Armour` type, respectively), up to some `BaseHealthMax`/`BaseArmourMax` value. 

`OverHealth`/`OverArmour` are portions of `Health`/`Armour` that can be healed above the user's normal maximum `BaseHealth`/`BaseArmour` by "over-healing" pickups (up to some`OverHealthMax`/
`OverArmourMax` value). When a user has `OverHealth` or `OverArmour`, those resources decay overtime, back towards their normal `BaseHealthMax`/`BaseArmourMax`. 

## Applying Damage
When a player takes an instance of damage, a percentage (determined by the recipient's `ArmourEfficiency`) of that damage is applied to their `Armour`, with the remaining damage applied to their `Health`. By default, the `ArmourEfficiency` is `0.67f`, meaning incoming damage is multiplied by `0.67f`, and that amount of damage is applied to the player's `Armour`; before the remaining damage is applied to the player's `Health`. If there is not enough `Armour` to absorb the damage applied to it, then the full remainder is applied to the player's `Health`.

Note that if `IncomingDamage * ArmourEfficiency` results in a decimal, the decimal portion will be truncated (as stack is an integer, while `ArmourEfficiency` is a float). 

Also, there are no differences or special properties (as far as damage is concerned) between `BaseHealth`/`BaseArmour` or `OverHealth`/`OverArmour`.

For example, consider a player taking `90` damage:
- The `90` damage is multiplied by `ArmourEfficiency (0.67f)`, resulting in `60.3`.
	- The `.3` is truncated, leaving `60`
	- This `60` is the damage that is applied to the player's `Armour`
	- This example assumes the player has `60` or more `Armour` at the time they took the damage
- The damage dealt to `Armour` (`60`) is subtracted from the total incoming damage `(90)`, leaving `30` damage that needs to be applied to the player's `Health`
- So in summary: `60` damage is dealt to `Armour`, and `30` damage is dealt to `Health`

Now consider a player taking `90` damage, while they only have `7` `Armour` at that time:
- The `90` damage is multiplied by `ArmourEfficiency (0.67f)`, resulting in `60.3` (truncated to `60`)
	- This `60` is the damage that is applied to the player's `Armour` 
		- As they only had `7` `Armour` remaining, only `7` damage can be dealt to `Armour`
- The damage dealt to `Armour` (`7`) is subtracted from the total incoming damage `(90)`, leaving `83` damage that needs to be applied to the player's `Health`
- So in summary: `7` damage is dealt to `Armour`, and `83` damage is dealt to `Health`

# Planned Future Work
## Short-term
- Update current features to work in multiplayer
- Create weapon pickups (and ammo pickups)

## Mid-term
- Allow players to edit their keybinds, mouse sensitivity, etc
	- Save these between load/close
- Create a very simple map
	- Include spawn points, health pickups, weapon pickups
- Create basic game mode logic 
- Create a main menu -> game flow
- Create post-game-menu flow 

## Long-term
- Add knockback to weapons
- Create remaining weapons
- Create powerups
- Create champions and their abilities


