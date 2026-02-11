# Implementation Plan

- [x] 1. Set up project structure and build system



  - Create CMakeLists.txt with SDL2, OpenGL, and dependency configuration
  - Set up directory structure (src/, assets/, external/, docs/, tests/)
  - Configure compiler flags for C++17 and optimization settings
  - Add third-party libraries (GLM, GLAD, stb_image, RapidJSON)
  - Create main.cpp entry point with basic application skeleton
  - _Requirements: 1.1, 1.2_

- [ ] 2. Implement core application and initialization system
  - [ ] 2.1 Create Application class with SDL2 initialization
    - Implement SDL_Init for video, audio, and input subsystems
    - Create window with OpenGL context (version 3.3+)
    - Set up OpenGL viewport and initial state
    - Implement error handling for initialization failures with message dialogs
    - _Requirements: 1.1, 1.2, 1.4_

  - [ ] 2.2 Implement main game loop with fixed timestep
    - Create Run() method with frame timing and delta time calculation
    - Implement fixed 120Hz physics update loop independent of rendering
    - Add ProcessInput(), Update(), and Render() method calls
    - Implement frame rate limiting and VSync support
    - _Requirements: 1.5, 2.1, 11.3_

  - [ ] 2.3 Create logging system
    - Implement Logger class with log levels (Debug, Info, Warning, Error, Fatal)
    - Add file output to logs/game.log with timestamps
    - Implement console output with color coding for different log levels
    - _Requirements: 12.3, 12.5_

  - [ ] 2.4 Implement configuration system
    - Create ConfigManager class to load/save JSON configuration files
    - Implement graphics.json parser for resolution, quality, fullscreen settings
    - Implement game.json parser for difficulty, overs, environment settings
    - Add validation and default value fallback for invalid config entries
    - _Requirements: 10.1, 10.2, 10.3, 10.4, 12.4_

- [ ] 3. Implement rendering system foundation
  - [ ] 3.1 Create shader management system
    - Implement Shader class to compile and link vertex/fragment shaders
    - Add shader uniform setting methods (mat4, vec3, float, int)
    - Create ShaderManager to cache and retrieve compiled shaders
    - Implement error handling for shader compilation with detailed error logs
    - Load fallback shaders if primary shaders fail to compile
    - _Requirements: 1.3, 1.4, 12.1_

  - [ ] 3.2 Implement basic rendering pipeline
    - Create Renderer class with BeginFrame() and EndFrame() methods
    - Implement perspective projection matrix calculation
    - Add basic mesh rendering with vertex buffers and index buffers
    - Implement depth testing and face culling
    - _Requirements: 2.1, 2.2_

  - [ ] 3.3 Create camera system
    - Implement Camera class with position, target, and FOV
    - Add GetViewMatrix() and GetProjectionMatrix() methods
    - Implement camera modes (BowlerView, BatsmanView, FollowBall, Cinematic)
    - Create smooth camera transitions between modes
    - Implement dynamic ball-following camera with interpolation
    - _Requirements: 2.2, 2.3_

  - [ ] 3.4 Implement resource management for rendering
    - Create ResourceManager singleton for textures, meshes, and shaders
    - Implement texture loading using stb_image with error handling
    - Add mesh loading from OBJ files with vertex/normal/UV parsing
    - Implement resource caching to avoid duplicate loads
    - _Requirements: 11.1, 11.5, 12.2_

- [ ] 4. Create 3D models and shaders
  - [ ] 4.1 Write core shader programs
    - Create player.vert and player.frag for character rendering with Phong lighting
    - Create field.vert and field.frag for terrain rendering
    - Create ball.vert and ball.frag for ball rendering with specular highlights
    - Create ui.vert and ui.frag for 2D orthographic UI rendering
    - Add uniform declarations for MVP matrices, lighting, and material properties
    - _Requirements: 1.3, 2.5_

  - [ ] 4.2 Create basic 3D models
    - Create cricket field mesh (pitch, boundary, grass)
    - Create player model with basic rigging for animation
    - Create ball mesh (sphere with seam texture)
    - Create stumps and bails models
    - Export models to OBJ format with proper UV coordinates
    - _Requirements: 2.1, 2.4_

  - [ ] 4.3 Implement lighting system
    - Add directional light with configurable direction and color
    - Implement ambient lighting with adjustable intensity
    - Calculate Phong lighting in fragment shader (ambient + diffuse + specular)
    - Add shadow mapping for player and ball shadows on field
    - _Requirements: 2.5_

