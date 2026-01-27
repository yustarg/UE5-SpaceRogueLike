# 能量罩击碎效果 - UE5.7 完整实现指南

## 目录
1. [项目编译](#1-项目编译)
2. [创建材质](#2-创建材质)
3. [创建蓝图](#3-创建蓝图)
4. [场景设置](#4-场景设置)
5. [测试与调试](#5-测试与调试)
6. [添加粒子效果](#6-添加粒子效果)

---

## 1. 项目编译

### 1.1 编译C++代码

**步骤：**

1. **关闭UE5编辑器**（如果正在运行）

2. **生成Visual Studio项目文件**
   - 找到 `VibeCoding.uproject` 文件
   - 右键点击 → 选择 **"Generate Visual Studio project files"**
   - 等待生成完成

3. **打开解决方案**
   - 双击打开 `VibeCoding.sln`
   - Visual Studio会打开

4. **编译项目**
   - 在Visual Studio中，设置编译配置为：
     - **Configuration**: `Development Editor`
     - **Platform**: `Win64`
   - 菜单栏 → **生成** → **生成解决方案** (或按 `Ctrl+Shift+B`)
   - 等待编译完成（可能需要几分钟）

5. **启动编辑器**
   - 编译成功后，双击 `VibeCoding.uproject` 打开UE5编辑器
   - 或者在Visual Studio中按 `F5` 调试启动

**预期结果：**
- 编译无错误
- UE5编辑器成功启动
- 在Content Browser的C++ Classes中可以看到 `EnergyShield` 类

---

## 2. 创建材质

### 2.1 创建新材质

**步骤：**

1. **打开Content Browser**
   - 在UE5编辑器底部找到Content Browser面板
   - 如果看不到，菜单栏 → **Window** → **Content Browser**

2. **创建新材质**
   - 在Content Browser中，右键点击空白处
   - 选择 **Material** (在弹出菜单中找到)
   - 命名为：`M_EnergyShield`
   - 按 `Enter` 确认

3. **打开材质编辑器**
   - 双击 `M_EnergyShield` 材质资源
   - 材质编辑器窗口会打开

### 2.2 配置材质基础设置

**步骤：**

1. **选中材质主节点**
   - 在材质编辑器左侧，找到灰色的材质节点（写着 "M_EnergyShield"）
   - 点击选中它

2. **打开Details面板**
   - 如果右侧没有Details面板，按 `Ctrl+Shift+D`
   - 或菜单栏 → **Window** → **Details**

3. **设置Shading Model**
   - 在Details面板中找到 **Material** 分类
   - 找到 **Shading Model** 下拉菜单
   - 选择 **Unlit**（无光照模型，用于自发光效果）

4. **设置Blend Mode（重要！）**
   - 在同一个 **Material** 分类中
   - 找到 **Blend Mode** 下拉菜单
   - 有以下几种选择：
     - **方法1（推荐）**：暂时保持 **Opaque**（不透明），稍后连接 Opacity 节点时会自动切换为透明模式
     - **方法2**：直接选择 **Translucent**（透明）- 标准透明效果
     - **方法3**：选择 **Additive**（加法混合）- 更适合发光能量效果，不需要设置 Opacity
   - **建议**：先保持 Opaque，等到步骤2.6连接 Opacity 节点时会自动切换

5. **设置Two Sided**
   - 在同一个 **Material** 分类中
   - 找到 **Two Sided** 复选框
   - **勾选**它（让能量罩双面可见）

6. **关闭Cast Shadow**
   - 找到 **Cast Shadow** 复选框
   - **取消勾选**（能量罩不需要投射阴影）

**此时的材质设置摘要：**
- Shading Model: **Unlit**
- Blend Mode: **Opaque**（或 Translucent/Additive，见上面说明）
- Two Sided: **勾选**
- Cast Shadow: **取消勾选**

### 2.3 创建材质参数

**步骤：**

1. **创建 Opacity 参数**
   - 在材质编辑器的空白处右键
   - 选择 **Parameters** → **Scalar Parameter**
   - 新节点出现
   - 选中这个节点，在Details面板中：
     - **Parameter Name**: 输入 `Opacity`
     - **Default Value**: 输入 `0.8`
   - 将这个节点拖到材质节点左侧备用

2. **创建 BreakProgress 参数**
   - 再次右键空白处
   - 选择 **Parameters** → **Scalar Parameter**
   - 在Details面板中：
     - **Parameter Name**: 输入 `BreakProgress`
     - **Default Value**: 输入 `0.0`
   - 将这个节点拖到材质节点左侧备用

3. **创建 ShieldStrength 参数**
   - 再次右键空白处
   - 选择 **Parameters** → **Scalar Parameter**
   - 在Details面板中：
     - **Parameter Name**: 输入 `ShieldStrength`
     - **Default Value**: 输入 `1.0`
   - 将这个节点拖到材质节点左侧备用

**检查点：现在你应该有3个参数节点**

### 2.4 连接 Base Color（基础颜色）

**步骤：**

1. **创建颜色常量**
   - 右键空白处
   - 选择 **Constants** → **Constant3Vector**
   - 双击这个节点（或选中后在Details中）
   - 设置颜色：
     - **R**: `0.2`
     - **G**: `0.5`
     - **B**: `1.0`
   - 这会创建一个蓝色

2. **连接到Base Color**
   - 点击 Constant3Vector 节点右侧的**输出引脚**（白色圆点）
   - 拖动到材质主节点左侧的 **Base Color** 输入引脚
   - 松开鼠标，连线完成

**检查点：材质主节点的Base Color引脚应该有一条线连接**

### 2.5 连接 Emissive Color（发光颜色，关键！）

**步骤：**

1. **创建发光颜色常量**
   - 右键空白处
   - 选择 **Constants** → **Constant3Vector**
   - 设置颜色：
     - **R**: `0.0`
     - **G**: `0.8`
     - **B**: `1.0`
   - 这会创建一个亮蓝色

2. **创建乘法节点**
   - 右键空白处
   - 选择 **Math** → **Multiply**
   - 一个写着 "×" 的节点出现

3. **连接节点**
   - 将 **Constant3Vector(0.0, 0.8, 1.0)** 的输出 → 连接到 **Multiply** 的 **A** 输入
   - 将 **ShieldStrength** 参数的输出 → 连接到 **Multiply** 的 **B** 输入
   - 将 **Multiply** 的输出 → 连接到材质主节点的 **Emissive Color** 输入

**连接图：**
```
Constant3Vector(0.0, 0.8, 1.0) ──→ [Multiply A]
ShieldStrength ────────────────→ [Multiply B]
                                   [Multiply 输出] ──→ Emissive Color
```

**检查点：材质主节点的Emissive Color引脚应该有连线，预览应该显示蓝色发光**

### 2.6 连接 Opacity（透明度 + 破碎效果）

**步骤：**

1. **创建 OneMinus 节点**
   - 右键空白处
   - 选择 **Math** → **OneMinus** (或搜索 "OneMinus")
   - 这个节点计算 (1 - X)

2. **连接 BreakProgress 到 OneMinus**
   - 将 **BreakProgress** 参数的输出 → 连接到 **OneMinus** 的输入
   - OneMinus 会输出 (1 - BreakProgress)

3. **创建第二个 Multiply 节点**
   - 右键空白处
   - 选择 **Math** → **Multiply**

4. **连接最终 Opacity**
   - 将 **Opacity** 参数的输出 → 连接到 **Multiply** 的 **A** 输入
   - 将 **OneMinus** 的输出 → 连接到 **Multiply** 的 **B** 输入
   - 将 **Multiply** 的输出 → 连接到材质主节点的 **Opacity** 输入

**连接图：**
```
BreakProgress ──→ [OneMinus] ──→ [Multiply B]
Opacity Parameter ──────────→ [Multiply A]
                                [Multiply 输出] ──→ Opacity
```

**重要：连接Opacity后，材质会自动切换混合模式！**

### 2.7 检查材质设置

**步骤：**

1. **检查Blend Mode（关键步骤！）**
   - 选中材质主节点
   - 在Details面板中找到 **Blend Mode**
   - 连接 Opacity 节点后，应该自动变为 **Translucent** 或类似的透明模式
   - 如果没有自动切换，手动选择以下之一：
     - **Translucent**（透明）- 标准透明效果，可以看到后面的物体
     - **Additive**（加法混合）- 推荐！更适合能量效果，发光更明显
   - **推荐使用 Additive** 以获得更好的能量罩视觉效果
   
   **三种模式对比：**
   ```
   Opaque（不透明）     - 完全实心，看不透
   Translucent（透明）  - 标准透明，可以看穿
   Additive（加法）     - 发光叠加，适合能量、火焰等效果
   ```

2. **保存材质**
   - 点击左上角的 **Save** 按钮（或按 `Ctrl+S`）
   - 绿色勾号表示保存成功

3. **测试参数**
   - 选中 **BreakProgress** 参数
   - 在Details面板中修改 **Default Value** 从 0.0 到 1.0
   - 观察材质预览球：应该逐渐变透明
   - 改回 0.0
   - 选中 **ShieldStrength** 参数
   - 修改值从 1.0 到 0.0
   - 观察材质预览球：发光应该减弱
   - 改回 1.0
   - 再次保存（`Ctrl+S`）

**预期结果：**
- 材质预览显示蓝色发光球体
- 边缘有轻微透明效果
- 调整参数时效果相应变化

---

## 3. 创建蓝图

### 3.1 创建蓝图类

**步骤：**

1. **打开Content Browser**

2. **创建蓝图类**
   - 右键空白处
   - 选择 **Blueprint Class**
   - 在弹出的窗口中，点击 **All Classes** 展开所有类别

3. **选择父类**
   - 在搜索框中输入：`EnergyShield`
   - 选择 **EnergyShield** 类（这是我们的C++类）
   - 如果找不到，确保项目已编译成功
   - 点击 **Select**

4. **命名蓝图**
   - 命名为：`BP_EnergyShield`
   - 按 `Enter` 确认

5. **打开蓝图编辑器**
   - 双击 `BP_EnergyShield`

### 3.2 配置蓝图组件

**步骤：**

1. **查看Components面板**
   - 在蓝图编辑器左上角，应该看到 **Components** 面板
   - 你会看到以下组件（由C++代码自动创建）：
     - `RootComponent`
     - `CollisionSphere`
     - `ShieldMesh`
     - `BreakParticles`
     - `EnergyFlowParticles`

### 3.3 设置 Shield Mesh（能量罩网格）

**步骤：**

1. **选中 ShieldMesh 组件**
   - 在Components面板中点击 `ShieldMesh`

2. **设置网格**
   - 在右侧Details面板中找到 **Static Mesh** 属性
   - 点击下拉菜单
   - 搜索并选择：`Sphere`（球体）
   - 如果没有Sphere，使用：`SM_Sphere` 或其他球形网格

3. **应用材质**
   - 在Details面板中找到 **Materials** 部分
   - **Element 0** 下拉菜单
   - 选择我们创建的 `M_EnergyShield`

4. **调整缩放（可选）**
   - 在Details面板中找到 **Transform** → **Scale**
   - 设置为：`X=1.0, Y=1.0, Z=1.0`
   - 或根据需要调整大小

### 3.4 设置 Collision Sphere（碰撞球体）

**步骤：**

1. **选中 CollisionSphere 组件**

2. **设置半径**
   - 在Details面板中找到 **Shape** 部分
   - **Sphere Radius**: 设置为 `100.0`（或与网格大小匹配）

3. **设置碰撞**
   - 找到 **Collision** 部分
   - **Collision Presets**: 选择 `BlockAllDynamic`
   - 或者自定义碰撞设置

### 3.5 设置粒子组件（暂时可选）

**步骤：**

1. **BreakParticles 和 EnergyFlowParticles**
   - 暂时可以留空
   - 稍后在第6步添加粒子效果

### 3.6 编译并保存蓝图

**步骤：**

1. **编译蓝图**
   - 点击左上角的 **Compile** 按钮
   - 等待编译完成（绿色勾号）

2. **保存蓝图**
   - 点击 **Save** 按钮（或按 `Ctrl+S`）

---

## 4. 场景设置

### 4.1 将能量罩放入场景

**步骤：**

1. **打开主关卡**
   - 如果没有关卡，创建一个新关卡：
     - 菜单栏 → **File** → **New Level**
     - 选择 **Empty Level** 或 **Default**

2. **拖动蓝图到场景**
   - 从Content Browser中找到 `BP_EnergyShield`
   - 拖动到视口（Viewport）中
   - 放置在合适的位置

3. **调整位置**
   - 在视口中选中能量罩
   - 使用移动工具（W键）调整位置
   - 建议放在 `Z=100` 左右的高度

### 4.2 添加测试环境（可选）

**步骤：**

1. **添加地面**
   - 从 **Place Actors** 面板（左侧）
   - 搜索 `Plane` 或 `Cube`
   - 拖动到场景作为地面

2. **添加光源**
   - 搜索 `Directional Light`
   - 拖动到场景
   - 调整角度照亮能量罩

3. **添加摄像机**
   - 搜索 `Camera`
   - 拖动到场景
   - 调整位置对准能量罩

### 4.3 设置测试触发

**步骤：**

1. **打开 BP_EnergyShield 蓝图**
   - 双击Content Browser中的 `BP_EnergyShield`

2. **切换到Event Graph**
   - 点击编辑器顶部的 **Event Graph** 标签

3. **添加自动击碎测试**
   - 在Event Graph中已经有 **Event BeginPlay** 节点
   - 右键空白处，搜索 **Delay**
   - 添加 **Delay** 节点，设置 **Duration** = `3.0`（3秒）
   - 连接：`Event BeginPlay` → `Delay`
   - 右键空白处，搜索 **Start Break**
   - 添加 **Start Break** 节点
   - 连接：`Delay` → `Start Break`

**节点连接：**
```
Event BeginPlay → Delay (3.0秒) → Start Break
```

4. **添加键盘触发（可选）**
   - 右键空白处
   - 搜索 **Keyboard Events** → **Space Bar**
   - 添加 **Space Bar** 键盘事件
   - 连接到 **Start Break** 节点

**节点连接：**
```
Space Bar (Pressed) → Start Break
```

5. **添加循环重置（可选）**
   - 右键空白处，搜索 **Event Tick**
   - 右键空白处，搜索 **Get Break Progress**
   - 右键空白处，搜索 **>= (float)**（大于等于）
   - 设置比较值为 `0.99`
   - 添加 **Branch** 节点
   - 如果 True，调用 **Reset Shield** 节点
   - 再添加 **Delay** (2秒)
   - 连接到 **Start Break**

**这会创建自动循环：击碎 → 等待 → 重置 → 重复**

6. **编译并保存**
   - 点击 **Compile**
   - 点击 **Save**

---

## 5. 测试与调试

### 5.1 运行游戏测试

**步骤：**

1. **点击Play按钮**
   - 在编辑器顶部工具栏，点击 **Play** 按钮（或按 `Alt+P`）
   - 游戏会在视口中开始运行

2. **观察效果**
   - 3秒后，能量罩应该开始击碎效果
   - 观察：
     - 透明度逐渐增加
     - 发光逐渐减弱
     - 网格缩小
     - 最后消失

3. **测试键盘输入（如果添加了）**
   - 按 **Space** 键触发击碎

4. **停止游戏**
   - 按 **Esc** 或点击 **Stop** 按钮

### 5.2 调试材质效果

**步骤：**

1. **检查材质显示**
   - 如果能量罩不透明或不发光：
     - 打开 `M_EnergyShield` 材质
     - 检查 Emissive Color 是否已连接
     - 检查 Opacity 是否已连接
     - 检查 Blend Mode 是否为 Translucent

2. **调整材质参数**
   - 在场景中选中能量罩
   - 在Details面板中找到 **Materials** 部分
   - 右键材质 → **Create Material Instance**
   - 在材质实例中可以实时调整参数

### 5.3 调试C++代码（如果需要）

**步骤：**

1. **添加日志输出**
   - 打开 `EnergyShield.cpp`
   - 在 `StartBreak()` 函数中添加：
     ```cpp
     UE_LOG(LogTemp, Warning, TEXT("EnergyShield: StartBreak called"));
     ```
   - 在 `UpdateBreakAnimation()` 中添加：
     ```cpp
     UE_LOG(LogTemp, Warning, TEXT("BreakProgress: %f"), BreakProgress);
     ```

2. **重新编译**
   - 在Visual Studio中重新编译项目

3. **查看日志**
   - 在UE5编辑器中，菜单栏 → **Window** → **Output Log**
   - 运行游戏，查看日志输出

### 5.4 常见问题排查

**问题1：能量罩不显示**
- 检查 ShieldMesh 是否分配了网格
- 检查材质是否应用
- 检查能量罩是否在摄像机视野内

**问题2：材质不透明**
- 检查 Opacity 节点是否连接
- 检查 Blend Mode 是否为 Translucent
- 检查 Opacity 参数默认值（应该 < 1.0）

**问题3：不发光**
- 检查 Emissive Color 是否连接
- 检查 Shading Model 是否为 Unlit
- 检查 ShieldStrength 参数值（应该 > 0）

**问题4：击碎效果不工作**
- 检查 BreakProgress 是否连接到 Opacity
- 检查材质参数名称拼写是否正确
- 查看Output Log是否有错误信息

**问题5：找不到 EnergyShield 类**
- 确保项目已成功编译
- 重新生成项目文件
- 重启UE5编辑器

---

## 6. 添加粒子效果

### 6.1 创建破碎粒子（Niagara系统）

**步骤：**

1. **创建Niagara System**
   - Content Browser → 右键
   - 选择 **FX** → **Niagara System**
   - 选择 **New system from a template**
   - 选择 **Simple Burst** 模板
   - 命名为：`NS_ShieldBreak`

2. **编辑粒子系统**
   - 双击 `NS_ShieldBreak` 打开Niagara编辑器

3. **调整发射器设置**
   - 在左侧找到 **Emitter State**
   - 在右侧找到 **Spawn Rate** 模块
   - 设置 **Spawn Rate** = `1000`（初始爆发）

4. **设置粒子生命周期**
   - 找到 **Particle Spawn** 部分
   - **Lifetime** 模块：设置为 `0.5 - 1.0` 随机范围

5. **设置粒子颜色**
   - 找到 **Initialize Particle** 部分
   - **Color** 模块：
     - 设置为蓝色 `(0.0, 0.6, 1.0)`
     - 或使用 **Curve** 从蓝色渐变到白色

6. **设置粒子大小**
   - **Sprite Size** 模块：
     - 设置 **Uniform Sprite Size** = `5.0 - 10.0`

7. **设置速度**
   - **Add Velocity** 模块：
     - 设置为 **随机方向**
     - 速度范围：`200 - 500`

8. **保存**
   - 按 `Ctrl+S` 保存

### 6.2 创建能量流动粒子

**步骤：**

1. **创建第二个Niagara System**
   - Content Browser → 右键
   - **FX** → **Niagara System**
   - 选择 **Simple Sprite Burst** 或 **Fountain**
   - 命名为：`NS_EnergyFlow`

2. **设置持续发射**
   - **Spawn Rate** = `50`（持续生成）

3. **设置粒子运动**
   - 添加 **Point Attraction Force** 或自定义轨迹
   - 让粒子围绕能量罩流动

4. **设置颜色和大小**
   - 颜色：蓝色 `(0.0, 0.6, 1.0)`
   - 大小：`2.0 - 4.0`
   - 添加 **Alpha Over Life**：淡入淡出

5. **保存**

### 6.3 将粒子添加到蓝图

**步骤：**

1. **打开 BP_EnergyShield**

2. **选中 BreakParticles 组件**
   - 在Details面板中找到 **Niagara System Asset**
   - 选择 `NS_ShieldBreak`

3. **选中 EnergyFlowParticles 组件**
   - **Niagara System Asset**: 选择 `NS_EnergyFlow`

4. **编译并保存**

5. **测试**
   - 运行游戏
   - 击碎时应该看到粒子爆发效果
   - 正常状态应该看到能量流动

---

## 7. 优化与增强

### 7.1 添加更多材质效果

**可选增强：**

1. **Fresnel边缘发光**
   - 在材质中添加 Fresnel 节点
   - 连接到 Emissive Color（相加）

2. **噪声纹理**
   - 添加 Simple Noise
   - 使用 Panner 创建流动效果
   - 连接到 Emissive Color

3. **裂纹效果**
   - 使用 Noise 纹理创建裂纹
   - 用 BreakProgress 控制显示

### 7.2 性能优化

1. **LOD设置**
   - 为网格添加LOD

2. **粒子限制**
   - 限制粒子最大数量

3. **材质优化**
   - 减少纹理采样
   - 使用简化版材质

### 7.3 添加音效

1. **创建或导入音效**
   - Content Browser → Import
   - 导入破碎音效

2. **在蓝图中添加**
   - 在 Event Graph 中
   - 在 **Start Break** 后添加 **Play Sound at Location**

---

## 8. 最终检查清单

- [ ] 项目编译成功，无错误
- [ ] 材质已创建，所有参数已连接
- [ ] 蓝图已创建并配置
- [ ] ShieldMesh 已分配网格和材质
- [ ] 能量罩可以在场景中显示
- [ ] 击碎效果正常工作（透明度、缩放、消失）
- [ ] 参数可以正确控制效果
- [ ] 粒子效果已添加（可选）
- [ ] 材质保存并应用
- [ ] 蓝图编译并保存

---

## 9. 展示优化建议

### 9.1 LinkedIn展示设置

1. **摄像机设置**
   - 添加 Cine Camera Actor
   - 设置合适的FOV和景深

2. **后期处理**
   - 添加 Post Process Volume
   - 启用 Bloom（增强发光）
   - 调整 Exposure（曝光）

3. **背景**
   - 使用暗色背景突出能量效果
   - 添加环境光照

4. **录制**
   - 使用 Sequencer 创建过场动画
   - 或使用 Take Recorder 录制
   - 导出为视频文件

### 9.2 循环展示

在蓝图Event Graph中设置：
```
Event BeginPlay → Start Loop Timer (每5秒)
Timer Event → Start Break
击碎完成 → Delay (2秒) → Reset Shield
```

---

## 完成！

现在你应该有一个完整的能量罩击碎效果了！

如果遇到任何问题，请参考上面的故障排查部分，或查看Output Log的错误信息。
