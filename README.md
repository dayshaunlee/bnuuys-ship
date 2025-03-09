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
