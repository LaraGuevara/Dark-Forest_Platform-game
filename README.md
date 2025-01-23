# Dark Forest - Platform Game

## GITHUB
---
[Project](https://github.com/LaraGuevara/Platform-game)

## MEMBERS 
---
* Lara Guevara->[LaraGuevara](https://github.com/LaraGuevara)
* Maria Besora->[mariabeo](https://github.com/mariabeo)

## ACKNOWLEDGEMENTS
---
* Player sprite credit:
  * [Witches Pack](https://9e0.itch.io/witches-pack) by [9E0](https://9e0.itch.io/)
* Tilesets credits:
  * [Oak Woods — Environment Asset 🍂](https://brullov.itch.io/oak-woods) by [brullov](https://brullov.itch.io/)
  * [STRINGSTAR FIELDS](https://trixelized.itch.io/starstring-fields) by [Trixie](https://trixelized.itch.io/)
* UI sprites credits:
  * [UI books & more](https://srtoasty.itch.io/ui-assets-pack-2) by [Sr.Toasty](https://srtoasty.itch.io/)
  * [FREE UI Icons - MoonRoar Asset Series](https://soulares.itch.io/free-ui) by [Studios SoulAres](https://soulares.itch.io/)
* Enemies and boss sprite credit:
  * [Dark Fantasy Enemies](https://monopixelart.itch.io/dark-fantasy-enemies-asset-pack) and [Golems Asset Pack](https://monopixelart.itch.io/golems-pack) by [MonoPixelArt](https://monopixelart.itch.io/)
  * [Moonstone Keeper](https://sucart.itch.io/moonstone-keeper) by [Sucart](https://sucart.itch.io/)
* Items credit:
  * [16x16 RPG Item Pack](https://alexs-assets.itch.io/16x16-rpg-item-pack) by [Alex's Assets](https://alexs-assets.itch.io/)
  * [Gems / Coins Free](https://laredgames.itch.io/gems-coins-free) by [La Red Games](https://laredgames.itch.io/)
* Music and sound credits:
  * [FREE - Character Footsteps (Dirt & Grass) (80 sounds effects)](https://nebula-audio.itch.io/character-footsteps-rock-grass-pack-1) by [nebula audio](https://nebula-audio.itch.io/)
  * [Fantasy Sound Effects](https://ivyism.itch.io/fantasy-pack) by [ivyism](https://ivyism.itch.io/)
  * [Fantasy UI Sound Effects](https://ateliermagicae.itch.io/fantasy-ui-sound-effects) by [Atelier Magicae](https://ateliermagicae.itch.io/)
  * [Free Fantasy 200 SFX Pack](https://tommusic.itch.io/free-fantasy-200-sfx-pack) by [TomMusic](https://tommusic.itch.io/)
  * [Music Loop Bundle](https://tallbeard.itch.io/music-loop-bundle) by [Abstraction](https://abstractionmusic.com/)
* Additional Credits:
  * Checkpoints: [Cauldron / cooking pot on a fire](https://kaiowoka.itch.io/cauldron-cooking-pot-on-a-fire) by [kaiowoka](https://kaiowoka.itch.io/)
  * Attacks: [Thunder Spell Effect 01](https://pimen.itch.io/thunder-spell-effect-01) by [pimen](https://pimen.itch.io/)
  * Menu background: [Forest stage](https://qdanp.itch.io/forest-stage-escenario-bosque) by [qDANp](https://qdanp.itch.io/)
  * Fonts:
    * [DungeonFont](https://vrtxrry.itch.io/dungeonfont) by [vrtxrry](https://vrtxrry.itch.io/)
    * [at01 Pixel Font](https://grafxkid.itch.io/at01) by [GrafxKid](https://grafxkid.itch.io/)

## GAME
---
This platform game is themed after a witch world. Explore two levels as a nice witch, collecting different items, like gems and power-ups, and use your attack abilities to defeat enemies- a ground based one and a flying one. Make your way through the levels to defeat a boss to finish the game.

[Last release](https://github.com/LaraGuevara/Platform-game/releases/download/v.0.2/PixelPioneers-Platformer-Beta.zip)


## GAMEPLAY KEYS
---
* A move left
* D move right
* Q attack
* SPACE jump
* ESC open settings menu

## GOD MODE KEYS
---
* W up
* A left
* S down
* D right

### DEBUG FUNCTION KEYS
---
* H Shows / hides a help menu showing the keys
* F1 Set player at the start of level 1
* F2 Set player at the start of level 2
* F7 Teleport between activated checkpoints
* F8 View GUI button bound rectangles and state in different colors
* F9 Visualize the colliders / logic
* F10 God Mode (fly around, cannot be killed)
* F11 Enable/Disable FPS cap to 30
* F5 Save game
* F6 Load game
* Additional testing keys:
  * I --> instanly beat current level
  * O --> automatically cause 1 damage to player

## FEATURES
---
* Player with animations and colliders
* Object with collision
* Diverse map
* Plataforms with dynamic created colliders
* Checkpoints with automatic save (Based on sensors)
* Checkpoint teleportation
* Saving and loading on command
* Flying enemy with pathfinding
* Ground enemy with pathfinding
* Killing enemies with an attack
* Boss fight
* Items
  * Health boost items
  * Power up items (adds a temporary attack boost with a different animation)
  * "Points" item (gems)
* Health and power bar
* Level time and best time
* Item counter UI
  * Gem counter
  * Icon when active power up
* UI animations
* Intro and main menu
* Settings
* Death and win screens
* Audio and fx sounds

## OPTIMIZATIONS
---
* Frustum culling
* Enemy lazy updates
* Code instrumented with [Tracy](https://github.com/wolfpld/tracy)

## FIXES
---
* More game aspects are considered in game save, such as active checkpoints and alive enemy positions
* List of alive enemies is properly deleted and rewritten in XML file when saving the game, avoiding repeated enemies in the list
* Added life bar animations to make change more visible for player
