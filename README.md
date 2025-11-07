# OptimizedGASDemo

## Overview

OptimizedGASDemo is an Unreal Engine project created from the Third Person C++ template focused on implementing and optimizing the Gameplay Ability System (GAS) (inspired by Lyra). The goal is an extensible, performant framework for abilities, attributes, and combat mechanics.

## Table of Contents

-   Overview
-   Quick setup
-   Running the editor
-   Where to look in the code
-   Gameplay Ability System (GAS) in this project
-   Quick tips for editing/extending GAS
-   Contributing, References, Contact, License

## Project summary

A small Unreal Engine 5.7 C++ project (Third Person template) exploring a performant implementation of the Gameplay Ability System (GAS). The repository contains a modular GAS-based combat system (attributes, abilities, damage tracing, and supporting components) organized into gameplay variants (Combat, Platforming, SideScrolling).

### Key points

-   Engine: Unreal Engine 5.7 (see `OptimizedGASDemo.uproject`)
-   Language: C++ with Blueprint integrations
-   Primary focus: Gameplay Ability System, AttributeSets, and optimized combat mechanics

### Features

-   Modular GAS foundation and AttributeSets (see `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes`)
-   Example combat abilities and damage/event data
-   Variant combat setups

## Quick setup (local)

1. Clone this repository.
2. Open `OptimizedGASDemo.uproject` in Unreal Editor 5.7. If prompted, let the editor generate project files.
3. Build from your IDE or use the provided build tasks in this workspace. Example command (from UE root):

```powershell
Engine\Build\BatchFiles\Build.bat OptimizedGASDemoEditor Win64 Development "${PWD}\OptimizedGASDemo.uproject" -waitmutex
```

If you're using the workspace tasks, pick one of the existing tasks such as "OptimizedGASDemoEditor Win64 Development Build".

## Running the editor

-   After building, open the project with the editor (double-click the `.uproject` or use the Editor target). Play in Editor (PIE) to test sample levels and variants.

## Where to look in the code

-   Module entry: `Source/OptimizedGASDemo/OptimizedGASDemo.Build.cs`, `*.Target.cs`
-   Gameplay Abilities and Attributes: `Source/OptimizedGASDemo/Variant_Combat/Gameplay`
-   AttributeSet example: `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/CombatAttributeSet.*`
-   Example character and controllers: `Source/OptimizedGASDemo/*Character*`, `*PlayerController*`, `*GameMode*`

## Gameplay Ability System (GAS)

The full GAS walkthrough and implementation notes were moved to `docs/GAS_OVERVIEW.md` — see that file for key classes, usage examples, troubleshooting, and tips for extending GAS in this project.

## Contributing

-   This repo is intended for experimentation. Contributions are welcome via pull requests. Keep changes small and focused (feature or bugfix per PR). Document any architectural changes in the PR description.

## References

-   Unreal Engine Gameplay Ability System docs (Lyra): https://docs.unrealengine.com/en-US/GameplayAbilitySystem/index.html

## Contact

-   Nguyen Minh Tam — GitHub: `ngmitam` (email in repository metadata)

## License

-   MIT — see `LICENSE`

---

## Notes

-   The project enables the `GameplayAbilities` and `ModularGameplay` plugins in `OptimizedGASDemo.uproject`. Build settings and additional dependencies are declared in `Source/OptimizedGASDemo/OptimizedGASDemo.Build.cs`.

If you'd like, I can also add a short troubleshooting section (common build errors and quick fixes) or expand the Usage section with examples for running specific variants or running automated tests (if any exist).

_This project showcases professional game development practices and is part of my portfolio demonstrating Unreal Engine expertise._