- [ ] 5. Implement physics engine core
  - [ ] 5.1 Create ball physics with basic dynamics
    - Implement Ball class with position, velocity, spin, mass, and radius
    - Add Update() method with Euler integration for position and velocity
    - Apply gravity (9.8 m/s²) to ball trajectory
    - Implement drag force calculation using air density and drag coefficient
    - _Requirements: 5.1, 5.4_

  - [ ] 5.2 Implement environmental effects on ball physics
    - Create EnvironmentParams struct with humidity, wind, pitch properties
    - Implement swing calculation based on humidity (20-40% increase above 70% humidity)
    - Add wind force application proportional to wind speed and direction
    - Implement seam movement based on pitch grass coverage (25-35% increase above 60%)
    - Calculate bounce height modification based on pitch hardness (15-25% increase for hard pitch)
    - Apply friction coefficient (0.3-0.7) based on pitch moisture for ball-ground contact
    - _Requirements: 5.5, 13.1, 13.2, 13.3, 13.4, 13.5_

  - [ ] 5.3 Implement Magnus effect for spin
    - Calculate Magnus force from ball spin and velocity
    - Apply lateral force perpendicular to velocity and spin axis
    - Adjust curve magnitude based on ball speed and spin rate
    - _Requirements: 4.4_

  - [ ] 5.4 Create collision detection system
    - Implement sphere-plane collision for ball-ground contact
    - Create sphere-box collision for ball-bat contact
    - Implement sphere-cylinder collision for ball-stumps contact
    - Calculate collision response with impulse-based physics
    - Add collision callbacks for game events (wicket, boundary)
    - _Requirements: 5.2, 5.3_

  - [ ] 5.5 Implement ball trajectory calculation
    - Create CalculateBallTrajectory() method for predictive trajectory
    - Calculate landing point and bounce trajectory for bowling
    - Implement trajectory visualization for bowling aim indicator
    - _Requirements: 4.2, 4.3_

- [ ] 6. Implement input handling system
  - [ ] 6.1 Create input handler with keyboard and mouse support
    - Implement InputHandler class with SDL event polling
    - Track current and previous frame key states for edge detection
    - Add IsKeyPressed() and IsKeyJustPressed() methods
    - Implement mouse position and delta tracking
    - _Requirements: 3.1, 8.5_

  - [ ] 6.2 Add gamepad support
    - Detect and initialize SDL_GameController
    - Implement axis value reading for analog sticks
    - Add button state tracking for gamepad buttons
    - Create gamepad vibration support for feedback
    - _Requirements: 3.1_

  - [ ] 6.3 Create control mapping system
    - Implement ControlScheme struct with configurable key bindings
    - Map batting shots (defensive, drive, cut, pull, sweep, lofted) to keys
    - Map bowling deliveries (fast, medium, spin, yorker, bouncer) to keys
    - Add control remapping UI in settings menu
    - _Requirements: 3.3, 4.1_

- [ ] 7. Implement batting gameplay mechanics
  - [ ] 7.1 Create batting shot system
    - Implement shot type selection based on input
    - Calculate shot timing relative to ball arrival time
    - Compute shot power based on timing accuracy (50% reduction for >100ms error)
    - Apply shot direction based on shot type and ball position
    - Calculate ball deflection angle and velocity after bat contact
    - _Requirements: 3.1, 3.2, 3.3, 3.4_

  - [ ] 7.2 Add shot timing feedback system
    - Create visual timing indicator (perfect, good, early, late)
    - Display feedback within 100ms of shot execution
    - Add color-coded timing zones (green for perfect, yellow for good, red for poor)
    - Implement timing window calculation based on ball speed
    - _Requirements: 3.5_

  - [ ] 7.3 Implement batsman animation system
    - Create AnimationController class for skeletal animation
    - Load batting shot animations (defensive, drive, cut, pull, sweep, lofted)
    - Implement smooth animation blending between idle and shot poses
    - Synchronize animation timing with shot execution
    - _Requirements: 2.4_

- [ ] 8. Implement bowling gameplay mechanics
  - [ ] 8.1 Create bowling delivery system
    - Implement delivery type selection UI with 1-second display time
    - Calculate initial ball velocity based on delivery type
    - Set ball spin parameters for different delivery types
    - Apply release timing variation to ball trajectory
    - _Requirements: 4.1, 4.2, 4.3_

  - [ ] 8.2 Implement bowling trajectory indicator
    - Create visual line showing predicted ball path
    - Display bounce point on pitch
    - Update indicator based on selected delivery type
    - Show indicator for 1 second before ball release
    - _Requirements: 4.2_

  - [ ] 8.3 Add bowling animations
    - Load bowling action animations (fast, medium, spin)
    - Implement run-up animation with speed variation
    - Synchronize ball release with animation frame
    - Add follow-through animation after delivery
    - _Requirements: 2.4_

