# AbilitySet Creation Guide

This document guides you through creating and configuring `UCombatAbilitySet` assets for the OptimizedGASDemo project.

> **🔗 Quick Setup**: For a complete walkthrough of setting up AbilitySets along with InputConfig and PawnData, see [COMPLETE_SETUP_GUIDE.md](COMPLETE_SETUP_GUIDE.md).

## Overview

`UCombatAbilitySet` is a data asset that groups related abilities, gameplay effects, and attributes that can be granted to a pawn (character) in the GAS system:

-   **Granted Abilities**: Gameplay abilities to grant to the pawn
-   **Granted Gameplay Effects**: Persistent effects to apply (e.g., stat modifications, regeneration)
-   **Granted Attributes**: Attribute sets to grant (usually handled in PawnData)

## Step 1: Create AbilitySet Asset

### In Unreal Editor:

1. **Open Content Browser**
2. **Right-click > Miscellaneous > Data Asset**
3. **Select UCombatAbilitySet**
4. **Name it** (e.g., `DAS_SharedCombatAbilities`, `DAS_PlayerSpecificAbilities`, `DAS_EnemySpecificAbilities`)

## Step 2: Configure Granted Abilities

### Granted Abilities Category:

**Granted Abilities** (TArray<FGameplayAbilityBindInfo>):

Each entry includes:

-   **Gameplay Ability Class**: The ability class to grant
-   **Command**: The input command enum value (e.g., Ability1, Ability2) for the ability

### Example Granted Abilities:

#### Shared Combat Ability Set:

```
Granted Abilities:
- Gameplay Ability Class: UCombatReceiveDamageAbility
  Command: Ability1

- Gameplay Ability Class: UCombatDeathAbility
  Command: Ability2

- Gameplay Ability Class: UCombatComboAttackAbility
  Command: Ability3

- Gameplay Ability Class: UCombatChargedAttackAbility
  Command: Ability4

- Gameplay Ability Class: UCombatStaminaRegenerationAbility
  Command: Ability5

- Gameplay Ability Class: UCombatTraceAttackAbility
  Command: Ability6
```

#### Player-Specific Ability Set:

```
Granted Abilities:
- Gameplay Ability Class: UCombatLockToggleAbility
  Command: Ability7
```

#### Enemy-Specific Ability Set:

```
Granted Abilities:
- Gameplay Ability Class: UCombatLockableAbility
  Command: Ability7
```

## Step 3: Configure Granted Gameplay Effects

### Granted Gameplay Effects Category:

**Granted Gameplay Effects** (TArray<FGameplayEffectApplicationInfo>):

Each entry includes:

-   **Gameplay Effect Class**: The effect class to apply
-   **Removal Policy**: When to remove the effect (e.g., DoNotRemove for persistent effects)

### Example Granted Effects:

#### Player Ability Set:

```
Granted Gameplay Effects:
- Gameplay Effect Class: UCombatStaminaRegenerationGameplayEffect
  Removal Policy: DoNotRemove
```

#### Enemy Ability Set:

```
Granted Gameplay Effects:
- (usually empty for basic enemies)
```

## Step 4: Configure Granted Attributes

### Granted Attributes Category:

**Granted Attributes** (TArray<FGameplayAttributeBindInfo>):

Each entry includes:

-   **Attribute Set**: The attribute set class to grant
-   **bCreateOnStack**: Whether to create on stack (usually false)

### Note:

Attribute sets are typically configured in PawnData rather than AbilitySets. This section is usually empty.

## Step 5: Using AbilitySet

### 5.1 Assign to PawnData

In `UCombatPawnData`:

```
Ability Sets:
- Ability Set: DAS_SharedCombatAbilities
- Ability Set: DAS_PlayerSpecificAbilities
```

### 5.2 Integration with Character

When PawnData is set on a character:

-   Abilities are granted via `UAbilitySystemComponent::GiveAbility()`
-   Effects are applied via `UAbilitySystemComponent::ApplyGameplayEffectSpecToTarget()`
-   Attributes are granted if specified

## Complete Examples

### Shared Combat Ability Set (DAS_SharedCombatAbilities):

```
Granted Abilities:
- UCombatReceiveDamageAbility
- UCombatDeathAbility
- UCombatComboAttackAbility
- UCombatChargedAttackAbility
- UCombatStaminaRegenerationAbility
- UCombatTraceAttackAbility

Granted Gameplay Effects:
- (empty)

Granted Attributes:
- (empty)
```

### Player-Specific Ability Set (DAS_PlayerSpecificAbilities):

```
Granted Abilities:
- UCombatLockToggleAbility

Granted Gameplay Effects:
- (empty)

Granted Attributes:
- (empty)
```

### Enemy-Specific Ability Set (DAS_EnemySpecificAbilities):

```
Granted Abilities:
- UCombatLockableAbility

Granted Gameplay Effects:
- (empty)

Granted Attributes:
- (empty)
```

## Troubleshooting

### Abilities Not Activating:

-   Check if ability classes are properly implemented
-   Ensure trigger tags are correctly set in ability specs
-   Verify input mapping in InputConfig (for players)

### Effects Not Applying:

-   Check if effect classes have valid modifiers
-   Ensure effect duration and stacking rules are correct
-   Verify Removal Policy is set appropriately for the effect's purpose

### Attributes Not Granted:

-   Attributes are usually handled in PawnData Attribute Sets
-   Only use this for special cases requiring ability-specific attributes

## Best Practices

1. **Logical Grouping**: Group related abilities together (e.g., combat abilities, movement abilities)
2. **Minimal Sets**: Keep AbilitySets focused on specific roles/behaviors
3. **Reusable Assets**: Create common AbilitySets that can be shared across multiple PawnData
4. **Testing**: Test each ability individually before combining in sets
5. **Documentation**: Clearly comment the purpose and behavior of each AbilitySet
6. **Input Separation**: Remember that input binding is handled via InputConfig, not AbilitySets
