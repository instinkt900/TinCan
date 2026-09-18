// Builds custom enum types from the game data so object properties can pick
// from the existing enemies and behaviours. Requires Tiled 1.12+.

const enums = [
    { type: "enemy_type", file: "enemy_database.json", category: "enemies" },
    { type: "behaviour", file: "behaviour_database.json", category: "behaviours" },
    { type: "pickup", file: "pickup_database.json", category: "pickups" },
];

function syncEnum(dataDir, { type, file, category }) {
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
        names = Object.keys(JSON.parse(text)[category] ?? {}).sort();
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

    if (enumType.values.join("\n") !== names.join("\n")) {
        enumType.values = names;
        tiled.log(`Updated enum ${type} with ${names.length} values`);
    }
}

function syncAll() {
    if (!tiled.projectFilePath) {
        return;
    }
    const dataDir = FileInfo.cleanPath(FileInfo.joinPaths(FileInfo.path(tiled.projectFilePath), "../data"));
    for (const entry of enums) {
        syncEnum(dataDir, entry);
    }
}

const action = tiled.registerAction("SyncGameDataEnums", syncAll);
action.text = "Sync Game Data Enums";
tiled.extendMenu("Map", [{ separator: true }, { action: "SyncGameDataEnums" }]);

// keep the enums current as the game data changes between edits
tiled.assetOpened.connect(syncAll);
