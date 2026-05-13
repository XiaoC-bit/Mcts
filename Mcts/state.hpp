#pragma once
#include "fms_types.hpp"
#include "robot.hpp"
#include "machine.hpp"
#include "rack.hpp"
#include "port.hpp"
#include "material.hpp"
#include "event.hpp"
#include "location.hpp"
#include "id_constants.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace fms {

class State {
public:
    TimeType current_time;
    std::vector<Robot> robots;
    std::vector<Machine> machines;
    std::vector<Rack> racks;
    std::vector<ToolRack> tool_racks;
    std::vector<Port> input_ports;
    std::vector<Port> output_ports;
    std::unordered_map<IdType, std::unique_ptr<Material>> materials;
    std::vector<Location> locations;
    EventQueue event_queue;
    
    State() : current_time(0.0) {}
    
    State(const State& other) 
        : current_time(other.current_time),
          robots(other.robots),
          machines(other.machines),
          racks(other.racks),
          tool_racks(other.tool_racks),
          input_ports(other.input_ports),
          output_ports(other.output_ports),
          locations(other.locations),
          event_queue(other.event_queue) {
        for (const auto& pair : other.materials) {
            if (pair.second->type == MaterialType::WORKPIECE) {
                materials[pair.first] = std::make_unique<Workpiece>(
                    *static_cast<const Workpiece*>(pair.second.get())
                );
            } else {
                materials[pair.first] = std::make_unique<Tool>(
                    *static_cast<const Tool*>(pair.second.get())
                );
            }
        }
    }
    
    State& operator=(const State& other) {
        if (this != &other) {
            current_time = other.current_time;
            robots = other.robots;
            machines = other.machines;
            racks = other.racks;
            tool_racks = other.tool_racks;
            input_ports = other.input_ports;
            output_ports = other.output_ports;
            locations = other.locations;
            event_queue = other.event_queue;
            
            materials.clear();
            for (const auto& pair : other.materials) {
                if (pair.second->type == MaterialType::WORKPIECE) {
                    materials[pair.first] = std::make_unique<Workpiece>(
                        *static_cast<const Workpiece*>(pair.second.get())
                    );
                } else {
                    materials[pair.first] = std::make_unique<Tool>(
                        *static_cast<const Tool*>(pair.second.get())
                    );
                }
            }
        }
        return *this;
    }
    
    State(State&& other) = default;
    State& operator=(State&& other) = default;
    
    IdType create_robot(const std::string& name, const Position& pos, 
                       size_t num_grippers = 1, bool seventh_axis = false) {
        IdType id = static_cast<IdType>(robots.size() + 1);
        robots.emplace_back(id, name, pos, num_grippers, seventh_axis);
        return id;
    }
    
    IdType create_machine(const std::string& name, const Position& pos,
                         size_t num_tables = 1, size_t num_tool_slots = 10) {
        IdType id = static_cast<IdType>(machines.size() + 1);
        machines.emplace_back(id, name, pos, num_tables, num_tool_slots);
        return id;
    }
    
    IdType create_rack(const std::string& name, const Position& pos,
                      size_t rows = 5, size_t cols = 5) {
        IdType id = static_cast<IdType>(racks.size() + 1);
        racks.emplace_back(id, name, pos, rows, cols);
        return id;
    }
    
    IdType create_tool_rack(const std::string& name, const Position& pos,
                           size_t rows = 3, size_t cols = 5) {
        IdType id = static_cast<IdType>(tool_racks.size() + 1);
        tool_racks.emplace_back(id, name, pos, rows, cols);
        return id;
    }
    
    IdType create_input_port(const std::string& name, const Position& pos, CapacityType capacity = 5) {
        IdType id = static_cast<IdType>(input_ports.size() + 1);
        input_ports.emplace_back(id, name, pos, LocationType::INPUT_PORT, capacity);
        return id;
    }
    
    IdType create_output_port(const std::string& name, const Position& pos, CapacityType capacity = 5) {
        IdType id = static_cast<IdType>(output_ports.size() + 1);
        output_ports.emplace_back(id, name, pos, LocationType::OUTPUT_PORT, capacity);
        return id;
    }
    
    IdType create_workpiece(const std::string& name, const std::string& process_type, 
                           uint32_t steps = 1) {
        IdType id = static_cast<IdType>(materials.size() + 1);
        materials[id] = std::make_unique<Workpiece>(id, name, process_type, steps);
        return id;
    }
    
    IdType create_tool(const std::string& name, const std::string& category, 
                      uint32_t life = id_constants::DEFAULT_TOOL_LIFE) {
        IdType id = static_cast<IdType>(materials.size() + 1);
        materials[id] = std::make_unique<Tool>(id, name, category, life);
        return id;
    }
    
