#pragma once
#include "fms_types.hpp"
#include "location.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace fms {

struct Material {
    virtual ~Material() = default;
    IdType id;
    MaterialType type;
    std::string name;
    std::string category;
    IdType location_id;
    double weight;
    bool is_active;
    
    Material() 
        : id(INVALID_ID), type(MaterialType::WORKPIECE), location_id(INVALID_ID),
          weight(0.0), is_active(true) {}
    
    Material(IdType mat_id, MaterialType mat_type, const std::string& mat_name,
             const std::string& mat_category = "", double w = 0.0)
        : id(mat_id), type(mat_type), name(mat_name), category(mat_category),
          location_id(INVALID_ID), weight(w), is_active(true) {}
    
    bool is_at_location(IdType loc_id) const {
        return location_id == loc_id;
    }
    
    void move_to(IdType new_loc_id) {
        location_id = new_loc_id;
    }
};

struct Workpiece : public Material {
    std::string process_type;
    uint32_t process_steps;
    uint32_t completed_steps;
    bool is_processed;
    
    Workpiece() : Material(), process_steps(0), completed_steps(0), is_processed(false) {}
    
    Workpiece(IdType id, const std::string& name, const std::string& proc_type,
              uint32_t steps = 1)
        : Material(id, MaterialType::WORKPIECE, name, "workpiece"),
          process_type(proc_type), process_steps(steps), 
          completed_steps(0), is_processed(false) {}
    
    void complete_step() {
        if (completed_steps < process_steps) {
            completed_steps++;
            is_processed = (completed_steps >= process_steps);
        }
    }
};

struct Tool : public Material {
    uint32_t tool_life;
    uint32_t remaining_life;
    bool is_worn;
    
    Tool() : Material(), tool_life(id_constants::DEFAULT_TOOL_LIFE), remaining_life(id_constants::DEFAULT_TOOL_LIFE), is_worn(false) {}
    
    Tool(IdType id, const std::string& name, const std::string& category,
         uint32_t life = id_constants::DEFAULT_TOOL_LIFE)
        : Material(id, MaterialType::TOOL, name, category),
          tool_life(life), remaining_life(life), is_worn(false) {}
    
    void use() {
        if (remaining_life > 0) {
            remaining_life--;
            is_worn = (remaining_life == 0);
        }
    }
    
    double get_health_percent() const {
        return static_cast<double>(remaining_life) / static_cast<double>(tool_life);
    }
};

class MaterialManager {
public:
    IdType create_workpiece(const std::string& name, const std::string& process_type, 
                           uint32_t steps = 1) {
        IdType id = next_id++;
        materials[id] = std::make_unique<Workpiece>(id, name, process_type, steps);
        return id;
    }
    
    IdType create_tool(const std::string& name, const std::string& category, 
                      uint32_t life = id_constants::DEFAULT_TOOL_LIFE) {
        IdType id = next_id++;
        materials[id] = std::make_unique<Tool>(id, name, category, life);
        return id;
    }
    
    const Material* get(IdType id) const {
        auto it = materials.find(id);
        return it != materials.end() ? it->second.get() : nullptr;
    }
    
    Material* get_mut(IdType id) {
        auto it = materials.find(id);
        return it != materials.end() ? it->second.get() : nullptr;
    }
    
    const Workpiece* get_workpiece(IdType id) const {
        const Material* mat = get(id);
        return mat && mat->type == MaterialType::WORKPIECE 
            ? static_cast<const Workpiece*>(mat) : nullptr;
    }
    
    Workpiece* get_workpiece_mut(IdType id) {
        Material* mat = get_mut(id);
        return mat && mat->type == MaterialType::WORKPIECE 
            ? static_cast<Workpiece*>(mat) : nullptr;
    }
    
    const Tool* get_tool(IdType id) const {
        const Material* mat = get(id);
        return mat && mat->type == MaterialType::TOOL 
            ? static_cast<const Tool*>(mat) : nullptr;
    }
    
    Tool* get_tool_mut(IdType id) {
        Material* mat = get_mut(id);
        return mat && mat->type == MaterialType::TOOL 
            ? static_cast<Tool*>(mat) : nullptr;
    }
    
    void remove(IdType id) {
        materials.erase(id);
    }
    
    size_t size() const {
        return materials.size();
    }
    
    std::vector<IdType> get_all_ids() const {
        std::vector<IdType> ids;
        ids.reserve(materials.size());
        for (const auto& pair : materials) {
            ids.push_back(pair.first);
        }
        return ids;
    }
    
    std::vector<IdType> get_workpiece_ids() const {
        std::vector<IdType> ids;
        for (const auto& pair : materials) {
            if (pair.second->type == MaterialType::WORKPIECE) {
                ids.push_back(pair.first);
            }
        }
        return ids;
    }
    
    std::vector<IdType> get_tool_ids() const {
        std::vector<IdType> ids;
        for (const auto& pair : materials) {
            if (pair.second->type == MaterialType::TOOL) {
                ids.push_back(pair.first);
            }
        }
        return ids;
    }
    
private:
    std::unordered_map<IdType, std::unique_ptr<Material>> materials;
    IdType next_id = 1;
};

} // namespace fms