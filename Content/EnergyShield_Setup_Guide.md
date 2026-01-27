# 能量罩击碎效果设置指南

## 概述
这是一个完整的能量罩击碎效果实现，包括C++代码、材质设置和粒子系统配置。

## 1. 编译项目
1. 在UE5编辑器中，关闭编辑器
2. 右键点击 `VibeCoding.uproject`，选择"Generate Visual Studio project files"
3. 打开 `VibeCoding.sln`，编译项目
4. 重新打开UE5编辑器

## 2. 创建材质设置

### 2.1 创建能量罩材质（M_EnergyShield）

**材质设置：**
1. 打开材质编辑器
2. 点击左侧的材质节点（材质球图标）
3. 在右侧 **Details** 面板的 **Material** 部分设置：
   - **Shading Model**: **Unlit**（无光照，自发光效果）
   - **Blend Mode**: 
     - 方法1：直接在下拉菜单中选择 **Translucent**
     - 方法2：如果找不到 Translucent，先连接 **Opacity** 节点，材质会自动变为透明模式
     - 方法3：使用 **Additive** 混合模式（适合发光能量效果）
   - **Two Sided**: 勾选（双面渲染，能量罩需要）
   - **Cast Shadow**: 取消勾选（能量罩通常不投射阴影）

**材质节点图：**

```
1. 基础颜色（Base Color）:
   - 使用 Constant3Vector: (0.2, 0.5, 1.0) - 蓝色能量
   - 连接到 Base Color

2. 透明度（Opacity）:
   - 创建 Scalar Parameter: "Opacity" (默认值 0.8)
   - 连接到 Opacity

3. 能量流动效果:
   - 使用 Time 节点
   - 使用 Noise 纹理（或 Simple Noise）
   - 使用 Panner 节点创建流动效果
   - 连接到 Emissive Color，增强发光效果

4. 边缘发光:
   - 使用 Fresnel 节点
   - 连接到 Emissive Color，增强边缘亮度
   - 颜色: (0.0, 0.8, 1.0) - 亮蓝色

5. 破碎效果:
   - 创建 Scalar Parameter: "BreakProgress" (0-1)
   - 使用 Noise 纹理创建破碎裂纹
   - 使用 BreakProgress 控制裂纹显示
   - 连接到 Opacity，让破碎区域透明

6. 能量强度:
   - 创建 Scalar Parameter: "ShieldStrength" (0-1)
   - 连接到 Emissive Color 的强度
```

**材质参数：**
- `Opacity`: 透明度 (0-1)
- `BreakProgress`: 破碎进度 (0-1)
- `ShieldStrength`: 能量强度 (0-1)
- `EnergyFlowSpeed`: 能量流动速度

### 2.2 创建材质参数集合（MPC_EnergyShield）

**创建 Material Parameter Collection:**
1. Content Browser → 右键 → Material & Textures → Material Parameter Collection
2. 命名为 `MPC_EnergyShield`
3. 添加以下标量参数：
   - `BreakProgress` (Float, 默认值 0.0)
   - `ShieldStrength` (Float, 默认值 1.0)

## 3. 创建粒子系统

### 3.1 破碎粒子效果（P_ShieldBreak）

**使用Niagara系统：**
1. Content Browser → 右键 → FX → Niagara System
2. 选择 "Empty System"
3. 命名为 `NS_ShieldBreak`

**粒子设置：**
- **Emitter Update:**
  - Spawn Rate: 1000 (初始爆发)
  - Lifetime: 0.5-1.0秒

- **Particle Spawn:**
  - Initial Velocity: 随机方向，速度 500-1000
  - Initial Color: 蓝色到白色渐变 (0.2, 0.5, 1.0) → (1.0, 1.0, 1.0)
  - Initial Size: 2-5

- **Particle Update:**
  - Color Over Life: 从亮到暗
  - Size Over Life: 从大到小
  - Velocity: 添加重力或阻力
  - Alpha Over Life: 淡出效果

### 3.2 能量流动粒子（NS_EnergyFlow）

**设置：**
- **Emitter Type:** Mesh Renderer 或 Sprite Renderer
- **Spawn Rate:** 持续生成，50-100/秒
- **Lifetime:** 2-3秒
- **Initial Velocity:** 沿球面切线方向
- **Color:** 蓝色能量 (0.0, 0.6, 1.0)
- **Size:** 1-3
- **Material:** 使用发光材质

## 4. 设置网格

1. 在场景中放置一个 Sphere 或自定义网格
2. 推荐使用 Sphere，半径 100-200
3. 将 `M_EnergyShield` 材质应用到网格

## 5. 蓝图设置

### 5.1 创建蓝图类

1. Content Browser → 右键 → Blueprint Class
2. 选择 `EnergyShield` 作为父类
3. 命名为 `BP_EnergyShield`

### 5.2 配置蓝图

1. **设置组件：**
   - Shield Mesh: 分配你的球体网格和材质
   - Collision Sphere: 设置合适的半径
   - Break Particles: 分配 `NS_ShieldBreak`
   - Energy Flow Particles: 分配 `NS_EnergyFlow`
   - Shield Material Params: 分配 `MPC_EnergyShield`

2. **添加输入事件（可选）：**
   - 在 Event Graph 中添加键盘输入（如按空格键触发击碎）
   - 连接到 `Start Break` 函数

## 6. 使用示例

### 在关卡中使用：

1. 将 `BP_EnergyShield` 拖入场景
2. 调整位置和大小
3. 运行游戏
4. 可以通过以下方式触发击碎：
   - 碰撞其他物体
   - 调用 `Start Break` 函数
   - 使用蓝图输入事件

### C++ 调用示例：

```cpp
// 在角色或其他Actor中
AEnergyShield* Shield = GetWorld()->SpawnActor<AEnergyShield>(ShieldClass, Location, Rotation);
Shield->StartBreak(); // 触发击碎效果
```

## 7. 优化建议

1. **性能优化：**
   - 使用 LOD 降低远距离细节
   - 限制粒子数量
   - 使用 Instanced Static Mesh 如果有多个能量罩

2. **视觉效果增强：**
   - 添加屏幕空间效果（后处理）
   - 添加音效
   - 添加屏幕震动
   - 添加慢动作效果

3. **材质优化：**
   - 使用 Material Functions 复用节点
   - 优化纹理采样
   - 使用 Material Parameter Collection 统一控制

## 8. 效果调整参数

在 `BP_EnergyShield` 中可以调整：
- **Break Duration**: 击碎动画时长（默认1.5秒）
- **Shield Strength**: 能量罩强度（0-1）
- 粒子系统参数
- 材质参数

## 9. 故障排除

**问题：材质不显示**
- 检查材质 Blend Mode 是否为 Translucent
- 检查 Opacity 参数是否正确设置

**问题：粒子不出现**
- 检查粒子系统是否已分配
- 检查粒子系统是否在正确的组件上

**问题：击碎效果不工作**
- 检查 Material Parameter Collection 是否已分配
- 检查材质参数名称是否匹配
