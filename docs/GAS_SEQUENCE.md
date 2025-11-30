# GAS Sequence Walkthrough

This short walkthrough contains sequence diagrams that show key runtime flows for the Gameplay Ability System (GAS) usage in the `Variant_Combat` codebase.

Note: diagrams use Mermaid syntax. View with a Markdown preview that supports Mermaid (e.g., VS Code with a Mermaid extension).

## 1) AbilitySystemComponent initialization

When a PlayerState is created, the ASC is constructed and initialized so abilities and attributes are available at runtime.

```mermaid
sequenceDiagram
    participant PS as ACombatPlayerState
    participant ASC as AbilitySystemComponent
    participant AS as AttributeSets

    PS->>ASC: CreateDefaultSubobject UAbilitySystemComponent
    PS->>ASC: SetIsReplicated(true)
    PS->>AS: CreateDefaultSubobject UHealthAttributeSet
    PS->>AS: CreateDefaultSubobject UDamageAttributeSet
    PS->>AS: CreateDefaultSubobject UStaminaAttributeSet
    PS->>PS: InitAbilityActorInfo(this, GetPawn())
    PS->>ASC: SetNumericAttributeBase(Health/MaxHealth/Stamina/MaxStamina/Damage)
    note over ASC,AS: ASC now owns AttributeSets and is ready
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
    Ability->>ASC: GetNumericAttribute(Damage/Knockback/LaunchImpulse/StaminaUsed)
    Ability->>Ability: ScaleDamage = Damage + (StaminaUsed * 1.5)
    Ability->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
    World-->>Ability: HitResults
    Ability->>Target: ApplyDamage(ScaledDamage, Instigator, ImpactPoint, Impulse)
    Ability->>Char: DealtDamage(ScaledDamage, ImpactPoint)
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
    Char->>ASC: Trigger Event.Attack.Combo.Start
    ASC->>Combo: ActivateAbility(TriggerEventData)
    Combo->>ASC: CommitAbility()
    Combo->>ASC: ApplyStaminaCost(StaminaCosts[0])
    Combo->>ASC: SetStaminaUsed(StaminaCosts[0])
    Combo->>Anim: PlayComboMontage
    Anim-->>Combo: MontageNotify_Hit
    Combo->>ASC: GetNumericAttribute(Damage)
    Combo->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
    World-->>Combo: HitResults
    Combo->>Target: ApplyDamage(Damage, Instigator, ImpactPoint, Impulse)
    Combo->>Char: DealtDamage(Damage, ImpactPoint)
    Player->>Char: ComboButton Press (next)
    Char->>ASC: Trigger Event.Attack.Combo.Next
    ASC->>Combo: HandleComboNext()
    Combo->>ASC: ApplyStaminaCost(StaminaCosts[1])
    Combo->>ASC: AddStaminaUsed(StaminaCosts[1])
    Combo->>Anim: Montage_JumpToSection(NextSection)

    %% Charged attack flow (press and hold)
    Player->>Char: ChargedButton Press (start)
    Char->>ASC: Trigger Event.Attack.Charged.Start
    ASC->>Charged: ActivateAbility(ChargeStart)
    Charged->>ASC: ApplyStaminaCost(StaminaCosts[0])
    Charged->>Charged: StartChargeLoop (accumulate TotalStaminaUsed)
    Charged->>Anim: PlayChargedAttackMontage
    loop While Holding
        Anim-->>Charged: MontageNotify_Charged.Loop
        Charged->>ASC: ApplyStaminaCost(StaminaCosts[0])
        Charged->>Charged: TotalStaminaUsed += StaminaCosts[0]
        Charged->>Anim: Montage_JumpToSection(ChargeLoopSection)
    end
    alt Release
        Player->>Char: ChargedButton Release
        Char->>Charged: Trigger Event.Attack.Charged.Release
        Charged->>ASC: SetStaminaUsed(TotalStaminaUsed)
        Charged->>Anim: Montage_JumpToSection(ChargeAttackSection)
        Anim-->>Charged: MontageNotify_Hit
        Charged->>ASC: GetNumericAttribute(Damage)
        Charged->>World: SweepMultiByObjectType(TraceStart, TraceEnd)
        World-->>Charged: HitResults
        Charged->>Target: ApplyDamage(Damage, Instigator, ImpactPoint, Impulse)
        Charged->>Char: DealtDamage(Damage, ImpactPoint)
    else AutoRelease (Stamina Depleted)
        Charged->>Charged: HandleChargedAttackRelease()
        Note right of Charged: Same flow as manual release
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

    Source->>Target: DealDamage(Damage, ImpactPoint, Impulse)
    Target->>TargetASC: Trigger Event.Damage.Received
    TargetASC->>DamageAbility: ActivateAbility(TriggerEventData)
    DamageAbility->>TargetASC: CommitAbility()
    DamageAbility->>TargetASC: MakeOutgoingSpec(GE)
    DamageAbility->>DamageAbility: Set SetByCaller Data.Damage = -Damage
    DamageAbility->>TargetASC: ApplyGameplayEffectSpecToTarget(DamageSpec)
    TargetASC-->>Target: AttributeChanged(Health decreased)
    alt Health <= 0
        Target->>TargetASC: Trigger Event.Death
        TargetASC->>DeathAbility: ActivateAbility()
        DeathAbility->>Target: HandleDeath() - DisableMovement, Ragdoll, Timer
        alt Player
            DeathAbility->>Target: RespawnCharacter()
        else Enemy
            DeathAbility->>Target: RemoveFromLevel()
        end
    else
        Target->>Target: ReceivedDamageFeedback()
    end
```

