#include "gamedata_pickup.h"
#include "utils.h"
#include <nlohmann/json.hpp>
#include "gamedata.h"

MAGIC_SERIALIZE_ENUM(PickupType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PickupData, type, name);

PickupData PickupData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    PickupData data = json.get<PickupData>();

    if (json.contains("value")) {
        json["value"].get_to(data.value);
    }

    auto const* spriteData = context.gamedata.GetSpriteDatabase().Get(json["sprite"]);
    if (spriteData != nullptr) {
        data.sprite = *spriteData;
    }

    return data;
}
