#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include "gamedata.h"

template <typename T>
class DataRef {
public:
    DataRef() = default;

    T& editable() {
        if (m_isRef) {
            m_value = *get();
            m_isRef = false;
            m_gamedata = nullptr;
            m_reference = nullptr;
        }
        return m_value;
    }

    bool valid() const { return m_init && (m_isRef ? (get() != nullptr) : true); }

    T* operator->() { return &editable(); }
    const T* operator->() const { return get(); }

    explicit operator bool() const { return valid(); }
    operator T const*() const { return get(); }

    T const* get() const {
        if (m_isRef) {
            if (m_reference == nullptr) {
                m_reference = m_gamedata->Get<T>(m_refName);
                assert(m_reference != nullptr);
            }
            return m_reference;
        }
        return &m_value;
    }

    DataRef<T>& operator=(T const& other) {
        m_init = true;
        m_isRef = false;
        m_gamedata = nullptr;
        m_reference = nullptr;
        m_value = other;
        return *this;
    }

    friend void from_json(nlohmann::json const& json, DataRef<T>& data) {
        data.m_init = true;
        if (json.is_string()) {
            data.m_isRef = true;
            data.m_refName = json.get<std::string>();
            data.m_gamedata = &GameData::s_currentContext->gamedata;
        } else {
            data.m_value = json.get<T>();
        }
    }

private:
    bool m_init = false;
    bool m_isRef = false;
    GameData const* m_gamedata = nullptr;
    std::string m_refName;
    mutable T const* m_reference = nullptr;
    T m_value;
};