- [ ] 9. Implement AI controller system
  - [ ] 9.1 Create AI difficulty system
    - Implement AIController class with Difficulty enum (Beginner, Medium, Hard, Expert)
    - Set reaction times (300ms, 150ms, 75ms, 50ms) for each difficulty
    - Configure shot accuracy (60%, 75%, 85%, 95%) for each difficulty
    - Add random variation to AI decisions using std::mt19937
    - _Requirements: 6.3, 6.4_

  - [ ] 9.2 Implement AI batting logic
    - Create DecideBattingShot() method analyzing ball trajectory
    - Calculate optimal shot type based on ball line and length
    - Consider field placement when selecting shot direction
    - Implement timing calculation with difficulty-based accuracy
    - Add shot selection within 50ms of ball release
    - _Requirements: 6.1, 6.4_

  - [ ] 9.3 Implement AI bowling logic
    - Create DecideBowlingDelivery() method with delivery variation
    - Ensure at least 30% variation in delivery types across an over
    - Analyze batsman weaknesses from scoring patterns
    - Select bowling target based on batsman position
    - _Requirements: 6.2_

  - [ ] 9.4 Add AI field placement strategy
    - Implement field placement presets (attacking, defensive, balanced)
    - Adjust fielder positions based on batsman scoring areas
    - Update field placement between overs based on match situation
    - _Requirements: 6.5_

- [ ] 10. Implement match management system
  - [ ] 10.1 Create match state tracking
    - Implement MatchState struct with runs, wickets, overs, balls
    - Track current striker, non-striker, and bowler
    - Calculate run rate and required run rate
    - Store over-by-over scores and wicket information
    - _Requirements: 7.1_

  - [ ] 10.2 Implement match progression logic
    - Create MatchManager class to control match flow
    - Handle ball delivery events and update state
    - Implement over completion with bowler change
    - Add innings transition when 10 wickets fall or overs complete
    - Detect match conclusion and determine winner
    - _Requirements: 7.4_

  - [ ] 10.3 Add scoring system
    - Implement run counting for singles, twos, threes, boundaries (4 and 6)
    - Update score within 100ms of runs being scored
    - Trigger boundary celebration animation for 4s and 6s
    - Handle extras (wides, no-balls, byes, leg-byes)
    - _Requirements: 7.2_

  - [ ] 10.4 Implement wicket handling
    - Detect wicket types (bowled, caught, LBW, run out)
    - Update wicket count and record dismissal details
    - Transition to next batsman within 2 seconds
    - Display wicket animation and celebration
    - _Requirements: 7.3_

  - [ ] 10.5 Create match conclusion system
    - Calculate final scores and match result
    - Determine player of the match based on performance
    - Display match statistics (top scorer, best bowler, boundaries)
    - Show results screen within 3 seconds of match end
    - _Requirements: 7.5_

- [ ] 11. Implement user interface and HUD
  - [ ] 11.1 Create HUD rendering system
    - Implement UI rendering with orthographic projection
    - Create text rendering using bitmap fonts or FreeType
    - Add alpha transparency (0.7-0.9) to HUD elements
    - Ensure HUD updates every frame without impacting performance
    - _Requirements: 8.2_

  - [ ] 11.2 Implement score display
    - Display current score, wickets, and overs continuously
    - Show run rate with one decimal precision
    - Add current partnership and last wicket information
    - Update display in real-time as match progresses
    - _Requirements: 8.1_

  - [ ] 11.3 Add ball speed indicator
    - Calculate and display ball speed in km/h
    - Show speed within 2% accuracy of actual velocity
    - Display speed when ball is in play
    - Add speed comparison with previous deliveries
    - _Requirements: 8.3_

  - [ ] 11.4 Create fielder position mini-map
    - Implement top-down view of field with fielder positions
    - Update mini-map every 500ms
    - Highlight ball position on mini-map
    - Show fielder names on hover
    - _Requirements: 8.4_

  - [ ] 11.5 Implement menu system
    - Create main menu with Play, Settings, Exit options
    - Add match setup screen for team selection and match configuration
    - Implement settings menu for graphics, audio, and controls
    - Add pause menu during gameplay
    - Ensure menu navigation responds within 50ms
    - _Requirements: 1.5, 8.5_

