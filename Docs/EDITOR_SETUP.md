# Editor setup — full walkthrough (after the config / UI / audio refactor)

Do this in order, as if setting the project up from scratch against the current C++. The C++ already
references everything by the **exact asset paths / Project Settings below**, so naming matters.

> Convention: *Project Settings* means **Edit → Project Settings → Game** section.

---

## 0. Plugins & first launch
1. Open `KitchenUnderPressure.uproject`. If prompted, let it build the editor modules.
2. Edit → Plugins → confirm **CommonUI** and **Audio Modulation** are enabled. Restart if asked.

---

## 1. Spells  (`Content/Spells/`)
The element/form/modifier data assets already exist. You only create the config that bundles them.

1. Right-click → **Miscellaneous → Data Asset** → pick class **SpellConfig** → name it **`DA_SpellConfig`**.
2. Fill it:
   | Field | Value |
   |---|---|
   | Damage Effect | `GE_Damage` |
   | Element Pool | `DA_Element_Fire`, `_Ice`, `_Lightning`, `_Poison` |
   | Form Pool | `DA_Form_Projectile`, `DA_Form_Nova` |
   | Modifier Pool | `DA_Modifier_Overcharged`, `DA_Modifier_Unstable` |
   | Common / Rare / Epic / Legendary **Weight** | 60 / 25 / 12 / 3 |
   | Common / Rare / Epic / Legendary **Power** | 1.0 / 1.4 / 1.9 / 2.6 |
   | Combo Window | 0.35 |
   | Oppositions | (`Element.Fire`, `Element.Ice`), (`Element.Lightning`, `Element.Poison`) |
   | Same Element Combo Multiplier / Opposed | 2.0 / 2.5 |
   | Cross Player Combo Window / Radius | 1.0 / 350 |
3. Project Settings → **Alchemist Spells** → **Active Config** = `DA_SpellConfig`.
   *(The .ini already points to `/Game/Spells/DA_SpellConfig`; name it exactly and it auto-links.)*

---

## 2. Audio — Control Buses  (`Content/Audio/`)
Modern Epic audio mixing: sliders drive control buses, everything routed through them scales.

1. **Volume parameter** (once): right-click → **Audio → Modulation → Control Bus**; when it asks for a
   *Parameter*, create/choose a **Volume** modulation parameter (linear, default 1.0).
2. **3 Control Buses** (same Volume parameter): `CB_Master`, `CB_Music`, `CB_Sfx`.
3. **Control Bus Mix** `CBMix_Main`: add **3 stages** → CB_Master / CB_Music / CB_Sfx, each value `1.0`.
4. **2 Sound Classes** (right-click → Audio → Sound Class):
   - `SC_Music` → details **Modulation → Volume** → add `CB_Master` **and** `CB_Music`.
   - `SC_SFX`   → details **Modulation → Volume** → add `CB_Master` **and** `CB_Sfx`.
5. Assign the Sound Class on the sounds:
   - `Music_MainMenu_1`, `Music_Game_1` → **Sound Class = SC_Music**.
   - every UI / gameplay SFX → **Sound Class = SC_SFX**.
6. **DA_AudioConfig** — Data Asset → class **AudioConfig**:
   | Field | Value |
   |---|---|
   | Menu Music / Gameplay Music | `Music_MainMenu_1` / `Music_Game_1` |
   | Music Fade Seconds | 1.0 |
   | Master Bus / Music Bus / Sfx Bus | `CB_Master` / `CB_Music` / `CB_Sfx` |
   | Main Mix | `CBMix_Main` |
7. Project Settings → **Game Audio** → **Active Config** = `DA_AudioConfig`.

*Volumes persist automatically through `UKUPGameUserSettings` (registered in `DefaultEngine.ini`) — no step.*

---

## 3. CommonUI

### 3a. Input data (gamepad navigation + Back)  `Content/UI/Input/`
1. **DT_InputActions** — right-click → **Miscellaneous → Data Table**, row struct **CommonInputActionDataBase**. Add at least:
   - row **`Back`** → keys: Gamepad *Face Button Right (B)*, Keyboard *Escape*.
   - row **`Confirm`** → keys: Gamepad *Face Button Bottom (A)*, Keyboard *Enter / Space Bar*.
2. **BP_CommonInputData** — Blueprint Class → parent **CommonUIInputData**:
   - **Default Click Action** = `DT_InputActions → Confirm`
   - **Default Back Action**  = `DT_InputActions → Back`
3. Project Settings → **Common Input Settings** → **Input Data** = `BP_CommonInputData`.
   *(Leave “Enable Enhanced Input Support” off — we use the data-table action path. PC controller/platform
   data uses engine defaults; auto gamepad detection is on by default.)*

### 3b. Shared button style + the one common button
1. **DA_ButtonStyle** — Blueprint Class → parent **CommonButtonStyle**. Set:
   - Normal / Hovered / Pressed / Disabled brushes, the text style, and the **Hovered & Pressed Slate Sounds**
     (this is where UI button sounds now live — they replaced the old menu SFX code).