    IdType add_location(LocationType type, const std::string& name, const Position& pos,
                       IdType parent_id = INVALID_ID, CapacityType capacity = 1) {
        IdType id = static_cast<IdType>(locations.size() + 1);
        locations.emplace_back(id, type, name, pos, parent_id, capacity);
        return id;
    }
    
    Robot* get_robot(IdType id) {
        for (auto& robot : robots) {
            if (robot.id == id) return &robot;
        }
        return nullptr;
    }
    
    const Robot* get_robot(IdType id) const {
        for (const auto& robot : robots) {
            if (robot.id == id) return &robot;
        }
        return nullptr;
    }
    
    Machine* get_machine(IdType id) {
        for (auto& machine : machines) {
            if (machine.id == id) return &machine;
        }
        return nullptr;
    }
    
    const Machine* get_machine(IdType id) const {
        for (const auto& machine : machines) {
            if (machine.id == id) return &machine;
        }
        return nullptr;
    }
    
    Rack* get_rack(IdType id) {
        for (auto& rack : racks) {
            if (rack.id == id) return &rack;
        }
        return nullptr;
    }
    
    const Rack* get_rack(IdType id) const {
        for (const auto& rack : racks) {
            if (rack.id == id) return &rack;
        }
        return nullptr;
    }
    
    ToolRack* get_tool_rack(IdType id) {
        for (auto& rack : tool_racks) {
            if (rack.id == id) return &rack;
        }
        return nullptr;
    }
    
    const ToolRack* get_tool_rack(IdType id) const {
        for (const auto& rack : tool_racks) {
            if (rack.id == id) return &rack;
        }
        return nullptr;
    }
    
    Port* get_input_port(IdType id) {
        for (auto& port : input_ports) {
            if (port.id == id) return &port;
        }
        return nullptr;
    }
    
    const Port* get_input_port(IdType id) const {
        for (const auto& port : input_ports) {
            if (port.id == id) return &port;
        }
        return nullptr;
    }
    
    Port* get_output_port(IdType id) {
        for (auto& port : output_ports) {
            if (port.id == id) return &port;
        }
        return nullptr;
    }
    
    const Port* get_output_port(IdType id) const {
        for (const auto& port : output_ports) {
            if (port.id == id) return &port;
        }
        return nullptr;
    }
    
    Material* get_material(IdType id) {
        auto it = materials.find(id);
        return it != materials.end() ? it->second.get() : nullptr;
    }
    
    const Material* get_material(IdType id) const {
        auto it = materials.find(id);
        return it != materials.end() ? it->second.get() : nullptr;
    }
    
    Workpiece* get_workpiece(IdType id) {
        Material* mat = get_material(id);
        return mat && mat->type == MaterialType::WORKPIECE 
            ? static_cast<Workpiece*>(mat) : nullptr;
    }
    
    const Workpiece* get_workpiece(IdType id) const {
        const Material* mat = get_material(id);
        return mat && mat->type == MaterialType::WORKPIECE 
            ? static_cast<const Workpiece*>(mat) : nullptr;
    }
    
    Tool* get_tool(IdType id) {
        Material* mat = get_material(id);
        return mat && mat->type == MaterialType::TOOL 
            ? static_cast<Tool*>(mat) : nullptr;
    }
    
    const Tool* get_tool(IdType id) const {
        const Material* mat = get_material(id);
        return mat && mat->type == MaterialType::TOOL 
            ? static_cast<const Tool*>(mat) : nullptr;
    }
    
    Location* get_location(IdType id) {
        for (auto& loc : locations) {
            if (loc.id == id) return &loc;
        }
        return nullptr;
    }
    
    const Location* get_location(IdType id) const {
        for (const auto& loc : locations) {
            if (loc.id == id) return &loc;
        }
        return nullptr;
    }
    
    size_t get_num_idle_robots() const {
        size_t count = 0;
        for (const auto& robot : robots) {
            if (robot.is_idle(current_time)) {
                count++;
            }
        }
        return count;
    }
    
    size_t get_num_idle_machine_tables() const {
        size_t count = 0;
        for (const auto& machine : machines) {
            for (const auto& table : machine.tables) {
                if (table.is_idle()) {
                    count++;
                }
            }
        }
        return count;
    }
    
    bool has_pending_workpieces() const {
        for (const auto& pair : materials) {
            const Workpiece* wp = dynamic_cast<const Workpiece*>(pair.second.get());
            if (wp && !wp->is_processed) {
                return true;
            }
        }
        return false;
    }
    
    size_t count_processed_workpieces() const {
        size_t count = 0;
        for (const auto& pair : materials) {
            const Workpiece* wp = dynamic_cast<const Workpiece*>(pair.second.get());
            if (wp && wp->is_processed) {
                count++;
            }
        }
        return count;
    }
};

} // namespace fms