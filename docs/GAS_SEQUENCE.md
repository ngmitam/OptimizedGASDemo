# GAS Sequence Walkthrough

This short walkthrough contains sequence diagrams that show key runtime flows for the Gameplay Ability System (GAS) usage in the `Variant_Combat` codebase.

Note: diagrams use Mermaid syntax. View with a Markdown preview that supports Mermaid (e.g., VS Code with a Mermaid extension).

## 1) AbilitySystemComponent initialization

When a PlayerState is created, the ASC is constructed and initialized so abilities and attributes are available at runtime.

```mermaid
sequenceDiagram
    participant PS as ACombatPlayerState
    participant ASC as AbilitySystemComponent
    participant AS as CombatAttributeSet

    PS->>ASC: CreateDefaultSubobject UAbilitySystemComponent
    PS->>ASC: SetIsReplicated(true)
    PS->>AS: CreateDefaultSubobject UCombatAttributeSet
    PS->>ASC: InitAbilityActorInfo(this, this)
    PS->>ASC: SetNumericAttributeBase(GetMaxHealth, DefaultMaxHP)
    PS->>ASC: SetNumericAttributeBase(GetHealth, DefaultMaxHP)
    note over ASC,AS: ASC now owns AttributeSet and is ready
```

## 2) Attack ability activation & trace flow (CombatTraceAttackAbility)

This diagram shows a high-level flow where an ability is triggered by an input or gameplay event, commits, reads attributes, performs a trace, and applies damage via interfaces or GameplayEffects.

```mermaid
sequenceDiagram
    participant Char as ACombatCharacter
    participant ASC as AbilitySystemComponent
    participant Ability as CombatTraceAttackAbility
    participant World as WorldPhysics
    participant Target as HitActor

    Char->>ASC: Trigger Event.Trace.Attack
    ASC->>Ability: ActivateAbility(TriggerEventData)
    Ability->>ASC: CommitAbility()
    Ability->>ASC: GetNumericAttribute(Damage/Knockback/TraceParams)
    Ability->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
    World-->>Ability: HitResults
    Ability->>Target: ApplyDamage(Damage, Instigator, ImpactPoint, Impulse)
    Ability->>Char: DealtDamage(Damage, ImpactPoint)
```

## 3) Combo and Charged attack flow

This diagram shows the input and ability flow for combo attacks (quick chained presses) and charged attacks (press and hold -> release to launch).

```mermaid
sequenceDiagram
    participant Player as PlayerInput
    participant Char as ACombatCharacter
    participant ASC as AbilitySystemComponent
    participant Combo as CombatComboAttackAbility
    participant Charged as CombatChargedAttackAbility
    participant Anim as AnimationSystem
    participant World as WorldPhysics
    participant Target as HitActor

    %% Combo attack flow (quick presses)
    Player->>Char: ComboButton Press
    Char->>ASC: Trigger Event.Combo.Attack
    ASC->>Combo: ActivateAbility(TriggerEventData)
    Combo->>ASC: CommitAbility()
    Combo->>Anim: PlayComboMontage
    Anim-->>Combo: MontageNotify_Hit
    Combo->>ASC: GetNumericAttribute(Damage)
    Combo->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
    World-->>Combo: HitResults
    Combo->>Target: ApplyDamage(Damage, Instigator, ImpactPoint, Impulse)
    Combo->>Char: DealtDamage(Damage, ImpactPoint)

    %% Charged attack flow (press and hold)
    Player->>Char: ChargedButton Press (start)
    Char->>ASC: Trigger Event.Charge.Start
    ASC->>Charged: ActivateAbility(ChargeStart)
    Charged->>Charged: StartChargeLoop (accumulate charge)
    alt Release
        Player->>Char: ChargedButton Release
        Char->>Charged: Trigger ChargeRelease
        Charged->>ASC: CommitAbility()
        Charged->>Anim: PlayChargedAttackMontage
        Anim-->>Charged: MontageNotify_Hit
        Charged->>ASC: GetNumericAttribute(Damage)
        Charged->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
        World-->>Charged: HitResults
        Charged->>Target: ApplyDamage(Damage, Instigator, ImpactPoint, Impulse)
        Charged->>Char: DealtDamage(Damage, ImpactPoint)
    else TimeoutOrCancel
        Charged->>Charged: AutoCancelOrMaxCharge
    end
```

## 4) Receive damage flow & GameplayEffect application (CombatReceiveDamageAbility)

This flow shows a damage event causing the receive-damage ability to apply a GameplayEffect (SetByCaller) to update health and trigger death logic.

```mermaid
sequenceDiagram
    participant Source as Attacker
    participant Target as Victim
    participant TargetASC as TargetAbilitySystemComponent
    participant DamageAbility as CombatReceiveDamageAbility
    participant GE as CombatDamageGameplayEffect

    Source->>Target: DealDamage
    Target->>TargetASC: Trigger Event.Damage.Received
    TargetASC->>DamageAbility: ActivateAbility(TriggerEventData)
    DamageAbility->>TargetASC: MakeOutgoingSpec(GE)
    DamageAbility->>DamageAbility: Set SetByCaller Data.Damage = -Damage
    DamageAbility->>TargetASC: ApplyGameplayEffectSpecToTarget(DamageSpec)
    TargetASC-->>Target: AttributeChanged(Health decreased)
    alt Health <= 0
        Target->>Target: HandleDeath()
    else
        Target->>Target: ReceivedDamageFeedback()
    end
```

## How to use these diagrams

-   Open `docs/GAS_SEQUENCE.md` in VS Code and preview with Mermaid support to see the diagrams.
-   Use these as a reference when adding new abilities, attributes, or GameplayEffects: follow the ASC init pattern and the ability commit/apply flow.
