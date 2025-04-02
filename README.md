# Bnuuy's Ship.
Team 05 - Brian Moniaga, Chinmay Bhansali, Clare Pan, Dayshaun Lee, Lily Zhang

# M1

### Smooth Interpolation Implementation

We did an interpolation for the player's healthbar changing the color from green to red as the value (Player's HP) decreases using linear interpolation. https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/0f4b3a7353ff0aeb817eb1c42dc6cfe6b4b83bba/src/bnuui/buttons.cpp#L258-L267

We did a linear interpolation with changing the player's texture. By waiting after a set time, it will interpolate into the next keyframe. https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/0f4b3a7353ff0aeb817eb1c42dc6cfe6b4b83bba/src/animation_system.cpp#L141-L174

### External Integration

We used [Tileson](https://github.com/SSBMTonberry/tileson) to parse maps and create entities/components from the parsed maps.
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/9ad7381f245cf55efcf3ac0fa5efaa7e5fb74613/src/map_init.cpp#L83-L171

### Precise Collisions

We implemented precise collisions to allow for convex and non-convex shapes.
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/9ad7381f245cf55efcf3ac0fa5efaa7e5fb74613/src/physics_system.cpp#L19-L92

# M2

### Advanced Decision-making

We implemented A* searching algorithm to the enemies for the creative component. The enemies will find the shorest path from their current location to the ship's location, while avoiding island collisions.
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/77722e7b2ae66384a6abcae13dd449b4db39f287/src/ai_system.cpp#L33-L193

# M3
## Features Added
- Better Tutorial Level
- Randomized Reward System (Gacha System)
- More Levels
- Ship Expansion
- Laser Weapon
- Bubble Buff
- Build Mode
- Enemies Respawning
- Radar System
- Different Enemy Types
- Healing Module
- Obstacles (Whirlpool & Tornado)

## Creative Components

### Audio Feedback
Different game states will play audio feedbacks accordingly. Such as, when an enemy collided with the ship, or when an projectile is shot
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/81a604e2f7559eb355d66da1a2fa2dbfb267c682/include/sound_system.hpp#L1-L35

### Reloadability
Completing a level will save the game to a save file. Player may choose to continue the game from their last save (load the game from the save file)
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/81a604e2f7559eb355d66da1a2fa2dbfb267c682/include/saveload_system.hpp#L1-L51

### Basic Integrated Assets
We have wide variety of game sprites that have the same art styles (pixel style) which can be found [here](https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/tree/81a604e2f7559eb355d66da1a2fa2dbfb267c682/data/textures)

### Camera Controls.
Camera view throughout the gameplay. Things other than player, UI, ship, and modules, stays in background as ship moves
https://github.students.cs.ubc.ca/CPSC427-2024W-T2/team-05/blob/81a604e2f7559eb355d66da1a2fa2dbfb267c682/include/camera_system.hpp#L1-L25

