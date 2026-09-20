#pragma once

#include "gamedata.h"
#include "gamedata_formation.h"
#include <moth/graphics/utils/math_utils.h>
#include <moth/graphics/utils/vector.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// the whole arrangement is staggered on an angle, which is visible in tiled while the spawn is being placed
inline void RotateFormationOffsets(std::vector<moth::gfx::FloatVec2>& offsets, float rotation) {
    if (rotation == 0.0f) {
        return;
    }
    for (auto& offset : offsets) {
        offset = moth::gfx::Rotate2D(offset, rotation);
    }
}

// offsets of the first count slots of a formation, in map pixels, sized by scale and turned by rotation.
// The formation editor previews the entry it is editing, which is not in the database yet, so this takes
// the data rather than a name. It is silent about a count larger than the shape: the caller that reads
// from the database reports that.
inline std::vector<moth::gfx::FloatVec2> GetFormationOffsets(FormationData const& data, int count,
                                                             moth::gfx::FloatVec2 const& scale, float rotation) {
    std::vector<moth::gfx::FloatVec2> offsets;
    auto const slots = static_cast<int>(data.slots.size());
    offsets.reserve(static_cast<size_t>(std::min(count, slots)));
    for (int i = 0; i < std::min(count, slots); ++i) {
        offsets.push_back({ data.slots[static_cast<size_t>(i)].x * scale.x,
                            data.slots[static_cast<size_t>(i)].y * scale.y });
    }
    RotateFormationOffsets(offsets, rotation);
    return offsets;
}

// offsets of each formation member from the spawn point, in map pixels. the spawn point is the front of the
// formation and members further back are placed above it, so they scroll into view after the front.
// A formation in the game data is a shape in unit space sized by scale; the built-in generators are
// built from a step and are sized by spacing instead, so the two never multiply together.
inline std::vector<moth::gfx::FloatVec2> GetFormationOffsets(GameData const& gamedata, std::string const& formation,
                                                             int count, float spacing,
                                                             moth::gfx::FloatVec2 const& scale, float rotation) {
    if (auto const* data = gamedata.Get<FormationData>()->Find(formation); data != nullptr) {
        if (auto const slots = static_cast<int>(data->slots.size()); count > slots) {
            spdlog::warn("Formation {} has {} slots, but {} were asked for", formation, slots, count);
        }
        return GetFormationOffsets(*data, count, scale, rotation);
    }

    std::vector<moth::gfx::FloatVec2> offsets;
    offsets.reserve(static_cast<size_t>(count));
    if (formation == "line") {
        for (int i = 0; i < count; ++i) {
            offsets.push_back({ (static_cast<float>(i) - static_cast<float>(count - 1) / 2.0f) * spacing, 0.0f });
        }
    } else if (formation == "column") {
        for (int i = 0; i < count; ++i) {
            offsets.push_back({ 0.0f, -static_cast<float>(i) * spacing });
        }
    } else if (formation == "v") {
        // the leader is on the point, the rest alternate stepping back to the left and right
        for (int i = 0; i < count; ++i) {
            auto const step = static_cast<float>((i + 1) / 2);
            auto const side = i % 2 == 1 ? -1.0f : 1.0f;
            offsets.push_back({ side * step * spacing, -step * spacing });
        }
    } else if (formation == "grid") {
        // rows fill front to back, each centred on the point
        auto const columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
        for (int i = 0; i < count; ++i) {
            auto const row = i / columns;
            auto const rowCount = std::min(columns, count - (row * columns));
            auto const column = i % columns;
            offsets.push_back({ (static_cast<float>(column) - static_cast<float>(rowCount - 1) / 2.0f) * spacing,
                                -static_cast<float>(row) * spacing });
        }
    } else {
        spdlog::error("Map spawn has unknown formation {}", formation);
        offsets.push_back({ 0.0f, 0.0f });
    }

    RotateFormationOffsets(offsets, rotation);
    return offsets;
}
