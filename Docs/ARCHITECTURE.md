# Architecture — configuration, UI & audio

This project follows a few conventions so configuration stays modular and the right tool is used
for each job. This document is the quick reference.

## Configuration: the right tool per job

| Need | Tool | Examples |
|------|------|----------|
| Single global values editable in Project Settings, saved to `.ini` | `UDeveloperSettings` | `UGameMapSettings`, `UOnlineSettings`, and the *pointers* below |
| Authored content (many instances, referenced, async-loadable) | `UPrimaryDataAsset` | `USpellConfig`, `UAudioConfig`, `UElementDefinition`, `UInputConfig` |
| Persisted player options | `UGameUserSettings` subclass | `UKUPGameUserSettings` (volumes) |
| Runtime state / registries / indexes | `USubsystem` | `USpellRegistrySubsystem`, `UAudioSubsystem`, `UKUPUISubsystem` |
| UI: styles, navigation, focus, screens | **CommonUI** | see below |

The recurring pattern: a `UDeveloperSettings` holds a `TSoftObjectPtr` to the active data asset
(so it's selectable in Project Settings), and a subsystem loads it on first use.

- Spells: `USpellSettings.ActiveConfig` → `USpellConfig` (pools, rarity, combo tuning), resolved at
  runtime by `USpellRegistrySubsystem` (tag → definition). `Spells/`
- Audio: `UKUPAudioSettings.ActiveConfig` → `UAudioConfig` (music + control buses), driven by
  `UAudioSubsystem`. `Audio/`
- UI: `UKUPUISettings.PrimaryLayout` → the `UKUPPrimaryLayout` widget class. `UI/`
- Gameplay input: `UInputConfig` data asset referenced on the player controller. `Input/`

## UI — CommonUI

- Screens derive `UKUPActivatableWidget` (`UCommonActivatableWidget`). They handle Back via
  `NativeOnHandleBackAction()` and first focus via `NativeGetDesiredFocusTarget()`.
- Buttons derive `UKUPButton` (`UCommonButtonBase`); the shared look + press/hover sounds come from a
  `UCommonButtonStyle` data asset assigned once on the button Blueprint.
- `UKUPPrimaryLayout` is the per-player viewport shell holding a `CommonActivatableWidgetStack`.
  Controllers push screens with `UKUPUISubsystem::PushScreen(WidgetClass)` instead of
  `CreateWidget + AddToViewport`; CommonUI owns focus, gamepad/keyboard/mouse routing and Back.
- Volume sliders use `UAnalogSlider` so the gamepad adjusts them when focused.

## Audio — Audio Modulation control buses

- `UAudioSubsystem` (game instance) pushes the three slider values into the Master / Music / SFX
  **control buses** through a control bus mix; sounds scale automatically via their SoundClass
  routing. No code multiplies volumes by hand.
- Volumes persist in `UKUPGameUserSettings` (`GameUserSettings.ini`).

## Editor-side assets (created in the editor, referenced by the settings above)

- `DA_SpellConfig` (`USpellConfig`) → `Spell Settings`.
- `DA_AudioConfig` (`UAudioConfig`) + control buses `CB_Master/CB_Music/CB_Sfx`, mix `CBMix_Main`,
  sound classes `SC_Music/SC_SFX` → `Game Audio` settings.
- `WBP_PrimaryLayout` (with a `MenuStack`) → `Game UI` settings; `WBP_CommonButton` + `DA_ButtonStyle`.

## Source layout (by domain, Lyra-style)

`Source/KitchenUnderPressure/` is organised by domain, not by type:

| Folder | Holds |
|--------|-------|
| `GameModes/` | game modes + game state (`*GameMode`, `AlchemistGameState`) |
| `Player/` | player controllers + player states |
| `Character/` | the player character |
| `Camera/` | the player camera manager |
| `AbilitySystem` is split as `Abilities/` (ASC + GAs) and `Attributes/` (attribute set) |
| `Spells/` | spell content, config, registry, generator, caster, projectiles |
| `Enemies/`, `Interaction/`, `Dungeon/` | their gameplay features |
| `Audio/`, `Input/`, `UI/` | audio, input config, and the CommonUI widgets (`UI/Base/` = base classes) |
| `Settings/` | `UDeveloperSettings` + `UKUPGameUserSettings` |
| `Online/` | EOS session subsystem |
| `System/` | cross-cutting helpers: gameplay tags, combat statics, debug HUD |

Include paths: each domain folder is on `PublicIncludePaths`, so headers are included by bare name
or domain-relative path (`Player/...`, `Spells/...`).
