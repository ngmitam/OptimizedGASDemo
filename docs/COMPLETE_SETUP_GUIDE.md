# Complete Setup Guide: AbilitySets, InputConfig, and PawnData

This guide provides step-by-step instructions to create and configure `UCombatAbilitySet`, `UCombatInputConfig`, and `UCombatPawnData` assets for Player and Enemy characters in the OptimizedGASDemo project.

> **📚 Related Documentation**:
>
> -   [ABILITYSET_GUIDE.md](ABILITYSET_GUIDE.md) - Detailed AbilitySet creation and configuration
> -   [INPUTCONFIG_GUIDE.md](INPUTCONFIG_GUIDE.md) - Comprehensive InputConfig setup and input mapping
> -   [PAWNDATA_GUIDE.md](PAWNDATA_GUIDE.md) - Complete PawnData configuration guide

## Prerequisites

Before starting, ensure you have:

1. **Unreal Engine 5.7** installed and the project open
2. **Input Actions** configured in Project Settings > Engine > Input:
    - `IA_ComboAttack`
    - `IA_ChargedAttack`
    - `IA_LockTarget`
    - `IA_Move`
    - `IA_Look`
    - `IA_Jump`
3. **Gameplay Tags** defined in DefaultGameplayTags.ini:
    - `Event.Attack.Combo.Start`
    - `Event.Attack.Charged.Start`
    - `Event.Attack.Charged.Release`
    - `Event.Target.Lock.Toggle`

## Overview

This guide covers the complete setup process in three main parts:

1. **Part 1: Create AbilitySets** → [ABILITYSET_GUIDE.md](ABILITYSET_GUIDE.md) for details

    - DAS_SharedCombatAbilities (shared between player/enemy)
    - DAS_PlayerSpecificAbilities (player-only abilities)
    - DAS_EnemySpecificAbilities (enemy-only abilities)

2. **Part 2: Create InputConfig** → [INPUTCONFIG_GUIDE.md](INPUTCONFIG_GUIDE.md) for details

    - Player input mappings and ability triggers
    - Input action to gameplay tag mapping

3. **Part 3: Create PawnData** → [PAWNDATA_GUIDE.md](PAWNDATA_GUIDE.md) for details
    - Player and enemy attribute configuration
    - Ability set assignment and input config linking

## Part 1: Create AbilitySets

> **📖 Detailed Guide**: For comprehensive information about AbilitySet creation, configuration, and best practices, see [ABILITYSET_GUIDE.md](ABILITYSET_GUIDE.md).

### Step 1.1: Create Shared AbilitySet

1. **Open Content Browser** in Unreal Editor
2. **Right-click > Miscellaneous > Data Asset**
3. **Select UCombatAbilitySet**
4. **Name it**: `DAS_SharedCombatAbilities`
5. **Open the asset** by double-clicking

#### Configure Granted Abilities:

-   Click **+** next to Granted Abilities
-   Add the following shared abilities:
    -   `UCombatReceiveDamageAbility` (Command: Ability1)
    -   `UCombatDeathAbility` (Command: Ability2)
    -   `UCombatComboAttackAbility` (Command: Ability3)
    -   `UCombatChargedAttackAbility` (Command: Ability4)
    -   `UCombatStaminaRegenerationAbility` (Command: Ability5)
    -   `UCombatTraceAttackAbility` (Command: Ability6)
-   For each ability:
    -   **Command**: Set to the corresponding Ability enum

**Note**: This shared ability set contains abilities common to both players and enemies.

#### Granted Attributes:

-   Leave empty (attributes are handled in PawnData)

6. **Save** the asset

### Step 1.2: Create Player-Specific AbilitySet

1. **Right-click > Miscellaneous > Data Asset**
2. **Select UCombatAbilitySet**
3. **Name it**: `DAS_PlayerSpecificAbilities`
4. **Open the asset**

#### Configure Granted Abilities:

-   Add player-specific abilities:
    -   `UCombatLockToggleAbility` (Command: Ability7)
-   For each ability:
    -   **Command**: Set to the corresponding Ability enum

**Note**: Contains abilities unique to players (like camera lock toggle).

#### Granted Attributes:

-   Leave empty

5. **Save** the asset

### Step 1.3: Create Enemy-Specific AbilitySet

1. **Right-click > Miscellaneous > Data Asset**
2. **Select UCombatAbilitySet**
3. **Name it**: `DAS_EnemySpecificAbilities`
4. **Open the asset**

#### Configure Granted Abilities:

-   Add enemy-specific abilities:
    -   `UCombatLockableAbility` (Command: Ability7)
-   For each ability:
    -   **Command**: Set to the corresponding Ability enum

**Note**: Contains abilities unique to enemies (like being lockable by players).

#### Granted Attributes:

-   Leave empty

5. **Save** the asset

## Part 2: Create InputConfig (Player Only)

> **📖 Detailed Guide**: For comprehensive information about InputConfig creation, input mappings, and trigger configuration, see [INPUTCONFIG_GUIDE.md](INPUTCONFIG_GUIDE.md).

### Step 2.1: Create InputConfig Asset

1. **Right-click > Miscellaneous > Data Asset**
2. **Select UCombatInputConfig**
3. **Name it**: `DA_PlayerInputConfig`
4. **Open the asset**

### Step 2.2: Configure Input Mappings

#### Add Combo Attack Mapping:

-   Click **+** next to Input Mappings
-   **Input Action**: Select `IA_ComboAttack`
-   **Consume Input**: true
-   Click **+** next to Ability Triggers
-   **Trigger Tag**: `Event.Attack.Combo.Start`
-   **Trigger Event**: Started
-   **Trigger Once**: false