2. **WBP_CommonButton** — Widget Blueprint, **parent class `KUPButton`**:
   - Class Defaults → **Style** = `DA_ButtonStyle` (this draws the button background from the brushes).
   - Inside its designer, add a **CommonTextBlock** named exactly **`ButtonTextBlock`** (centred). This is
     the label: `CommonButtonBase` is **not** a single-child panel, so the caption lives *inside* the button.
   - Each placed instance sets its caption in Details → **Button → Button Text** (e.g. "Solo").

> This single button + style is the “common button style” goal: design once, reuse everywhere.
> Don’t try to drop a TextBlock onto a *placed* button — set its **Button Text** property instead.

### 3c. Primary layout (the viewport shell)
1. **WBP_PrimaryLayout** — Widget Blueprint, **parent class `KUPPrimaryLayout`**:
   - Add a **CommonActivatableWidgetStack** named exactly **`MenuStack`**, anchored to fill the screen.
2. Project Settings → **Game UI** → **Primary Layout** = `WBP_PrimaryLayout`.

### 3d. The screen Blueprints (required widget names + types)
Each WBP keeps its existing **parent class** (unchanged C++ name). Inside, every old `Button` becomes a
**WBP_CommonButton** instance and every `Slider` an **AnalogSlider**, keeping the **exact widget name** so
`BindWidget` resolves. *“Is Back Handler” is already set in C++ — don’t tick anything.* If the editor flags
the parent, use **File → Reparent Blueprint** to the same C++ class.

**WBP_PrimaryLayout** (`KUPPrimaryLayout`): `MenuStack` → CommonActivatableWidgetStack.

**WBP_MainMenu** (`UMainMenuWidget`):
| Widget name | Type |
|---|---|
| SoloButton, MultiplayerButton, OptionsButton, QuitButton | WBP_CommonButton |
| CreateButton, JoinButton, MultiplayerBackButton | WBP_CommonButton |
| OptionsBackButton, RefreshButton, BrowserBackButton | WBP_CommonButton |
| TokenInput | EditableTextBox |
| ServerListBox | ScrollBox |
| BrowserStatusText | TextBlock |
| MenuSwitcher | WidgetSwitcher — children order **0 Root, 1 Multiplayer, 2 Options, 3 Browser** |
| AudioOptions | WBP_AudioOptions instance *(optional)* |
| *Class Defaults:* Server Row Class = `WBP_ServerRow`, Max Players = 5 |

**WBP_Lobby** (`ULobbyWidget`):
| Widget name | Type |
|---|---|
| PlayerListBox | ScrollBox |
| ReadyButton, StartButton, LeaveButton | WBP_CommonButton |
| StatusText, CountText | TextBlock |

**WBP_InGameMenu** (`UInGameMenuWidget`):
| Widget name | Type |
|---|---|
| ContinueButton, OptionsButton, MainMenuButton, QuitButton | WBP_CommonButton |
| MenuSwitcher | WidgetSwitcher *(optional: page 0 = buttons, page 1 = options)* |
| OptionsBackButton | WBP_CommonButton *(optional)* |
| AudioOptions | WBP_AudioOptions instance *(optional)* |

**WBP_AudioOptions** (`UAudioOptionsWidget`):
| Widget name | Type |
|---|---|
| MasterSlider, MusicSlider, SfxSlider | **AnalogSlider** |
| MasterValueText, MusicValueText, SfxValueText | TextBlock *(optional)* |

**WBP_ServerRow** (`UServerRowWidget`):
| Widget name | Type |
|---|---|
| OwnerText, SlotsText, PingText | TextBlock |
| JoinRowButton | WBP_CommonButton |

### 3e. Wire the widget classes onto the controllers / game modes
- **Menu** PC (`AMenuPlayerController` BP, used by `BP_MenuGameMode`) → **Main Menu Class** = `WBP_MainMenu`.
- **Lobby** PC (`ALobbyPlayerController` BP) → **Lobby Widget Class** = `WBP_Lobby`.
- **Gameplay** PC (BP of `AKitchenUnderPressurePlayerController`, which is `abstract`) →
  **In Game Menu Class** = `WBP_InGameMenu`, **Input Config** = `DA_GameplayInput`.

---

## 4. Optional content cleanup
These assets are no longer referenced (the bespoke menu input system was deleted) and can be removed:
`Content/Input/IMC_Menu`, `IA_MenuNavigate`, `IA_MenuAdjust`, `IA_MenuAccept`, `IA_MenuBack`.

---

## 5. Smoke test
- **Boot** → Main menu appears via CommonUI; buttons share the style; Solo loads the game; gamepad navigates,
  Back returns to the root panel.
- **Multiplayer** → host/join, lobby ready/start/leave, Back leaves.
- **Pause** (in game) → Esc/Back opens & closes; Options page; the 3 sliders move the mix live and persist
  after relaunch (`Saved/Config/Windows/GameUserSettings.ini`).
