#include "gamedata_drop.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DropData, sprite);

DropData DropData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    return json.get<DropData>();   
}
