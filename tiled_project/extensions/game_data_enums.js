// Builds custom enum types from the game data so object properties can pick
// from the existing enemies and behaviours. Requires Tiled 1.12+.

const enums = [
    { type: "enemy_type", file: "enemy_database.json", category: "enemies" },
    { type: "behaviour", file: "behaviour_database.json", category: "behaviours" },
    // "none" is not in the game data. it gives EnemySpawn.drop a way to say "no drop",
    // since tiled cannot clear an enum property
    { type: "pickup", file: "pickup_database.json", category: "pickups", extra: ["none"] },
    // the built-in formations are generated in code rather than authored in the game data, so they are
    // named here and the database adds the authored shapes after them
    {
        type: "formation",
        file: "formation_database.json",
        category: "formations",
        extra: ["line", "column", "v", "grid"],
    },
];

// Tiled resolves an enum property to the index of its value when a map is loaded and
// back to the name when it is saved, so the position of a value in the enum is what a
// map really stores. Two rules keep an already open map meaning what it says:
// the order of values already in the project never changes, and values are only
// dropped by the explicit menu action, never behind the user's back.
function mergeValues(existing, names, prune) {
    const kept = prune ? existing.filter((name) => names.indexOf(name) !== -1) : existing.slice();
    const added = names.filter((name) => kept.indexOf(name) === -1);
    return kept.concat(added);
}

function syncEnum(dataDir, { type, file, category, extra }, prune) {
    const path = FileInfo.joinPaths(dataDir, file);
    if (!File.exists(path)) {
        tiled.error(`Game data not found at ${path}`);
        return;
    }

    const textFile = new TextFile(path, TextFile.ReadOnly);
    const text = textFile.readAll();
    textFile.close();

    let names;
    try {
        names = (extra ?? []).concat(Object.keys(JSON.parse(text)[category] ?? {}).sort());
    } catch (e) {
        tiled.error(`Failed to parse ${path}: ${e.message}`);
        return;
    }

    let enumType = tiled.project.findTypeByName(type);
    if (enumType === null) {
        enumType = tiled.project.addEnumType(type);
    } else if (!enumType.isEnum) {
        tiled.error(`Custom type ${type} exists but is not an enum`);
        return;
    }

    const existing = enumType.values;
    const merged = mergeValues(existing, names, prune);
    if (merged.join("\n") !== existing.join("\n")) {
        enumType.values = merged;
        tiled.log(`Updated enum ${type} with ${merged.length} values`);
    }
}

function syncAll(prune) {
    if (!tiled.projectFilePath) {
        return;
    }
    const dataDir = FileInfo.cleanPath(FileInfo.joinPaths(FileInfo.path(tiled.projectFilePath), "../data"));
    for (const entry of enums) {
        syncEnum(dataDir, entry, prune);
    }
}

// Dropping a value shifts every value after it, so a map that is already open would
// keep the index it loaded and mean the wrong entry. Only do it on request, and say
// what the open maps need.
function syncAndPrune() {
    syncAll(true);
    tiled.log("Sync Game Data Enums: values removed from the game data are gone from the enums. "
        + "Reload any map that was already open (File > Reload) before saving it.");
}

const action = tiled.registerAction("SyncGameDataEnums", syncAndPrune);
action.text = "Sync Game Data Enums";
tiled.extendMenu("Map", [{ separator: true }, { action: "SyncGameDataEnums" }]);

// Pick up new game data before the session restores its maps, so their enum
// properties resolve against the enums the data describes.
syncAll(false);

// and keep them current as the game data changes between edits
tiled.assetOpened.connect(() => syncAll(false));
