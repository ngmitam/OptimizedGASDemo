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

-   **Modular GAS Framework**: Extensible abilities and attribute sets (see `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes`)
-   **Gameplay Variants**: Combat, Platforming, and Side-Scrolling setups
-   **Performance Optimized**: Efficient damage tracing and event handling
-   **Example Implementations**: Ready-to-use abilities and character controllers

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
    -   `Variant_Combat/Gameplay/`: Abilities, attributes, and combat logic
    -   `*Character*`, `*PlayerController*`: Example characters and controllers
-   `Content/`: Blueprints, assets, and levels
-   `docs/`: Detailed GAS documentation (`GAS_OVERVIEW.md`)
-   `Binaries/`: Compiled executables
-   `Plugins/`: Enabled plugins (e.g., GameplayAbilities, ModularGameplay)

## Gameplay Ability System (GAS)

For a comprehensive guide to GAS in this project, including key classes, usage examples, and extension tips, refer to [`docs/GAS_OVERVIEW.md`](docs/GAS_OVERVIEW.md).

### Quick Tips

-   Enable `GameplayAbilities` and `ModularGameplay` plugins in your project settings.
-   Use AttributeSets for health, mana, and custom stats.
-   Implement abilities as subclasses of `UGameplayAbility`.

### Replication and Prediction Best-Practices

This project follows Lyra-inspired standards for multiplayer safety:

-   **Authority Checks**: Use `HasAuthority()` guards for server-only operations like ability granting and attribute initialization.
-   **Replication Conditions**: UI attributes (e.g., Health, MaxHealth) use `COND_OwnerOnly` to optimize bandwidth; gameplay attributes use `COND_None`.
-   **Net Execution Policies**: Passive abilities use `ServerOnly` to prevent client-side inconsistencies; predicted abilities use `LocalPredicted`.
-   **Error Handling**: Log failures in data loading and ability activation for debugging.

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
Email: ngmitamit@gmai.com

---

_This project demonstrates professional Unreal Engine development practices and serves as part of a portfolio showcasing GAS expertise._
