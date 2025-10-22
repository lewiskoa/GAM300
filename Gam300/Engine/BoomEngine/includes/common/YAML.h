#pragma once
//#define YAML_CPP_STATIC_DEFINE  // Add this for static linking
#include "Core.h"
#include "yaml-cpp/yaml.h"
#include "BoomProperties.h"  // Add this

namespace YAML
{
    //Change this to whatever data type we need to serialize/deserialize (SINCE GLM NOT ALLOWED OUTSIDE JRAFICS)
    //DONT YOU DARE SAY WE CANT USE GLM HERE AS WE ALSO NEED TO SERIALIZE GRAPHICS DATA RAAAAAAAAAAAAAAAH
    template<>
    struct convert<glm::vec3>
    {
        BOOM_INLINE static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }
        BOOM_INLINE static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <typename Enum>
    BOOM_INLINE Enum deserializeEnum(const YAML::Node& node, Enum fallback) {
        if (!node) return fallback;
        // Try string form
        if (node.IsScalar()) {
            try {
                auto str = node.as<std::string>();
                if (auto e = magic_enum::enum_cast<Enum>(str))
                    return *e;
            }
            catch (...) {
                // not a string, ignore
            }
            // Try integer form
            try {
                int val = node.as<int>();
                if (auto e = magic_enum::enum_cast<Enum>(val))
                    return *e;
            }
            catch (...) {
                // not an int, ignore
            }
        }
        return fallback;
    }

    //stream operator
    BOOM_INLINE Emitter& operator<<(Emitter& out, const glm::vec3& v)
    {
        out << Flow;
        out << BeginSeq << v.x << v.y << v.z << EndSeq;
        return out;
    }
}

// ============================================================================
// XPROPERTY <-> YAML INTEGRATION
// ============================================================================
namespace Boom {

    // Forward declarations
    void PropertyToYAML(YAML::Emitter& e, const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx);
    void YAMLToProperty(const YAML::Node& node, const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx);

    /**
     * @brief Recursively serialize an object's properties to YAML using xproperty
     */
    inline void SerializeObjectToYAML(YAML::Emitter& e, const xproperty::type::object* pObj, void* pInstance, xproperty::settings::context& ctx)
    {
        if (!pObj || !pInstance) return;

        // Iterate through all members
        for (const auto& member : pObj->m_Members)
        {
            PropertyToYAML(e, member, pInstance, ctx);
        }
    }

    /**
     * @brief Recursively deserialize YAML to an object's properties using xproperty
     */
    inline void DeserializeObjectFromYAML(const YAML::Node& node, const xproperty::type::object* pObj, void* pInstance, xproperty::settings::context& ctx)
    {
        if (!pObj || !pInstance || !node.IsDefined()) return;

        // Iterate through all members
        for (const auto& member : pObj->m_Members)
        {
            if (node[member.m_pName])
            {
                YAMLToProperty(node[member.m_pName], member, pInstance, ctx);
            }
        }
    }

    /**
     * @brief Convert a single property member to YAML
     */
    inline void PropertyToYAML(YAML::Emitter& e, const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx)
    {
        e << YAML::Key << member.m_pName;

        // Handle different member types
        if (std::holds_alternative<xproperty::type::members::var>(member.m_Variant))
        {
            // Atomic variable (int, float, vec3, etc.)
            auto& var = std::get<xproperty::type::members::var>(member.m_Variant);

            xproperty::any value;
            var.m_pRead(pInstance, value, var.m_UnregisteredEnumSpan, ctx);

            // Serialize based on type GUID
            auto typeGUID = value.getTypeGuid();

            if (typeGUID == xproperty::settings::var_type<float>::guid_v) {
                e << YAML::Value << value.get<float>();
            }
            else if (typeGUID == xproperty::settings::var_type<int32_t>::guid_v) {
                e << YAML::Value << value.get<int32_t>();
            }
            else if (typeGUID == xproperty::settings::var_type<uint32_t>::guid_v) {
                e << YAML::Value << value.get<uint32_t>();
            }
            else if (typeGUID == xproperty::settings::var_type<uint64_t>::guid_v) {
                e << YAML::Value << value.get<uint64_t>();
            }
            else if (typeGUID == xproperty::settings::var_type<bool>::guid_v) {
                e << YAML::Value << value.get<bool>();
            }
            else if (typeGUID == xproperty::settings::var_type<std::string>::guid_v) {
                e << YAML::Value << value.get<std::string>();
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec2>::guid_v) {
                auto v = value.get<glm::vec2>();
                e << YAML::Value << YAML::Flow << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec3>::guid_v) {
                auto v = value.get<glm::vec3>();
                e << YAML::Value << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec4>::guid_v) {
                auto v = value.get<glm::vec4>();
                e << YAML::Value << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
            }
            else if (typeGUID == xproperty::settings::var_type<glm::quat>::guid_v) {
                auto q = value.get<glm::quat>();
                e << YAML::Value << YAML::Flow << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
            }
            else if (value.isEnum()) {
                // Handle enums - save as string
                e << YAML::Value << value.getEnumString();
            }
            else {
                BOOM_WARN("[PropertyYAML] Unhandled atomic type GUID: {} for member: {}", typeGUID, member.m_pName);
            }
        }
        else if (std::holds_alternative<xproperty::type::members::props>(member.m_Variant))
        {
            // Nested object/scope
            auto& props = std::get<xproperty::type::members::props>(member.m_Variant);

            auto [pChildInstance, pChildObj] = props.m_pCast(pInstance, ctx);

            if (pChildInstance && pChildObj)
            {
                e << YAML::Value << YAML::BeginMap;
                SerializeObjectToYAML(e, pChildObj, pChildInstance, ctx);
                e << YAML::EndMap;
            }
            else
            {
                e << YAML::Value << YAML::Null;
            }
        }
        else
        {
            BOOM_WARN("[PropertyYAML] Unhandled variant type for member: {}", member.m_pName);
            e << YAML::Value << YAML::Null;
        }
    }

