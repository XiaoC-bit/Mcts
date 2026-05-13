# MCTS Flexible Manufacturing System (FMS) Simulator

基于蒙特卡洛树搜索(MCTS)的柔性制造系统仿真框架，用于机器人动作调度优化。

## 项目概述

本项目实现了一个可扩展的柔性制造系统(FMS)仿真环境，支持：
- 多机器人协同调度
- 多工位机床加工
- 物料存储与搬运
- MCTS智能决策

## 核心功能

### 1. 实体建模
- **机器人系统**: 支持多夹爪、负载能力、第七轴
- **机床系统**: 多工作台并行加工、刀库管理
- **料架系统**: 2D网格存储、动态容量扩展
- **刀架系统**: 刀具分类管理、寿命跟踪
- **端口系统**: 入料口/出料口、批量处理

### 2. 状态管理
- 统一位置抽象(Location)
- 物料跟踪系统
- 事件驱动仿真
- 可深拷贝状态

### 3. MCTS集成
- Selection/Expansion/Simulation/Backpropagation
- 动作序列预测
- 树结构可视化
- 自定义评价函数

## 文件结构

```
Mcts/
├── Mcts/
│   ├── fms_types.hpp/cpp      # 基础类型定义
│   ├── location.hpp          # 统一位置系统
│   ├── material.hpp          # 物料系统
│   ├── robot.hpp             # 机器人系统
│   ├── machine.hpp           # 机床系统
│   ├── rack.hpp              # 料架/刀架
│   ├── port.hpp              # 入料口/出料口
│   ├── event.hpp             # 事件系统
│   ├── action.hpp            # 动作抽象
│   ├── state.hpp             # 状态类
│   ├── simulator.hpp/cpp     # 仿真器
│   ├── id_constants.hpp/cpp  # ID常量定义
│   ├── mcts_interface.hpp    # MCTS接口
│   ├── mcts_node.hpp         # MCTS节点
│   ├── mcts_core.hpp         # MCTS核心算法
│   └── mcts.cpp              # 主程序入口
└── .gitignore                # Git忽略配置
```

## 编译与运行

### 编译
```bash
g++ -std=c++17 mcts.cpp simulator.cpp fms_types.cpp id_constants.cpp -o mcts.exe
```

### 运行
```bash
./mcts.exe
```

## 使用示例

```cpp
#include "state.hpp"
#include "simulator.hpp"
#include "mcts_core.hpp"

// 创建状态
fms::State state;

// 添加设备
state.create_robot("Robot1", {0, 0, 0}, 2);
state.create_machine("Machine1", {5, 0, 0}, 2, 5);
state.create_rack("Storage", {10, 0, 0}, 5, 5);

// 创建入出料口
fms::IdType input_port = state.create_input_port("Input", {-5, 0, 0}, 3);
fms::IdType output_port = state.create_output_port("Output", {20, 0, 0}, 3);

// 创建工件
fms::IdType wp1 = state.create_workpiece("WP001", "drilling", 1);

// 放置工件到入料口
state.get_input_port(input_port)->get_slot(1)->place_material(wp1);

// 运行MCTS
fms::Simulator simulator;
fms::FMSSimulatorAdapter adapter(simulator);
auto result = mcts_plan(state, adapter, 100);

// 获取结果
std::cout << "Best action: " << result.next_action.to_string() << std::endl;
std::cout << "Score: " << result.estimated_score << std::endl;
```

## MCTS输出示例

```
MCTS Search Tree:
Root Node [n=100, v=85.5, avg=0.855]
├── [n=34, v=32.2, avg=0.947] Robot_1 PICK from Port_1_Slot_1
│   └── [n=12, v=11.3, avg=0.942] Robot_1 LOAD_MACHINE WP_1 to Machine_1
├── [n=33, v=30.8, avg=0.933] Robot_1 PICK from Port_1_Slot_2
│   └── [n=11, v=10.2, avg=0.927] Robot_1 PLACE WP_2 to Rack_1_Slot_1
└── [n=33, v=30.1, avg=0.912] Robot_1 WAIT for 1.0s

Best action: Robot_1 PICK from Port_1_Slot_1
Estimated score: 0.947
Predicted sequence (3 steps):
  Step 1: Robot_1 PICK from Port_1_Slot_1
  Step 2: Robot_1 LOAD_MACHINE WP_1 to Machine_1
  Step 3: Robot_1 PICK from Port_1_Slot_2
```

## 评价函数设计

评分公式(归一化到[0,1])：

| 项 | 权重 | 说明 |
|-----|------|------|
| PROCESSING_BONUS | +100.0 | 正在加工的工件 |
| LOADED_READY_BONUS | +30.0 | 已装载待加工 |
| IDLE_MACHINE_PENALTY | -20.0 | 空闲工作台 |
| WORKPIECE_ON_ROBOT_PENALTY | -15.0 | 机器人持有工件 |
| WORKPIECE_ON_RACK_PENALTY | -10.0 | 料架上的工件 |
| UTILIZATION_BONUS | +50.0 | 机床利用率 |
| PROCESSING_ADVANCEMENT_BONUS | +20.0 | 加工进度 |

## 扩展指南

### 添加新动作类型
```cpp
// 在 action.hpp 中添加新的 ActionType
enum class ActionType {
    PICK, PLACE, LOAD_MACHINE, UNLOAD_MACHINE, CHANGE_TOOL, WAIT,
    YOUR_NEW_ACTION  // 添加新动作
};

// 在 simulator.cpp 中实现动作逻辑
void Simulator::execute_your_action(State& state, const Action& action) const {
    // 实现逻辑
}
```

### 实现自定义评价函数
```cpp
class MySimulator : public fms::Simulator {
    double evaluate(const State& state) const override {
        // 自定义评分逻辑
        return custom_score;
    }
};
```

## 技术说明

### ID编码规则
- 机器人ID: 1, 2, 3...
- 机器人夹爪ID: robot_id × 100 + index
- 机床ID: 1, 2, 3...
- 机床工作台ID: machine_id × 2000 + index
- 机床刀槽ID: machine_id × 10000 + index
- 料架ID: 1, 2, 3...
- 料架槽位ID: rack_id × 1000 + index
- 端口ID: 1, 2, 3...
- 端口槽位ID: port_id × 200 + index

### 终止条件
当满足以下任一条件时视为终止状态：
1. 没有待加工工件
2. 所有机床工作台都在加工中

## 依赖
- C++17 或更高版本
- 无外部依赖

## 许可证
MIT License

## 作者
MCTS FMS Development Team