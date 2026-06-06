# Armored Core 6 Archipelago

A multiworld randomiser for Armored Core VI. Story, arena, and mercenary-rank
progress send Archipelago checks; the AC parts you receive are shuffled across
the multiworld.

# Instructions

## 1. Set up ModEngine2 for Armored Core VI

Download [ModEngine2](https://github.com/soulsmods/ModEngine2/releases) and
confirm it launches the game normally before adding the mod.

## 2. Copy the mod files into your ModEngine2 mod folder

Place all of the following into the ModEngine2 `mod` folder:

- `ac6ap.dll`
- `ac6ap.cfg`
- `regulation.bin`

## 3. Register the DLL in your ME2 config

Open `config_armoredcore6.toml` and add `ac6ap.dll` to the `external_dlls`
list:

```toml
external_dlls = ["mod/ac6ap.dll"]
```

## 4. Edit `ac6ap.cfg` with your connection details

Open it in any text editor and set:

```
host=        your room's address
port=        your room's port number
slot=        your slot name
password=    your room's password (leave blank if none)
```

## 5. Launch the game through ME2

Run `launchmod_armoredcore6.bat`. Do **not** launch the game from Steam
directly. It must start through ModEngine2 for the mod to load.

---

## Important Notes

- `slot` must match the `name` in your YAML **exactly** (same spelling and
  capitalisation).
- Leave `password` blank if the room has no password.
- If you already use a modified `regulation.bin` for another mod, you will need
  to merge the changes rather than overwrite it.
- A log file (`ac6ap_log.txt`) appears in the mod folder — check it if the
  mod doesn't connect.

# Notes / Known Limits

- Item-acquisition notifications don't pop on screen; check your AP client to
  see what you received. Parts appear in the assembly menu (back out and
  re-enter the menu if you don't see one immediately).
- Shop-purchase checks are not in this build. Checks come from story, key
  missions, arena, and mercenary ranks (plus optional archive logs).
- The `archive_logs` option relies on archive collection flags that are still
  being verified; treat that option as experimental for now.
