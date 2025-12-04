# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-12-05

### Added

-   **Combat Variant Implementation**: Full implementation of combat gameplay variant with Gameplay Ability System (GAS)

    -   Added abilities: TraceAttack, Combo, ChargedAttack, ReceiveDamage, Death, StaminaRegeneration
    -   Added attribute sets: Health, Damage, Stamina, Movement
    -   Added gameplay effects and executions for damage calculation and stamina management
    -   Implemented pawn data and input configurations for combat mechanics
    -   Added comprehensive documentation in `docs/` folder

-   **Stamina System**: Comprehensive stamina mechanics
    -   Stamina regeneration using Gameplay Effect Execution
    -   Stamina costs for abilities and stun effects
    -   Proper replication for multiplayer support

### Changed

-   **Attribute System Refactor**: Split `UCombatAttributeSet` into specialized attribute sets

    -   `UHealthAttributeSet`: Manages health-related attributes (Health, MaxHealth)
    -   `UDamageAttributeSet`: Handles damage calculations and multipliers
    -   `UStaminaAttributeSet`: Manages stamina attributes (Stamina, MaxStamina, StaminaUsed)
    -   `UMovementAttributeSet`: Controls movement-related attributes

-   **Player Controller Classes**: Refactored for better code quality

    -   Updated copyright notices
    -   Standardized code formatting
    -   Added missing `Super::SetupInputComponent()` calls

-   **Character Movement and Combat**: Enhanced systems

    -   Improved trace attacks and combo mechanics
    -   Better charged attack implementations
    -   Optimized damage tracing and event handling

-   **Multiplayer Safety**: Enhanced GAS for multiplayer

    -   Proper replication conditions (COND_OwnerOnly for UI attributes, COND_None for gameplay)
    -   Authority checks and net execution policies
    -   GrantedHandles pattern for ability management

-   **README.md**: Improved structure, clarity, and professionalism
    -   Added table of contents
    -   Detailed feature descriptions
    -   Better documentation links

### Technical Details

-   **New C++ Classes Added**:

    -   **Abilities**: `UCombatGameplayAbility`, `UCombatTraceAttackAbility`, `UCombatComboAttackAbility`, `UCombatChargedAttackAbility`, `UCombatReceiveDamageAbility`, `UCombatDeathAbility`, `UCombatStaminaRegenerationAbility`, `UCombatLockToggleAbility`, `UCombatLockableAbility`
    -   **Attribute Sets**: `UHealthAttributeSet`, `UDamageAttributeSet`, `UStaminaAttributeSet`, `UMovementAttributeSet`
    -   **Gameplay Effects**: `UCombatDamageGameplayEffect`, `UCombatStaminaCostGameplayEffect`, `UCombatStaminaRegenerationGameplayEffect`, `UCombatStaminaStunGameplayEffect`, `UCombatStaminaUsedGameplayEffect`, `UCombatStaminaUsedResetGameplayEffect`
    -   **Executions**: `UCombatExecutionCalculation`, `UCombatStaminaRegenerationExecution`
    -   **Data Assets**: `UCombatAbilitySet`, `UCombatInputConfig`, `UCombatPawnData`
    -   **Components**: `UCombatHealthComponent`, `UCombatStaminaComponent`, `UCombatLockSystemComponent`
    -   **Characters/Controllers**: `ACombatCharacter`, `ACombatPlayerController`, `ACombatPlayerState`, `ACombatEnemy`, `ACombatBase`, `ACombatGameMode`

-   **Key Files Modified**:

    -   `Source/OptimizedGASDemo/Variant_Combat/CombatPlayerController.cpp/h`: Added input setup and ability management
    -   `Source/OptimizedGASDemo/Variant_Platforming/PlatformingPlayerController.cpp`: Standardized formatting and added missing calls
    -   `Source/OptimizedGASDemo/Variant_SideScrolling/SideScrollingPlayerController.cpp`: Standardized formatting and added missing calls
    -   `README.md`: Restructured with table of contents and detailed sections

-   **Assets Added**:

    -   `Content/Variant_Combat/Data/AbilitySets/`: `DAS_SharedCombatAbilities.uasset`, `DAS_PlayerSpecificAbilities.uasset`, `DAS_EnemySpecificAbilities.uasset`
    -   `Content/Variant_Combat/Data/`: `DA_PlayerPawnData.uasset`, `DA_EnemyPawnData.uasset`, `DA_PlayerInputConfig.uasset`
    -   `Content/Variant_Combat/Blueprints/`: `BP_CombatCharacter.uasset`, `BP_CombatEnemy.uasset`
    -   `Content/Variant_Combat/UI/`: `UI_LifeBar.uasset`

-   **Documentation Updates**:

    -   `docs/GAS_OVERVIEW.md`: Added examples for new abilities and attribute sets
    -   `docs/COMPLETE_SETUP_GUIDE.md`: New guide for setting up AbilitySets, InputConfig, and PawnData
    -   `docs/ABILITYSET_GUIDE.md`: Detailed instructions for AbilitySet assets
    -   `docs/PAWNDATA_GUIDE.md`: Guide for PawnData configuration
    -   `docs/INPUTCONFIG_GUIDE.md`: Instructions for InputConfig setup
    -   `docs/LYRA_COMPLIANCE.md`: Updated with new implemented features
    -   `docs/GAS_SEQUENCE.md`: Enhanced sequence diagrams

-   **Configuration Changes**:
    -   `Config/DefaultGameplayTags.ini`: Added new gameplay tags for combat mechanics

## [0.1.0] - 2025-11-27

Initial release with basic GAS framework setup.
