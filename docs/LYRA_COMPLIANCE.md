# Lyra Compliance Roadmap

This document outlines the current compliance status with Epic's Lyra sample game standards and provides a roadmap for future improvements.

## Current Status

### ✅ Fully Implemented

-   **GAS Architecture**: AbilitySystemComponent on PlayerState with proper initialization
-   **AttributeSets**: Separate attribute sets for Health, Damage, Stamina, Movement
-   **Replication**: Correct replication conditions (COND_OwnerOnly for UI, COND_None for gameplay)
-   **GrantedHandles Pattern**: Proper cleanup of abilities and effects
-   **AbilitySet Assets**: UCombatAbilitySet for modular ability management
-   **PawnData System**: Data-driven pawn configuration with ability sets
-   **Input Integration**: Enhanced Input with ability activation via gameplay events

### 🚧 Partially Implemented

-   **Modular Gameplay**: Variant system provides modularity but not as extensible as Lyra's Game Features
-   **Input Mapping**: Input actions are mapped but lack centralized InputConfig system

### ❌ Not Implemented

#### Core Systems

-   **InputConfig**: Centralized input configuration system

    -   Create `UCombatInputConfig` data asset
    -   Map input actions to ability activation
    -   Support for multiple input configs per pawn

-   **CameraMode**: Dynamic camera mode system

    -   Implement `UCombatCameraMode` base class
    -   Create modes: ThirdPerson, Aim, LockOn, etc.
    -   Camera mode stack management

-   **HUD Management**: Modular HUD system
    -   `UCombatHUDLayout` for layout management
    -   Widget switching based on game state
    -   HUD extension system

#### Advanced Features

-   **Pawn Extension System**: Modular pawn components

    -   `UCombatPawnExtensionComponent` base class
    -   Extensions for health, movement, interaction, etc.

-   **Controller Extension System**: Input handling extensions

    -   `UCombatControllerExtensionComponent` base class
    -   Extensions for camera control, ability input, etc.

-   **Player Spawning System**: Proper player spawning

    -   `ACombatPlayerSpawningManager` for spawn logic
    -   Pawn data application during spawn
    -   Respawn handling

-   **Experience System**: Leveling and progression

    -   ExperienceDefinition data assets
    -   Experience progression curves
    -   Level-based attribute scaling

-   **Game Feature System**: Modular game features
    -   Game feature plugins
    -   Feature activation/deactivation
    -   Runtime feature management

## Implementation Priority

### High Priority (Core Gameplay)

1. **InputConfig System**: Essential for clean input management
2. **CameraMode System**: Required for different gameplay states
3. **HUD Management**: Needed for UI consistency

### Medium Priority (Extensibility)

4. **Pawn/Controller Extensions**: Improve modularity
5. **Player Spawning System**: Better spawn handling

### Low Priority (Advanced Features)

6. **Experience System**: For RPG elements
7. **Game Feature System**: For large-scale modularity

## Migration Guide

### From Legacy Arrays to AbilitySets

The project has migrated from legacy `GrantedAbilities`/`GrantedEffects` arrays to `AbilitySets`. To complete the migration:

1. Create `UCombatAbilitySet` assets for each pawn type
2. Move abilities and effects from arrays to ability sets
3. Update `UCombatPawnData` to use `AbilitySets` instead of legacy arrays
4. Remove legacy array usage from code

### Adding New Lyra Systems

When implementing new systems, follow these patterns:

1. **Data Assets**: Use data assets for configuration (e.g., `UCombatInputConfig`)
2. **Extension Components**: Use extension components for modular functionality
3. **Handles and Cleanup**: Always use handle-based management with proper cleanup
4. **Replication**: Follow Lyra's replication patterns
5. **Modularity**: Design systems to be easily enabled/disabled

## Benefits of Full Compliance

-   **Maintainability**: Easier to extend and modify gameplay systems
-   **Performance**: Optimized replication and ability management
-   **Scalability**: Better support for multiplayer and complex games
-   **Best Practices**: Alignment with Epic's recommended patterns
-   **Community**: Easier for other developers to understand and contribute