- [ ] 12. Implement audio system
  - [ ] 12.1 Create audio manager with SDL_mixer
    - Initialize SDL_mixer with appropriate audio format
    - Implement AudioManager class for sound and music playback
    - Create sound effect cache with Mix_Chunk loading
    - Add music playback with Mix_Music support
    - _Requirements: 1.1, 9.1_

  - [ ] 12.2 Add gameplay sound effects
    - Load bat-hit sounds (soft for defensive, hard for aggressive)
    - Play bat-hit sound within 16ms of collision
    - Load and play stumps-breaking sound for wickets
    - Add ball-bounce sound for ground contact
    - _Requirements: 9.1, 9.2_

  - [ ] 12.3 Implement crowd audio
    - Play ambient crowd noise continuously at 30-50% volume
    - Trigger crowd cheer at 80% volume for boundaries
    - Play crowd reaction for wickets within 100ms
    - Add crowd anticipation sounds during close calls
    - _Requirements: 9.2, 9.3, 9.4_

  - [ ] 12.4 Add volume control system
    - Implement independent volume controls for SFX and music
    - Provide 10 discrete volume levels (0-100% in 10% increments)
    - Apply volume changes immediately without restart
    - Persist volume settings to configuration file
    - _Requirements: 9.5_

- [ ] 13. Implement performance optimizations
  - [ ] 13.1 Add rendering optimizations
    - Implement frustum culling to skip off-screen objects
    - Add level-of-detail (LOD) system for distant objects
    - Use instanced rendering for multiple fielders
    - Implement texture atlasing to reduce state changes
    - _Requirements: 2.1, 11.3_

  - [ ] 13.2 Optimize physics calculations
    - Implement spatial partitioning (octree) for collision detection
    - Use predictive collision for analytical solutions when possible
    - Add sleep states for stationary objects
    - Cache frequently calculated values (trajectory predictions)
    - _Requirements: 5.1_

  - [ ] 13.3 Implement memory management
    - Create object pools for particles and ball trails
    - Implement lazy loading for menu assets
    - Use texture compression (DXT/BC) for large textures
    - Ensure memory usage stays under 2GB
    - _Requirements: 11.1, 11.2, 11.4_

- [ ] 14. Add error handling and stability features
  - [ ] 14.1 Implement comprehensive error handling
    - Add try-catch blocks around critical operations
    - Log stack traces for unhandled exceptions
    - Display user-friendly error dialogs with recovery options
    - Implement graceful degradation for non-critical failures
    - _Requirements: 12.3_

  - [ ] 14.2 Add crash reporting system
    - Implement automatic crash log generation
    - Save current game state before crash
    - Create crash report with system information and error details
    - Add user consent mechanism for crash reporting
    - _Requirements: 12.3, 12.5_

  - [ ] 14.3 Implement resource validation
    - Validate all loaded assets (textures, models, shaders)
    - Use placeholder resources for failed loads
    - Log warnings for missing or corrupted assets
    - Continue operation with degraded functionality if possible
    - _Requirements: 12.1, 12.2, 12.4_

- [ ] 15. Create documentation
  - [ ] 15.1 Write code documentation
    - Add Doxygen-style comments to all public APIs
    - Document complex algorithms with inline comments
    - Create class and method documentation for all major components
    - _Requirements: All_

  - [ ] 15.2 Create user documentation
    - Write README.md with build instructions and dependencies
    - Create user manual covering controls, game modes, and settings
    - Add quick start guide for first-time players
    - Write troubleshooting guide for common issues
    - _Requirements: All_

  - [ ] 15.3 Generate API reference
    - Configure Doxygen for C++ code documentation
    - Generate HTML documentation from code comments
    - Create architecture overview document
    - Add contributing guide with code style guidelines
    - _Requirements: All_

- [ ] 16. Build and packaging
  - [ ] 16.1 Finalize build configuration
    - Configure CMake for Release builds with optimizations
    - Set up platform-specific build targets (Windows, Linux, macOS)
    - Add install targets for assets and executables
    - Create build scripts for automated compilation
    - _Requirements: 1.1, 1.2_

  - [ ] 16.2 Package game for distribution
    - Create installer packages for each platform
    - Bundle all required DLLs and dependencies
    - Include assets in distribution package
    - Add license files and documentation
    - _Requirements: All_

  - [ ] 16.3 Create automated build pipeline
    - Set up CI/CD for automated builds on commits
    - Add automated testing in build pipeline
    - Configure artifact generation for releases
    - _Requirements: All_
