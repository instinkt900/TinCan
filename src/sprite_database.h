#pragma once

#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>
#include <nlohmann/json_fwd.hpp>

struct SpriteImage {
    std::shared_ptr<canyon::graphics::IImage> image;
    moth_ui::FloatVec2 scale;
    moth_ui::IntVec2 offset;
    float rotation;
    canyon::graphics::BlendMode blend_mode;
    canyon::graphics::Color color;
    int32_t zOrder;

    bool Load(nlohmann::json& json, canyon::graphics::SurfaceContext& surfaceContext);
};

struct SpriteData {
    std::string name;
    std::map<std::string, SpriteImage> images;

    bool Load(nlohmann::json& json, canyon::graphics::SurfaceContext& surfaceContext);
};

class SpriteDatabase {
public:
    static std::unique_ptr<SpriteDatabase> Load(std::filesystem::path const& path,
                                                canyon::graphics::SurfaceContext& surfaceContext);

    SpriteData const* GetSpriteData(std::string const& name) const;

private:
    SpriteDatabase() = default;

    std::map<std::string, SpriteData> m_database;
};
