# PawnData Creation Guide for Player and Enemy

This document guides you through creating and configuring `UCombatPawnData` assets for Player and Enemy in the OptimizedGASDemo project.

> **🔗 Quick Setup**: For a complete walkthrough of setting up PawnData along with AbilitySets and InputConfig, see [COMPLETE_SETUP_GUIDE.md](COMPLETE_SETUP_GUIDE.md).

## Overview

`UCombatPawnData` is a data asset that contains all necessary information for a pawn (character) in the GAS system:

-   Basic attributes (Health, Stamina, Damage)
-   Ability Sets (abilities and effects)
-   Input Configuration (for player)

## Step 1: Create PawnData Asset

### In Unreal Editor:

1. **Open Content Browser**
2. **Right-click > Miscellaneous > Data Asset**
3. **Select UCombatPawnData**
4. **Name it** (e.g., `DA_PlayerPawnData`, `DA_EnemyPawnData`)

## Step 2: Configure Basic Attributes

### Attributes Category:

```cpp
// Default values for attributes
Default Health: 1000.0
Default Max Health: 1000.0
Default Damage: 10.0
Default Knockback Impulse: 500.0
Default Launch Impulse: 300.0
Default Stamina: 100.0
Default Max Stamina: 100.0

// Required Attribute Sets
Attribute Sets:
- UHealthAttributeSet
- UDamageAttributeSet
- UStaminaAttributeSet
- UMovementAttributeSet (for player)
```

## Step 3: Create Ability Sets

### 3.1 Create CombatAbilitySet Asset

1. **Create new Data Asset**: `UCombatAbilitySet`
2. **Name it** (e.g., `DAS_SharedCombatAbilities`, `DAS_PlayerSpecificAbilities`, `DAS_EnemySpecificAbilities`)

### 3.2 Configure Abilities

**Granted Abilities** (TArray<FGameplayAbilityBindInfo>):

-   Add required ability classes

**Granted Gameplay Effects**:

-   Add effect classes (e.g., stamina regeneration)

**Granted Attributes**:

-   Add attribute sets if needed

### 3.3 Example Ability Sets

#### Player Ability Set:

```
Granted Abilities:
- UCombatReceiveDamageAbility
- UCombatDeathAbility
- UCombatComboAttackAbility
- UCombatChargedAttackAbility
- UCombatLockToggleAbility
- UCombatStaminaRegenerationAbility

Granted Gameplay Effects:
- UCombatStaminaRegenerationGameplayEffect

Granted Attributes:
- (usually empty, attributes are set in PawnData)
```

#### Enemy Ability Set:

```
Granted Abilities:
- UCombatReceiveDamageAbility
- UCombatDeathAbility
- UCombatComboAttackAbility
- UCombatChargedAttackAbility
- UCombatLockableAbility
- UCombatStaminaRegenerationAbility
- UCombatTraceAttackAbility

Granted Gameplay Effects:
- (can be empty)

Granted Attributes:
- (usually empty)
```

## Step 4: Configure Input (Player Only)

### 4.1 Create InputConfig Asset

1. **Create new Data Asset**: `UCombatInputConfig`
2. **Name it** (e.g., `DA_PlayerInputConfig`)

### 4.2 Configure Input Mappings

**Input Mappings** (TArray<FCombatInputMapping>):

```cpp
// Example for Combo Attack
Input Action: IA_ComboAttack (from project settings)
Ability Triggers:
- Trigger Tag: Event.Attack.Combo.Start
- Trigger Event: Started

// Example for Charged Attack
Input Action: IA_ChargedAttack
Ability Triggers:
- Trigger Tag: Event.Attack.Charged.Start
- Trigger Event: Started
- Trigger Tag: Event.Attack.Charged.Release
- Trigger Event: Completed
```

### 4.3 Assign InputConfig to PawnData

In **Player PawnData**:

```
Input Config: DA_PlayerInputConfig
```

## Step 5: Configure Ability Sets

### In PawnData:

**Ability Sets** (TArray<FCombatAbilitySetWithInput>):

```cpp
// Player PawnData
Ability Sets:
- Ability Set: DAS_SharedCombatAbilities
- Ability Set: DAS_PlayerSpecificAbilities

// Enemy PawnData
Ability Sets:
- Ability Set: DAS_SharedCombatAbilities
- Ability Set: DAS_EnemySpecificAbilities
```

**Note**: Ability Sets are used to grant abilities and effects to the pawn. Input binding is handled separately via the InputConfig asset (for players only).

## Step 6: Using PawnData

### 6.1 For Player Character

In `ACombatCharacter`:

-   PawnData is created in `ACombatBase::ACombatBase()`
-   Ability Sets are granted in `ACombatPlayerState::SetPawnData()`

### 6.2 For Enemy Character

In `ACombatEnemy`:

-   PawnData is created similarly
-   Ability Sets are granted in `ACombatEnemy::BeginPlay()`

## Complete Examples

### Player PawnData (DA_PlayerPawnData):

```
Attributes:
- Default Health: 1000
- Default Max Health: 1000
- Default Damage: 15
- Default Stamina: 100
- Default Max Stamina: 100
- Attribute Sets: [UHealthAttributeSet, UDamageAttributeSet, UStaminaAttributeSet, UMovementAttributeSet]

Abilities:
- Ability Sets:
  - Ability Set: DAS_SharedCombatAbilities
  - Ability Set: DAS_PlayerSpecificAbilities

Input:
- Input Config: DA_PlayerInputConfig
```

### Enemy PawnData (DA_EnemyPawnData):

```
Attributes:
- Default Health: 500
- Default Max Health: 500
- Default Damage: 8
- Default Stamina: 50
- Default Max Stamina: 50
- Attribute Sets: [UHealthAttributeSet, UDamageAttributeSet, UStaminaAttributeSet]

Abilities:
- Ability Sets:
  - Ability Set: DAS_SharedCombatAbilities
  - Ability Set: DAS_EnemySpecificAbilities
```

## Troubleshooting

### Abilities Not Being Granted:

-   Check if AbilitySet is properly referenced in PawnData
-   Ensure AbilitySystemComponent is initialized in PlayerState

### Input Not Working:

-   Check if InputConfig is set in PawnData
-   Ensure Input Actions are configured in Project Settings > Engine > Input

### Attributes Not Being Set:

-   Check if Attribute Sets are added to PawnData
-   Ensure AbilitySystemComponent has Granted Attribute Sets
