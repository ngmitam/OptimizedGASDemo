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
-   **Input Integration**: Enhanced Input with ability activation via gameplay events and InputConfig system
-   **Stamina System**: Comprehensive stamina mechanics with regeneration, costs, and stun effects

### 🚧 Partially Implemented

-   **Modular Gameplay**: Variant system provides modularity but not as extensible as Lyra's Game Features
-   **Input Mapping**: Input actions are mapped with centralized InputConfig system

### ❌ Not Implemented

#### Core Systems

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

## Benefits of Full Compliance

-   **Maintainability**: Easier to extend and modify gameplay systems
-   **Performance**: Optimized replication and ability management
-   **Scalability**: Better support for multiplayer and complex games
-   **Best Practices**: Alignment with Epic's recommended patterns
-   **Community**: Easier for other developers to understand and contribute
