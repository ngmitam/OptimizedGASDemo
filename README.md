# OptimizedGASDemo

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue.svg)](https://www.unrealengine.com/)

An optimized Unreal Engine 5.7 project demonstrating a performant implementation of the Gameplay Ability System (GAS), inspired by Lyra. This C++-based framework provides extensible abilities, attributes, and combat mechanics.

## Table of Contents

-   [Overview](#overview)
-   [Features](#features)
-   [Quick Start](#quick-start)
-   [Usage](#usage)
-   [Project Structure](#project-structure)
-   [Gameplay Ability System (GAS)](#gameplay-ability-system-gas)
-   [Contributing](#contributing)
-   [License](#license)
-   [Contact](#contact)

## Overview

OptimizedGASDemo is built from the Third Person C++ template, focusing on creating an efficient and scalable GAS foundation. It includes modular combat systems, attribute sets, and example abilities, making it ideal for prototyping action-oriented games.

-   **Engine Version**: Unreal Engine 5.7
-   **Language**: C++ with Blueprint support
-   **Primary Focus**: GAS optimization, attributes, and combat mechanics

## Features

-   **Modular GAS Framework**: Extensible abilities and attribute sets (see `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes` and `Abilities`)
-   **Gameplay Variants**:
    -   **Combat**: Action-oriented combat with trace attacks, combos, charged attacks, stamina system, and damage systems
-   **Performance Optimized**: Efficient damage tracing and event handling
-   **Example Implementations**: Ready-to-use abilities and character controllers
-   **Multiplayer Ready**: Proper replication for attributes and abilities

### Lyra Compliance Status

#### ✅ Implemented (Lyra-compliant)

-   **GAS Architecture**: AbilitySystemComponent on PlayerState, separate AttributeSets (Health, Damage, Stamina, Movement), PawnData-driven ability granting with AbilitySets
-   **Replication**: Proper attribute replication with COND_OwnerOnly for UI attributes (Health, MaxHealth, Stamina, MaxStamina, StaminaUsed), COND_None for gameplay attributes
-   **Enhanced Input**: Modern input system with Input Actions and Mapping Contexts integrated with AbilitySets
-   **GrantedHandles Pattern**: Lyra-style ability/effect management with proper cleanup during hot-reloading and pawn data changes
-   **Modern UE API**: Updated GameplayEffect components to use current UE 5.7 standards (no deprecated warnings)
-   **AbilitySet Assets**: UCombatAbilitySet data assets implemented to replace legacy pawn data arrays
-   **PawnData Migration**: Completed migration from GrantedAbilities/GrantedEffects arrays to AbilitySets with input mapping
-   **InputConfig**: Basic InputConfig system implemented for centralized input configuration (UCombatInputConfig)
-   **Stamina System**: Comprehensive stamina mechanics with regeneration, costs, and stun effects
-   **Attribute System Refactor**: Split UCombatAttributeSet into specialized UHealthAttributeSet, UDamageAttributeSet, UStaminaAttributeSet, and UMovementAttributeSet

#### 🚧 In Progress / Planned

-   **Experience System**: ExperienceDefinition and experience progression system for leveling
-   **CameraMode**: Dynamic camera mode switching system (e.g., UCombatCameraMode_ThirdPerson, UCombatCameraMode_Aim)
-   **HUD Management**: Proper HUD layout and management system with widget switching
-   **Pawn Extension System**: Lyra-style pawn extension components for modular pawn functionality
-   **Controller Extension System**: Controller extension components for input handling
-   **Player Spawning System**: Proper player spawning with pawn data application
-   **Game Feature System**: Modular game features with activation/deactivation

For detailed roadmap and implementation guidance, see [`docs/LYRA_COMPLIANCE.md`](docs/LYRA_COMPLIANCE.md).

## Quick Start

### Prerequisites

-   Unreal Engine 5.7 installed
-   Visual Studio or compatible IDE for C++ development

### Setup

1. Clone the repository:
    ```bash
    git clone https://github.com/ngmitam/OptimizedGASDemo.git
    ```
2. Open `OptimizedGASDemo.uproject` in Unreal Editor 5.7. Generate project files if prompted.
3. Build the project using one of the provided tasks (e.g., "OptimizedGASDemoEditor Win64 Development Build") or via command line:
    ```powershell
    Engine\Build\BatchFiles\Build.bat OptimizedGASDemoEditor Win64 Development "C:\Path\To\OptimizedGASDemo.uproject" -waitmutex
    ```

### Running

-   Launch the editor and use Play in Editor (PIE) to test levels and variants.

## Usage

-   **Building**: Use the workspace tasks for different configurations (Debug, Development, Shipping).
-   **Editing**: Modify abilities in `Source/OptimizedGASDemo/Variant_Combat/Gameplay`.
-   **Testing**: Run PIE or package for standalone testing.

## Project Structure

-   `Source/OptimizedGASDemo/`: Core C++ modules and GAS implementations
    -   `Variant_Combat/Gameplay/`: Abilities, attributes, and combat logic (Health, Damage, Stamina, Movement AttributeSets; TraceAttack, Combo, Charged, ReceiveDamage, Death, StaminaRegeneration, etc. abilities)
    -   `Variant_Platforming/`: Platforming-specific gameplay mechanics
    -   `Variant_SideScrolling/`: Side-scrolling gameplay variant
    -   `*Character*`, `*PlayerController*`, `*PlayerState*`: Example characters, controllers, and player states for each variant
-   `Content/`: Blueprints, assets, and levels
    -   `Variant_Combat/`: Combat gameplay assets, animations, UI, VFX
    -   `Variant_Platforming/`: Platforming assets
    -   `Variant_SideScrolling/`: Side-scrolling assets
-   `docs/`: Detailed GAS documentation (`GAS_OVERVIEW.md`, `GAS_SEQUENCE.md`, `COMPLETE_SETUP_GUIDE.md`, `ABILITYSET_GUIDE.md`, `PAWNDATA_GUIDE.md`, `INPUTCONFIG_GUIDE.md`, `LYRA_COMPLIANCE.md`)
-   `Binaries/`: Compiled executables
-   `Plugins/`: Enabled plugins (e.g., GameplayAbilities, ModularGameplay)

## Gameplay Ability System (GAS)

For a comprehensive guide to GAS in this project, including key classes, usage examples, and extension tips, refer to [`docs/GAS_OVERVIEW.md`](docs/GAS_OVERVIEW.md).

For step-by-step setup instructions for creating AbilitySets, InputConfig, and PawnData assets for players and enemies, see [`docs/COMPLETE_SETUP_GUIDE.md`](docs/COMPLETE_SETUP_GUIDE.md).

For detailed instructions on creating and configuring AbilitySet assets, see [`docs/ABILITYSET_GUIDE.md`](docs/ABILITYSET_GUIDE.md).

For detailed instructions on creating and configuring PawnData assets for players and enemies, see [`docs/PAWNDATA_GUIDE.md`](docs/PAWNDATA_GUIDE.md).

For detailed instructions on creating and configuring InputConfig assets for ability input mapping, see [`docs/INPUTCONFIG_GUIDE.md`](docs/INPUTCONFIG_GUIDE.md).

### Quick Tips

-   Enable `GameplayAbilities` and `ModularGameplay` plugins in your project settings.
-   Use AttributeSets for health, mana, stamina, damage, movement, and custom stats.
-   Implement abilities as subclasses of `UGameplayAbility`. Check `Variant_Combat/Gameplay/Abilities/` for examples.
-   For multiplayer, ensure proper replication conditions (OwnerOnly for UI attributes, None for gameplay).

### Replication and Prediction Best-Practices

This project follows Lyra-inspired standards for multiplayer safety:

-   **Authority Checks**: Use `HasAuthority()` guards for server-only operations like ability granting and attribute initialization.
-   **Replication Conditions**: UI attributes (e.g., Health, MaxHealth) use `COND_OwnerOnly` to optimize bandwidth; gameplay attributes use `COND_None`.
-   **Net Execution Policies**: Passive abilities use `ServerOnly` to prevent client-side inconsistencies; predicted abilities use `LocalPredicted`.
-   **Error Handling**: Log failures in data loading and ability activation for debugging.
-   **GrantedHandles Pattern**: Uses Lyra-style ability/effect management for proper cleanup during hot-reloading and pawn data changes.

## Contributing

This repository is for experimentation and learning. Contributions are welcome!

-   Fork the repo and submit pull requests.
-   Keep changes focused (one feature or bugfix per PR).
-   Document architectural changes in PR descriptions.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

Nguyen Minh Tam  
GitHub: [@ngmitam](https://github.com/ngmitam)  
Email: ngmitamit@gmail.com

---

_This project demonstrates professional Unreal Engine development practices and serves as part of a portfolio showcasing GAS expertise._