## 5) Stamina regeneration flow (CombatStaminaRegenerationAbility)

This flow shows how stamina regenerates passively for players and reactively for enemies.

```mermaid
sequenceDiagram
    participant Player as PlayerCharacter
    participant Enemy as EnemyCharacter
    participant ASC as AbilitySystemComponent
    participant RegenAbility as CombatStaminaRegenerationAbility
    participant GE as CombatStaminaRegenerationGE

    %% Player continuous regeneration
    Player->>ASC: Grant CombatStaminaRegenerationAbility
    ASC->>RegenAbility: OnGiveAbility() -> TryActivateAbility()
    RegenAbility->>RegenAbility: ApplyRegenerationEffect() - Continuous GE
    RegenAbility->>ASC: ApplyGameplayEffectSpec(CombatStaminaRegenerationGE)
    ASC-->>Player: Stamina gradually increases over time

    %% Enemy reactive regeneration
    Enemy->>ASC: Grant CombatStaminaRegenerationAbility
    ASC->>RegenAbility: OnGiveAbility() -> TryActivateAbility()
    RegenAbility->>RegenAbility: Bind to Stamina attribute changes
    Enemy->>Enemy: Use Stamina (attacks, etc.)
    ASC-->>RegenAbility: OnStaminaChanged() - Detect depletion
    alt Stamina == 0
        RegenAbility->>ASC: ApplyGameplayEffectSpec(CombatStaminaStunGE)
        ASC-->>Enemy: State.Stunned tag applied
        RegenAbility->>RegenAbility: Start refill timer
        RegenAbility->>ASC: Remove StaminaStunGE after delay
        RegenAbility->>RegenAbility: ApplyRegenerationEffect() - Refill to max
    else Stamina > 0
        RegenAbility->>RegenAbility: ApplyRegenerationEffect() - Gradual regen
    end
```

## How to use these diagrams

-   Open `docs/GAS_SEQUENCE.md` in VS Code and preview with Mermaid support to see the diagrams.
-   Use these as a reference when adding new abilities, attributes, or GameplayEffects: follow the ASC init pattern and the ability commit/apply flow.
-   Key concepts: Stamina costs scale damage, momentum preservation during attacks, enemy notification system, and proper cleanup with GrantedHandles.
