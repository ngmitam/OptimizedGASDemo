## Gameplay Ability System (GAS) — Walkthrough

This document extracts the Gameplay Ability System (GAS) walkthrough used in the `Variant_Combat` gameplay variant.

Purpose

-   Explain how GAS is integrated in this project.
-   Point to the key classes and files to get started.
-   Provide implementation notes, tips for extending, and quick troubleshooting guidance.

For detailed instructions on creating and configuring PawnData assets for players and enemies, see [`PAWNDATA_GUIDE.md`](PAWNDATA_GUIDE.md).

For detailed instructions on creating and configuring InputConfig assets for ability input mapping, see [`INPUTCONFIG_GUIDE.md`](INPUTCONFIG_GUIDE.md).

### High-level contract

-   Inputs: gameplay events (e.g. `Event.Trace.Attack`, `Event.Damage.Received`), input actions (combo/charged attack), and GameplayEffect/SetByCaller values.
-   Outputs: attribute updates (Health, Damage, Knockback, Launch), GameplayEffects applied to targets, and gameplay tags/state changes for ability flow control.
-   Error modes: missing `AbilitySystemComponent` on actor will fallback to default values; GameplayEffect application is guarded by validity checks.

### Key classes and where to find them

-   `ACombatPlayerState` — AbilitySystemComponent owner and attribute init
    -   `Source/OptimizedGASDemo/Variant_Combat/CombatPlayerState.h/.cpp`
-   `UHealthAttributeSet` — Health, MaxHealth
    -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/HealthAttributeSet.h/.cpp`
-   `UDamageAttributeSet` — Damage, KnockbackImpulse, LaunchImpulse
    -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/DamageAttributeSet.h/.cpp`
-   `UStaminaAttributeSet` — Stamina, MaxStamina, StaminaUsed
    -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/StaminaAttributeSet.h/.cpp`
-   `UMovementAttributeSet` — Movement-related attributes (for stun effects)
    -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/MovementAttributeSet.h/.cpp`
-   Abilities (activation, commit, use of attributes and GameplayEffects):
    -   `CombatTraceAttackAbility` — performs sweep traces and uses attributes for damage/knockback
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatTraceAttackAbility.*`
    -   `CombatReceiveDamageAbility` — triggered on damage events, applies GameplayEffects and handles death
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatReceiveDamageAbility.*`
    -   `CombatComboAttackAbility` — handles combo attacks with montage notifications
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatComboAttackAbility.*`
    -   `CombatChargedAttackAbility` — press-and-hold charged attacks
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatChargedAttackAbility.*`
    -   `CombatDeathAbility` — handles death logic and effects
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatDeathAbility.*`
    -   `CombatStaminaRegenerationAbility` — passive stamina regeneration with different behaviors for players/enemies
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatStaminaRegenerationAbility.*`
    -   Other abilities: `CombatLockableAbility`, `CombatLockToggleAbility`, `CombatNotifyEnemiesAbility`
-   Character and integration points:
    -   `ACombatCharacter` — binds input, triggers ability events, and exposes helpers for visual feedback
        -   `Source/OptimizedGASDemo/Variant_Combat/CombatCharacter.*`
    -   `ACombatPlayerState` initializes `AbilitySystemComponent` and sets default attribute values at `BeginPlay`

### How GAS is used (implementation notes)

-   The `AbilitySystemComponent` lives on the `PlayerState` (`ACombatPlayerState`) and is replicated. `InitAbilityActorInfo(this, this)` is called in `BeginPlay` to wire actor <-> owner relationship.
-   Default attribute base values are set via `AbilitySystemComponent->SetNumericAttributeBase(...)` using `UCombatAttributeSet` accessor helpers.
-   Abilities use `CommitAbility` and `EndAbility` where appropriate. Abilities fetch attributes at runtime via `GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(...)`.
-   Damage is applied using a dedicated `UCombatDamageGameplayEffect` class (see `Gameplay/Effects`) with SetByCaller magnitude tags (e.g. `Data.Damage`) so the ability supplies the amount.
-   Abilities use GameplayTags extensively for ability grouping, blocking, and triggering (e.g. `Ability.Type.Attack`, `State.Attacking`, `Event.Trace.Attack`).

### Lyra-style GrantedHandles Pattern

This project now implements Lyra's GrantedHandles pattern for proper ability and effect management:

-   **FCombatAbilitySetHandle**: Tracks granted abilities, effects, and attributes for cleanup
-   **UCombatAbilitySet**: Manages granting and removing ability sets with proper handle tracking
-   **Dynamic Pawn Data Changes**: AbilitySystemComponent can now properly clean up when pawn data changes

#### Usage Example

```cpp
// Grant ability set
FCombatAbilitySetHandle AbilitySetHandle;
AbilitySet->GiveToAbilitySystem(ASC, AbilitySetHandle, this);

// Later, remove the granted abilities/effects
AbilitySet->TakeFromAbilitySystem(ASC, AbilitySetHandle);
```

#### Migration from Legacy Arrays - COMPLETED

The old `GrantedAbilities` and `GrantedEffects` arrays in `UCombatPawnData` have been removed. The project now exclusively uses `AbilitySets`:

1. Create a `UCombatAbilitySet` asset
2. Add abilities and effects to the asset
3. Reference the asset in `UCombatPawnData::AbilitySets`
4. The system automatically handles granting/removing with proper cleanup

### Lyra Compliance Status

This project follows Lyra standards where applicable for a combat-focused GAS implementation:

#### ✅ Compliant Features

-   **GAS Architecture**: ASC on PlayerState, separate AttributeSets (Health, Damage, Stamina, Movement), PawnData-driven granting
-   **Replication**: Proper COND_OwnerOnly for UI attributes, COND_None for gameplay
-   **GrantedHandles Pattern**: Implemented for ability/effect management
-   **AbilitySet Assets**: UCombatAbilitySet for modular ability granting
-   **Input Integration**: Input Actions mapped to abilities via AbilitySets and InputConfig
-   **Stamina System**: Comprehensive stamina mechanics with regeneration, costs, and stun effects
-   **Attribute System Refactor**: Split UCombatAttributeSet into specialized attribute sets

#### ❌ Missing Lyra Systems

-   **CameraMode**: Dynamic camera modes (e.g., for aiming, different perspectives)
-   **HUD Management**: Modular HUD system with layout switching
-   **Pawn/Controller Extensions**: Extension components for modular functionality
-   **Experience System**: Leveling and progression mechanics
-   **Game Features**: Modular feature activation/deactivation

### Troubleshooting (common issues)

-   "AbilitySystemComponent is null" — ensure the actor has a PlayerState with `AbilitySystemComponent` and `InitAbilityActorInfo` is called (done in `BeginPlay` of `ACombatPlayerState`). For possess events, ensure `NotifyControllerChanged`/possess hooks re-init actor info.
-   Replication problems for attributes — verify `GetLifetimeReplicatedProps` in `UCombatAttributeSet` contains `DOREPLIFETIME_CONDITION_NOTIFY` for each attribute and the owning `AbilitySystemComponent` is replicated.
-   GameplayEffect doesn't apply or doesn't change attributes — ensure the effect is valid, the SetByCaller tag name matches, and the target ASC is valid.
