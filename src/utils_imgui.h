#include <imgui.h>
#include <magic_enum.hpp>

namespace imgui_ext {
    namespace {
        template <typename E>
        bool EnumComboGetter(void* /*data*/, int idx, const char** out_text) {
            static_assert(std::is_enum_v<E>, "ImGuiEnumComboGetter requires an enum type.");

            constexpr auto names = magic_enum::enum_names<E>();
            constexpr int count = magic_enum::enum_count<E>();

            if (idx < 0 || idx >= count) {
                return false;
            }

            // ImGui expects a pointer to a null-terminated string; string_view::data() is fine
            *out_text = names.at(idx).data();
            return true;
        }

        // Callback used by InputText to resize std::string
        int InputTextCallback_StringResize(ImGuiInputTextCallbackData* data) {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                auto* str = static_cast<std::string*>(data->UserData);
                str->resize(data->BufTextLen);
                data->Buf = str->data();
            }
            return 0;
        }
    }

    inline bool InputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0) {
        flags |= ImGuiInputTextFlags_CallbackResize;
        return ImGui::InputText(label, str.data(), str.capacity() + 1, flags, InputTextCallback_StringResize,
                                &str);
    }

    template <typename E>
    bool EnumCombo(const char* label, E& value) {
        static_assert(std::is_enum_v<E>, "ImGuiEnumCombo requires an enum type.");

        constexpr auto values = magic_enum::enum_values<E>();
        constexpr auto count = magic_enum::enum_count<E>();

        // Find current index for the enum
        int current_index = 0;
        for (int i = 0; i < static_cast<int>(count); ++i) {
            if (values.at(i) == value) {
                current_index = i;
                break;
            }
        }

        // Draw the combo
        bool const changed = ImGui::Combo(label, &current_index, &EnumComboGetter<E>, nullptr, count);

        // If user picked something else, update the enum
        if (changed) {
            value = values.at(current_index);
        }

        return changed;
    }
}