#### Add Charged Attack Mapping:

-   Click **+** next to Input Mappings
-   **Input Action**: Select `IA_ChargedAttack`
-   **Consume Input**: true
-   Add two Ability Triggers:
    -   First: **Trigger Tag**: `Event.Attack.Charged.Start`, **Trigger Event**: Started, **Trigger Once**: false
    -   Second: **Trigger Tag**: `Event.Attack.Charged.Release`, **Trigger Event**: Completed, **Trigger Once**: false

#### Add Lock Target Mapping:

-   Click **+** next to Input Mappings
-   **Input Action**: Select `IA_LockTarget`
-   **Consume Input**: true
-   Add Ability Trigger:
    -   **Trigger Tag**: `Event.Target.Lock.Toggle`, **Trigger Event**: Started, **Trigger Once**: false

#### Add Movement Mappings (Optional, for completeness):

-   **Input Action**: `IA_Move`, **Consume Input**: false, no triggers
-   **Input Action**: `IA_Look`, **Consume Input**: false, no triggers
-   **Input Action**: `IA_Jump`, **Consume Input**: false, no triggers

5. **Save** the asset

## Part 3: Create PawnData

> **📖 Detailed Guide**: For comprehensive information about PawnData creation, attribute configuration, and ability set assignment, see [PAWNDATA_GUIDE.md](PAWNDATA_GUIDE.md).

### Step 3.1: Create Player PawnData

1. **Right-click > Miscellaneous > Data Asset**
2. **Select UCombatPawnData**
3. **Name it**: `DA_PlayerPawnData`
4. **Open the asset**

#### Configure Attributes:

-   **Default Health**: 1000.0
-   **Default Max Health**: 1000.0
-   **Default Damage**: 15.0
-   **Default Stamina**: 100.0
-   **Default Max Stamina**: 100.0
-   **Attribute Sets**: Add `UHealthAttributeSet`, `UDamageAttributeSet`, `UStaminaAttributeSet`, `UMovementAttributeSet`

#### Configure Ability Sets:

-   Click **+** next to Ability Sets
-   **Ability Set**: Select `DAS_SharedCombatAbilities`
-   Click **+** next to Ability Sets
-   **Ability Set**: Select `DAS_PlayerSpecificAbilities`

#### Configure Input:

-   **Input Config**: Select `DA_PlayerInputConfig`

5. **Save** the asset

### Step 3.2: Create Enemy PawnData

1. **Right-click > Miscellaneous > Data Asset**
2. **Select UCombatPawnData**
3. **Name it**: `DA_EnemyPawnData`
4. **Open the asset**

#### Configure Attributes:

-   **Default Health**: 500.0
-   **Default Max Health**: 500.0
-   **Default Damage**: 8.0
-   **Default Stamina**: 50.0
-   **Default Max Stamina**: 50.0
-   **Attribute Sets**: Add `UHealthAttributeSet`, `UDamageAttributeSet`, `UStaminaAttributeSet`

#### Configure Ability Sets:

-   Click **+** next to Ability Sets
-   **Ability Set**: Select `DAS_SharedCombatAbilities`
-   Click **+** next to Ability Sets
-   **Ability Set**: Select `DAS_EnemySpecificAbilities`

#### Input Config:

-   Leave empty (enemies don't use input)

5. **Save** the asset

## Part 4: Integration and Testing

### Step 4.1: Assign PawnData to Characters

#### For Player:

-   In `ACombatCharacter` (or its derived class), ensure `PawnData` is set to `DA_PlayerPawnData`
-   This is typically done in the character's constructor or via editor properties

#### For Enemy:

-   In `ACombatEnemy`, set `PawnData` to `DA_EnemyPawnData`

### Step 4.2: Test in Editor

1. **Play in Editor** (PIE)
2. **Test Player Abilities**:
    - Press combo attack button → Should trigger combo attack
    - Hold charged attack → Should charge and release
    - Press lock button → Should toggle camera lock
3. **Test Enemy Abilities**:
    - Spawn enemy → Should have basic combat abilities
    - Attack enemy → Should react and die appropriately
    - Enemy should have stamina regeneration like player

### Step 4.3: Common Errors and Fixes

#### Error: "Ability not found" or "Tag not recognized"

-   **Cause**: Missing gameplay tags or incorrect tag names
-   **Fix**: Check DefaultGameplayTags.ini and ensure tags match exactly

#### Error: "Input action not bound"

-   **Cause**: Input actions not configured in Project Settings
-   **Fix**: Go to Project Settings > Engine > Input and add missing actions

#### Error: "Attribute set not granted"

-   **Cause**: Attribute sets not added to PawnData
-   **Fix**: Add required attribute sets in PawnData > Attribute Sets

#### Error: "Effect not applying"

-   **Cause**: Invalid effect class or missing modifiers
-   **Fix**: Check the GameplayEffect asset for proper stat modifiers

#### Error: Build fails with "Undeclared identifier"

-   **Cause**: Missing includes or incorrect class names
-   **Fix**: Ensure all ability/effect classes exist and are properly included

## Best Practices

1. **Naming Convention**: Use `DAS_` for AbilitySets, `DA_` for Data Assets
2. **Modular Design**: Keep AbilitySets focused on specific roles
3. **Testing**: Test each asset individually before integration
4. **Documentation**: Comment complex configurations
5. **Version Control**: Commit assets separately for easier tracking

If you encounter errors not covered here, check the individual guide documents (ABILITYSET_GUIDE.md, INPUTCONFIG_GUIDE.md, PAWNDATA_GUIDE.md) or provide error details for guide updates.
