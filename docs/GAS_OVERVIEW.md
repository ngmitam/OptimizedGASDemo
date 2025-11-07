## Gameplay Ability System (GAS) — Walkthrough

This document extracts the Gameplay Ability System (GAS) walkthrough used in the `Variant_Combat` gameplay variant.

Purpose

-   Explain how GAS is integrated in this project.
-   Point to the key classes and files to get started.
-   Provide implementation notes, tips for extending, and quick troubleshooting guidance.

### High-level contract

-   Inputs: gameplay events (e.g. `Event.Trace.Attack`, `Event.Damage.Received`), input actions (combo/charged attack), and GameplayEffect/SetByCaller values.
-   Outputs: attribute updates (Health, Damage, Knockback, Launch), GameplayEffects applied to targets, and gameplay tags/state changes for ability flow control.
-   Error modes: missing `AbilitySystemComponent` on actor will fallback to default values; GameplayEffect application is guarded by validity checks.

### Key classes and where to find them

-   `ACombatPlayerState` — AbilitySystemComponent owner and attribute init
    -   `Source/OptimizedGASDemo/Variant_Combat/CombatPlayerState.h/.cpp`
-   `UCombatAttributeSet` — Health, MaxHealth, Damage, KnockbackImpulse, LaunchImpulse, TraceDistance, TraceRadius
    -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Attributes/CombatAttributeSet.h/.cpp`
-   Abilities (activation, commit, use of attributes and GameplayEffects):
    -   `CombatTraceAttackAbility` — performs sweep traces and uses attributes for damage/knockback
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatTraceAttackAbility.*`
    -   `CombatReceiveDamageAbility` — triggered on damage events, applies GameplayEffects and handles death
        -   `Source/OptimizedGASDemo/Variant_Combat/Gameplay/Abilities/CombatReceiveDamageAbility.*`
    -   Other abilities: `CombatComboAttackAbility`, `CombatChargedAttackAbility`, `CombatLockableAbility`, `CombatLockToggleAbility`
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

### Quick tips for editing/extending GAS

-   Adding a new attribute: add it to `UCombatAttributeSet`, add REP callbacks and `DOREPLIFETIME` entries, then expose a getter and set a sensible default in the AttributeSet constructor.
-   Adding a new ability: create a `UGameplayAbility` subclass in `Gameplay/Abilities/`, set its tags, and use `AbilityTriggers` to activate from gameplay events; use `MakeOutgoingSpec` + `ApplyGameplayEffectSpecToTarget` to apply effects.
-   For authoritative gameplay state (server-controlled): prefer applying GameplayEffects on the server via ASC on the authoritative actor or PlayerState.

### Troubleshooting (common issues)

-   "AbilitySystemComponent is null" — ensure the actor has a PlayerState with `AbilitySystemComponent` and `InitAbilityActorInfo` is called (done in `BeginPlay` of `ACombatPlayerState`). For possess events, ensure `NotifyControllerChanged`/possess hooks re-init actor info.
-   Replication problems for attributes — verify `GetLifetimeReplicatedProps` in `UCombatAttributeSet` contains `DOREPLIFETIME_CONDITION_NOTIFY` for each attribute and the owning `AbilitySystemComponent` is replicated.
-   GameplayEffect doesn't apply or doesn't change attributes — ensure the effect is valid, the SetByCaller tag name matches, and the target ASC is valid.
