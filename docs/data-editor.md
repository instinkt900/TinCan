# Generic data editor

Exploratory plan for an ImGui editor that can add, remove and edit entries in
every TinCan database, with drop-downs for enums and references between entries.

Status: **idea only.** Nothing here is committed to, and none of it is in
`docs/tasks.md`. When it is picked up, the stages below become tasks.

## Goal

A designer can open one editor window and, for any category (sprites,
projectiles, weapons, enemies, formations, pickups, behaviours, players, input):

- see every entry, filter them, and add, duplicate, rename or delete one;
- edit an entry's fields with widgets that suit each field, so enums are
  drop-downs of their values and references are drop-downs of the entries in the
  referenced category;
- see straight away when an entry would fail to load, and why;
- save back to the file the entry came from, without writing defaults the file
  never had.

## Where things stand

- The runtime data types (`gamedata_*.h`) are loaded with `from_json` through the
  `DATA_REQUIRED` / `DATA_OPTIONAL` macros. Only behaviours and formations can be
  written back (`to_json`).
- The runtime types are not a faithful copy of the files. Sprites hold loaded
  textures rather than paths, a `DataRef` resolves a name to a pointer, and an
  optional field that was left out cannot be told apart from one set to its
  default.
- A category can be spread over several files (`weapon_database_enemy.json` and
  `weapon_database_player.json`, the two sprite files). `Database<T>` does not
  record which file an entry came from.
- Some fields depend on a sibling: a behaviour state reads waypoints only for
  `Path` and a target only for `Target`.
- The behaviour and formation editors each read, modify and rewrite their own
  file on save. Their changes are picked up the next time the game starts.
- `DataRef` keeps pointers into the database maps, so the data cannot be
  reloaded in place under a running world.
- The Tiled project's `enemy_type`, `behaviour` and `formation` enums copy
  database keys by hand. Tiled stores enum values by index, so their order has to
  stay append-only.

## Approach

Edit the JSON, not the runtime types. The runtime types stay as they are; the
editor works on the documents they are loaded from.

**Data store.** An editor-side store holds each data file as a JSON document,
keeping the key order it was written in. It knows which file every entry lives
in, tracks unsaved files, and saves only those. It becomes the one thing that
writes data files, so the behaviour and formation editors save through it
instead of writing their files themselves.

**Schema per category.** Each data type gets a description of its fields,
declared beside its `from_json`: the field's kind, whether it is required, its
default, a tooltip, and when it is shown. The kinds cover what the data uses
today: bool, int, float, string, 2D vector, enum, reference, nested object,
optional block, list and named map.

- Enum fields take their values from `magic_enum`, so they need no extra
  upkeep.
- Reference fields list the entries of their category from the store, so a new
  entry can be referred to before it is saved. They also allow an inline value,
  because a `DataRef` accepts either a name or an embedded entry.
- Fields that only apply to one variant, such as a behaviour state's type, are
  shown only when that variant is chosen.
- A field that is not set shows its default dimmed. Setting it and resetting it
  are explicit, so saving does not fill the files with defaults.

**Validation by the loader.** After each edit the entry is run through its own
`from_json`, and any error is shown against the entry. The loader stays the
authority on what is valid, and a schema that has fallen out of step with it
shows up as an error instead of bad data. A check at load time also lists keys in
the files that the schema does not know, which catches both typos in the data and
fields missing from a schema. Sprites need care, as parsing one loads its
textures.

**Generic widgets.** One recursive drawing routine turns a schema and a JSON
value into ImGui widgets: scalars, vectors, enum and reference combos, optional
blocks toggled by a checkbox, lists with add, remove and reorder, and maps with
renamable keys.

**Editor window.** A category list, a filtered entry list with add, duplicate,
rename and delete, and a property panel. Because the schema knows where every
reference is, deleting or renaming an entry reports what refers to it, and a
rename can update those references.

**Existing editors.** The behaviour and formation editors keep their in-world
editing. The generic window covers everything else. Later they could reuse the
generic property widgets for the non-spatial fields.

## Stages

1. Data store: load every data file as a document, index entries by category and
   file, and save changed files.
2. Schema types and the generic widgets, with schemas for a couple of simple
   categories (weapons, projectiles) to prove the shape.
3. Editor window: category and entry lists, add, duplicate, rename, delete.
4. Enum and reference drop-downs, conditional fields, set and reset of defaults.
5. Validation through `from_json`, and the unknown-key check.
6. Schemas for the remaining categories, including sprites and behaviours.
7. Reference tracking on delete and rename.
8. Move the behaviour and formation editors' saving onto the store.
9. Optional: append new database keys to the Tiled enums, never reordering or
   removing existing ones.

## Open questions

- **When changes apply.** "Picked up on the next start", like the current
  editors, or "save and rebuild the world"? Reloading in place is ruled out by
  the pointers `DataRef` keeps.
- **Where it runs.** Inside the game's editor mode, which already has ImGui and a
  surface context for sprite previews and validation, or as a separate tool?
- **Keeping the schema and loader together.** Hand-written schemas beside
  `from_json` are the smaller change. The alternative is one field list per type
  that generates both, which removes the drift but means rewriting every gamedata
  header.
- **Which file a new entry goes in**, for categories spread across several files.

## Rough size

A data store of about 200 lines, schema types and widgets of 400 to 500, schemas
for the nine categories of about 200, and an editor window of about 300.
