# InputConfig Asset Creation Guide

This document guides you through creating and configuring `UCombatInputConfig` assets to manage input mapping for abilities in the GAS system.

> **🔗 Quick Setup**: For a complete walkthrough of setting up InputConfig along with AbilitySets and PawnData, see [COMPLETE_SETUP_GUIDE.md](COMPLETE_SETUP_GUIDE.md).

## Overview

`UCombatInputConfig` is a data asset that contains all information about mapping between input actions and ability triggers:

-   **Input Mappings**: Links between Input Actions and Ability Triggers
-   **Ability Triggers**: Trigger tags to activate abilities
-   **Trigger Events**: Types of trigger events (Started, Completed, etc.)

## Step 1: Create InputConfig Asset

### In Unreal Editor:

1. **Open Content Browser**
2. **Right-click > Miscellaneous > Data Asset**
3. **Select UCombatInputConfig**
4. **Name it** (e.g., `DA_PlayerInputConfig`)

## Step 2: Configure Input Mappings

### Input Mappings Category:

**Input Mappings** (TArray<FCombatInputMapping>):

Each mapping includes:

-   **Input Action**: Input action from project settings
-   **Ability Triggers**: List of triggers for the ability
-   **Consume Input**: Whether to consume input when processed

### 2.1 Create Input Actions (If Not Already Created)

Before configuring InputConfig, you need to create Input Actions in Project Settings:

1. **Project Settings > Engine > Input**
2. **Action Mappings > + Button**
3. **Name it** (e.g., `IA_ComboAttack`, `IA_ChargedAttack`, `IA_LockTarget`)

### 2.2 Example Input Mappings

#### Combo Attack Mapping:

```
Input Action: IA_ComboAttack
Consume Input: true
Ability Triggers:
- Trigger Tag: Event.Attack.Combo.Start
- Trigger Event: Started
- Trigger Once: false
```

#### Charged Attack Mapping:

```
Input Action: IA_ChargedAttack
Consume Input: true
Ability Triggers:
- Trigger Tag: Event.Attack.Charged.Start
- Trigger Event: Started
- Trigger Once: false
- Trigger Tag: Event.Attack.Charged.Release
- Trigger Event: Completed
- Trigger Once: false
```

#### Lock Target Mapping:

```
Input Action: IA_LockTarget
Consume Input: true
Ability Triggers:
- Trigger Tag: Event.Target.Lock.Toggle
- Trigger Event: Started
- Trigger Once: false
```

## Step 3: Understanding Ability Triggers

### FCombatAbilityTriggerData Structure:

```cpp
/** The trigger tag for this ability */
UPROPERTY(EditDefaultsOnly, Category = "Trigger")
FGameplayTag TriggerTag;

/** The trigger event to listen for */
UPROPERTY(EditDefaultsOnly, Category = "Trigger")
ETriggerEvent TriggerEvent = ETriggerEvent::Started;

/** Whether this trigger should be consumed when processed */
UPROPERTY(EditDefaultsOnly, Category = "Trigger")
bool bTriggerOnce = false;
```

### Trigger Events:

-   **Started**: When input action begins (button press)
-   **Completed**: When input action ends (button release)
-   **Ongoing**: When input action is being held
-   **Canceled**: When input action is canceled

### Trigger Tags:

Trigger tags must match tags defined in the Ability System. Examples:

-   `Event.Attack.Combo.Start`
-   `Event.Attack.Charged.Start`
-   `Event.Attack.Charged.Release`
-   `Event.Target.Lock.Toggle`

## Step 4: Using InputConfig

### 4.1 Assign to PawnData

In **CombatPawnData**:

```
Input Config: DA_PlayerInputConfig
```

### 4.2 Integration with Character

In `ACombatCharacter::SetupPlayerInputComponent()`:

-   InputConfig is loaded from PawnData
-   Input mappings are bound automatically via `BindInputsFromConfig()`
-   Ability triggers are sent to AbilitySystemComponent via `SendAbilityTrigger()`
-   If no InputConfig is available, falls back to `BindBasicMovementInputs()` for basic movement/look controls

### 4.3 Ability System Integration

When input is triggered:

1. `SendAbilityTrigger()` is called with trigger tag
2. AbilitySystemComponent finds ability with matching tag
3. Ability is activated if found

## Complete Examples

### Player InputConfig (DA_PlayerInputConfig):

```
Input Mappings:
1. Input Action: IA_ComboAttack
   Consume Input: true
   Ability Triggers:
   - Trigger Tag: Event.Attack.Combo.Start
     Trigger Event: Started
     Trigger Once: false

2. Input Action: IA_ChargedAttack
   Consume Input: true
   Ability Triggers:
   - Trigger Tag: Event.Attack.Charged.Start
     Trigger Event: Started
     Trigger Once: false
   - Trigger Tag: Event.Attack.Charged.Release
     Trigger Event: Completed
     Trigger Once: false

3. Input Action: IA_LockTarget
   Consume Input: true
   Ability Triggers:
   - Trigger Tag: Event.Target.Lock.Toggle
     Trigger Event: Started
     Trigger Once: false

4. Input Action: IA_Move
   Consume Input: false
   Ability Triggers: [] (for movement)

5. Input Action: IA_Look
   Consume Input: false
   Ability Triggers: [] (for camera)
```

## Troubleshooting

### Input Not Triggering Ability:

-   **Check Trigger Tag**: Ensure tag matches ability's trigger tags
-   **Check Input Action**: Ensure input action is set up in Project Settings
-   **Check PawnData**: Ensure InputConfig is assigned in PawnData
-   **Check Ability**: Ensure ability has Granted Ability Spec with correct trigger tags

### Multiple Triggers for Same Input:

-   One input action can have multiple trigger tags
-   Example: Charged attack has both Started and Completed triggers
-   Ensure ability system handles logic correctly for each trigger

### Input Conflicts:

-   Avoid using same input action for different mappings
-   Use Consume Input to prevent conflicts
-   Test thoroughly in gameplay to ensure no overlapping

## Gameplay Tag Requirements

Ensure the following gameplay tags are defined in DefaultGameplayTags.ini:

```
GameplayTags=(
    Tag="Event.Attack.Combo.Start",
    DevComment="Trigger for combo attack start"
),
(
    Tag="Event.Attack.Charged.Start",
    DevComment="Trigger for charged attack start"
),
(
    Tag="Event.Attack.Charged.Release",
    DevComment="Trigger for charged attack release"
),
(
    Tag="Event.Target.Lock.Toggle",
    DevComment="Trigger for target lock toggle"
)
```