    /**
     * @brief Convert YAML to a single property member
     */
    inline void YAMLToProperty(const YAML::Node& node, const xproperty::type::members& member, void* pInstance, xproperty::settings::context& ctx)
    {
        if (!node.IsDefined()) return;

        // Handle different member types
        if (std::holds_alternative<xproperty::type::members::var>(member.m_Variant))
        {
            // Atomic variable
            auto& var = std::get<xproperty::type::members::var>(member.m_Variant);

            if (member.m_bConst || !var.m_pWrite) {
                return; // Read-only property
            }

            xproperty::any value;
            auto typeGUID = var.m_AtomicType.m_GUID;

            // Parse YAML based on expected type
            if (typeGUID == xproperty::settings::var_type<float>::guid_v) {
                value.set<float>(node.as<float>());
            }
            else if (typeGUID == xproperty::settings::var_type<int32_t>::guid_v) {
                value.set<int32_t>(node.as<int32_t>());
            }
            else if (typeGUID == xproperty::settings::var_type<uint32_t>::guid_v) {
                value.set<uint32_t>(node.as<uint32_t>());
            }
            else if (typeGUID == xproperty::settings::var_type<uint64_t>::guid_v) {
                value.set<uint64_t>(node.as<uint64_t>());
            }
            else if (typeGUID == xproperty::settings::var_type<bool>::guid_v) {
                value.set<bool>(node.as<bool>());
            }
            else if (typeGUID == xproperty::settings::var_type<std::string>::guid_v) {
                value.set<std::string>(node.as<std::string>());
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec2>::guid_v) {
                value.set<glm::vec2>(glm::vec2(node[0].as<float>(), node[1].as<float>()));
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec3>::guid_v) {
                value.set<glm::vec3>(glm::vec3(node[0].as<float>(), node[1].as<float>(), node[2].as<float>()));
            }
            else if (typeGUID == xproperty::settings::var_type<glm::vec4>::guid_v) {
                value.set<glm::vec4>(glm::vec4(node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>()));
            }
            else if (typeGUID == xproperty::settings::var_type<glm::quat>::guid_v) {
                value.set<glm::quat>(glm::quat(node[3].as<float>(), node[0].as<float>(), node[1].as<float>(), node[2].as<float>()));
            }
            else if (var.m_AtomicType.m_IsEnum) {
                // Handle enum - convert string to value
                value.set<std::string>(node.as<std::string>());
            }
            else {
                BOOM_WARN("[PropertyYAML] Unhandled atomic type GUID: {} for member: {}", typeGUID, member.m_pName);
                return;
            }

            // Write the value back
            var.m_pWrite(pInstance, value, var.m_UnregisteredEnumSpan, ctx);
        }
        else if (std::holds_alternative<xproperty::type::members::props>(member.m_Variant))
        {
            // Nested object/scope
            auto& props = std::get<xproperty::type::members::props>(member.m_Variant);

            auto [pChildInstance, pChildObj] = props.m_pCast(pInstance, ctx);

            if (pChildInstance && pChildObj && node.IsMap())
            {
                DeserializeObjectFromYAML(node, pChildObj, pChildInstance, ctx);
            }
        }
    }

} // namespace Boom