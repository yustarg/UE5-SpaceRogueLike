# 能量罩击碎效果 - 快速开始指南

## 快速设置（5分钟）

### 步骤1: 编译项目
1. 关闭UE5编辑器
2. 右键 `VibeCoding.uproject` → "Generate Visual Studio project files"
3. 打开 `VibeCoding.sln` 编译（或直接在UE5中编译）

### 步骤2: 创建基础材质（最简单版本）

1. **创建材质 `M_EnergyShield`**
   - Content Browser → 右键 → Material
   - 双击打开材质编辑器
   - 在材质编辑器中，点击材质节点（左侧的材质球图标）
   - 在右侧 Details 面板中找到 **Material** 部分
   - 设置 **Blend Mode**：
     - 如果看到下拉菜单，选择 **Translucent**（透明）
     - 如果没有 Translucent，尝试：
       - **Additive**（加法混合，适合发光效果）
       - 或者先设置为 **Opaque**，然后连接 Opacity 节点后会自动变为透明
   - 设置 **Shading Model**: **Unlit**（无光照，适合自发光）
   - **重要**：如果使用 Unlit，确保连接了 **Emissive Color** 节点

2. **材质节点完整连接（重要！）：**
   
   **步骤A：创建所有参数**
   - 右键空白处 → **Parameters** → **Scalar Parameter**
     - 创建三个参数：
       - `Opacity` (默认值 0.8)
       - `BreakProgress` (默认值 0.0)
       - `ShieldStrength` (默认值 1.0)
   
   **步骤B：连接 Base Color**
   - 右键空白处 → **Constants** → **Constant3Vector**
     - 设置颜色为 (0.2, 0.5, 1.0) - 蓝色
     - 连接到材质节点的 **Base Color** 输入
   
   **步骤C：连接 Emissive Color（发光效果）**
   - 右键空白处 → **Constants** → **Constant3Vector**
     - 设置颜色为 (0.0, 0.8, 1.0) - 亮蓝色
   
   - 右键空白处 → **Math** → **Multiply**（乘法节点）
     - 将 Constant3Vector(0.0, 0.8, 1.0) 连接到 Multiply 的 **A** 输入
     - 将 `ShieldStrength` 参数连接到 Multiply 的 **B** 输入
     - 将 Multiply 的输出连接到材质节点的 **Emissive Color** 输入
   
   **这样 ShieldStrength 就能控制发光强度了！**
   
   **步骤D：连接 Opacity（透明度，包含破碎效果）**
   - 右键空白处 → **Math** → **OneMinus**（1 - X 节点）
     - 将 `BreakProgress` 参数连接到 OneMinus 的输入
     - OneMinus 输出 = (1 - BreakProgress)
   
   - 右键空白处 → **Math** → **Multiply**
     - 将 `Opacity` 参数连接到 Multiply 的 **A** 输入
     - 将 OneMinus 的输出连接到 Multiply 的 **B** 输入
     - 将 Multiply 的输出连接到材质节点的 **Opacity** 输入
   
   **这样当 BreakProgress 从 0 增加到 1 时，透明度会逐渐降低！**
   
   **完整连接总结：**
   ```
   Base Color ← Constant3Vector(0.2, 0.5, 1.0)
   
   Emissive Color ← Constant3Vector(0.0, 0.8, 1.0) × ShieldStrength
   
   Opacity ← Opacity × (1 - BreakProgress)
   ```
   
   **注意**：连接 Opacity 节点后，材质会自动变为透明模式

### 步骤3: 创建蓝图

1. **创建蓝图类**
   - Content Browser → 右键 → Blueprint Class
   - 父类选择: **EnergyShield**
   - 命名为 `BP_EnergyShield`

2. **在蓝图中设置：**
   - **Shield Mesh**: 
     - 添加 Static Mesh Component（如果还没有）
     - Mesh: 使用 Sphere（在 Starter Content 中）
     - Material: 分配 `M_EnergyShield`
   
   - **Collision Sphere**: 
     - 设置 Radius = 100
   
   - **Break Particles**: 
     - 可以先留空，或使用默认粒子系统测试
   
   - **Energy Flow Particles**: 
     - 可以先留空

### 步骤4: 测试

1. 将 `BP_EnergyShield` 拖入场景
2. 运行游戏
3. 在蓝图中添加输入事件：
   - Event Graph → 右键 → Input → Keyboard Events → Space Bar
   - 连接到 `Start Break` 函数

## 快速测试脚本（蓝图）

在 `BP_EnergyShield` 的 Event Graph 中添加：

```
Event BeginPlay
  └─> Delay (2.0秒)
      └─> Start Break
```

这样会在游戏开始2秒后自动触发击碎效果。

## 视觉效果增强（可选）

### 添加简单粒子效果：

1. **使用UE5内置粒子：**
   - 在 Content Browser 搜索 "P_Explosion" 或类似效果
   - 分配给 Break Particles

2. **或创建简单Niagara系统：**
   - Content Browser → FX → Niagara System → Empty
   - 添加 Emitter → 设置基本参数即可

### 材质增强（进阶）：

在材质中添加：
- **Fresnel Effect**（边缘发光）
- **Noise Texture**（能量流动）
- **Time-based Animation**（动态效果）

详细设置请参考 `EnergyShield_Setup_Guide.md`

## 常见问题

**Q: 材质不透明？**
A: 
   - 确保连接了 **Opacity** 节点到材质（连接后会自动变为透明）
   - 检查 Opacity 参数值是否 > 0
   - 如果还是没有透明效果，尝试：
     1. 在材质 Details 面板中手动设置 Blend Mode
     2. 或者使用 **Additive** 混合模式（适合发光效果）
     3. 确保材质已保存并应用到网格上

**Q: 找不到 Blend Mode 选项？**
A: 
   - 在材质编辑器中，点击左侧的材质球节点
   - 在右侧 Details 面板的 **Material** 分类下查找
   - 或者尝试搜索 "Blend" 在 Details 面板中
   - 某些UE5版本中，连接 Opacity 节点后会自动切换混合模式

**Q: 击碎效果不工作？**
A: 
   - 检查材质参数名称是否匹配（BreakProgress, Opacity, ShieldStrength）
   - **重要**：确保参数已连接到材质节点，不仅仅是创建了参数！
   - 检查 BreakProgress 是否通过 OneMinus 和 Multiply 连接到 Opacity
   - 检查 ShieldStrength 是否通过 Multiply 连接到 Emissive Color
   - 在材质编辑器中选中参数，在 Details 面板修改默认值测试效果

**Q: 参数创建了但没有效果？**
A: 
   - **必须将参数连接到材质输入！** 仅仅创建参数是不够的
   - 参考上面的"步骤C"和"步骤D"完整连接
   - 详细连接图请参考 `EnergyShield_Complete_Material.md`

**Q: 如何调整击碎速度？**
A: 在蓝图 Details 面板中调整 `Break Duration` 参数

## 展示建议

1. **多角度展示：** 设置多个摄像机角度
2. **慢动作：** 使用 Time Dilation 让效果更明显
3. **背景：** 使用暗色背景突出能量效果
4. **循环：** 设置自动重置，形成循环展示
