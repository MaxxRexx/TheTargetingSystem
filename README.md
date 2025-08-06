# TheTargetingSystem
FAB (Unreal Marketplace) Listing: https://www.fab.com/listings/2485519f-b6d7-494a-9f48-5bc92a575129


The Targeting System is a complete solution for customized, directional targeting in UE4, and is completely networked and multiplayer ready.﻿  Designed from the ground up for ease of use, all of its functionality is detailed in a single, plug and play component.

It uses the AI Perception System to perceive targets, and sorts the targets onto 3 categories:
- the most ideal actor to target immediately;
- actors to the left of the targeted actor;
- and actors to the right of the targeted actor;
thus enabling you to switch targets clockwise and anti-clockwise: to the left and right.

There are several options for customization available to the user and exposed on the instance of the component for easy, quick prototyping.

## Technical Details

### Features:
- Multiplayer Ready;
- Use Actor Rotation/Control Rotation;
- Error Tolerance For Sorting Targets To The Left And Right;
- Use Delta Rotation Or Arc-Tan Difference Algorithms;
- Auto-Switch Targets On Perception Failure?;
- Auto-Switch Targets If "Actor To Target Is Invalid"?
- Show Widget For Lock-On?;
- Filter Perceived Actors To Specific Class?;
- Can Lock-On To Target?;
- Loop Timer For Storing Possible Targets?;
- Camera Interpolation Speed;
- Sense To Use For Perception;
- Initialize The Targeting System?;
- Delay Before Lock-On To Target?/Delay Amount;

### Code Modules
- TheTargetingSystem - Runtime

### Number of Blueprints: 4

- One Example Game Mode
- Default Third Person Animation Blueprint
- Default Third Person Character Blueprint (Child of TS_Character)
- One Example Widget

### Inputs
- Example Configuration Included (Keyboard):
  - Press T To Target
  - Press Q To Target Left
  - Press E To Target Right

### Multiplayer and Platforms
- Network Replicated: (Yes)
- Supported Development Platforms: (PC)
- Supported Target Build Platforms: (PC)

### Contact
- You can reach me personally on:
  - Discord: ragnar9805
  - Email: contact@deuxniistudios.com
