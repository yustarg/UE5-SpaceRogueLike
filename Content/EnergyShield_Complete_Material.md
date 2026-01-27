# 能量罩材质 - 完整节点连接指南

## 完整的材质节点图

这是一个完整的、可直接使用的材质设置，包含所有参数连接。

### 步骤1：创建基础参数

在材质编辑器中创建以下参数：

1. **Scalar Parameter** - 命名为 `Opacity` (默认值 0.8)
2. **Scalar Parameter** - 命名为 `BreakProgress` (默认值 0.0)
3. **Scalar Parameter** - 命名为 `ShieldStrength` (默认值 1.0)
4. **Scalar Parameter** - 命名为 `EnergyFlowSpeed` (默认值 1.0) - 可选

### 步骤2：基础颜色和发光

```
1. Base Color（基础颜色）:
   ┌─────────────────┐
   │ Constant3Vector │ → Base Color
   │ (0.2, 0.5, 1.0) │
   └─────────────────┘
   蓝色能量基础色

2. Emissive Color（发光颜色）:
   ┌─────────────────┐     ┌──────────────────┐
   │ Constant3Vector │  ×  │ ShieldStrength   │ → Emissive Color
   │ (0.0, 0.8, 1.0) │     │ Scalar Parameter │
   └─────────────────┘     └──────────────────┘
   亮蓝色发光 × 能量强度
```

**节点连接：**
- 右键 → Constants → Constant3Vector，设置为 (0.0, 0.8, 1.0)
- 右键 → Parameters → Scalar Parameter，命名为 "ShieldStrength"
- 右键 → Math → Multiply（乘法）
- 将 Constant3Vector 连接到 Multiply 的 A
- 将 ShieldStrength 连接到 Multiply 的 B
- 将 Multiply 的输出连接到 Emissive Color

### 步骤3：边缘发光效果（Fresnel）

```
3. Fresnel Effect（边缘发光）:
   ┌──────────┐     ┌──────────────────┐     ┌──────────┐
   │ Fresnel  │  ×  │ ShieldStrength   │  ×  │ Constant │ → 连接到 Emissive Color（加法）
   │          │     │ Scalar Parameter │     │ (2.0)    │
   └──────────┘     └──────────────────┘     └──────────┘
```

**节点连接：**
- 右键 → Material Functions → Fresnel
- 右键 → Parameters → Scalar Parameter，命名为 "ShieldStrength"（复用上面的）
- 右键 → Constants → Constant，设置为 2.0
- 连接：Fresnel → Multiply A，ShieldStrength → Multiply B，Constant → Multiply C
- 或者：Fresnel × ShieldStrength × 2.0
- 将结果与步骤2的 Emissive Color 相加（Add 节点）

**最终 Emissive Color 连接：**
```
(基础发光 × ShieldStrength) + (Fresnel × ShieldStrength × 2.0)
```

### 步骤4：能量流动效果（可选但推荐）

```
4. Energy Flow（能量流动）:
   ┌──────────┐     ┌──────────────┐     ┌──────────┐
   │ Time     │  ×  │ FlowSpeed    │  →  │ Panner   │
   └──────────┘     └──────────────┘     └──────────┘
                                              ↓
                                    ┌──────────────┐
                                    │ Simple Noise │ → 连接到 Emissive Color（加法）
                                    └──────────────┘
```

**节点连接：**
- 右键 → Utility → Time
- 右键 → Parameters → Scalar Parameter，命名为 "EnergyFlowSpeed" (默认值 1.0)
- 右键 → Coordinates → Panner
- 右键 → Textures → Simple Noise（或 Noise）
- 连接：Time × EnergyFlowSpeed → Panner 的 Time
- Panner 的输出 → Simple Noise 的 Coordinates
- Simple Noise 的输出与 Emissive Color 相加

### 步骤5：破碎裂纹效果

```
5. Break Cracks（破碎裂纹）:
   ┌──────────────────┐     ┌──────────────┐
   │ BreakProgress    │  →  │ OneMinus     │
   │ Scalar Parameter │     │ (1 - Break)  │
   └──────────────────┘     └──────────────┘
                                    ↓
                            ┌──────────────┐
                            │ Simple Noise │
                            └──────────────┘
                                    ↓
                            ┌──────────────┐
                            │ Step         │ → 连接到 Opacity（乘法）
                            └──────────────┘
```

**节点连接：**
- 右键 → Parameters → Scalar Parameter，命名为 "BreakProgress" (默认值 0.0)
- 右键 → Math → OneMinus（1 - X）
- 右键 → Textures → Simple Noise
- 右键 → Math → Step（或使用 If 节点）
- 连接：BreakProgress → OneMinus
- OneMinus → Simple Noise 的某个输入（用于控制噪声）
- Simple Noise → Step 的输入
- Step 的输出用于控制 Opacity

**更简单的破碎效果：**
```
BreakProgress → OneMinus → 直接连接到 Opacity（乘法）
这样破碎时透明度会逐渐降低
```

### 步骤6：最终 Opacity 连接

```
6. Final Opacity（最终透明度）:
   ┌──────────┐     ┌──────────────────┐
   │ Opacity  │  ×  │ (1 - BreakProgress)│ → Opacity
   │ Parameter│     │ OneMinus          │
   └──────────┘     └──────────────────┘
```

**节点连接：**
- 右键 → Parameters → Scalar Parameter，命名为 "Opacity" (默认值 0.8)
- 右键 → Parameters → Scalar Parameter，命名为 "BreakProgress"
- 右键 → Math → OneMinus
- 右键 → Math → Multiply
- 连接：Opacity → Multiply A
- BreakProgress → OneMinus → Multiply B
- Multiply 输出 → Opacity

### 步骤7：完整的节点连接总结

**Emissive Color 最终连接：**
```
Add 节点：
  ├─ A: (Constant3Vector(0.0,0.8,1.0) × ShieldStrength)
  └─ B: (Fresnel × ShieldStrength × 2.0)
     └─ 可选：+ (EnergyFlow Noise)
```

**Opacity 最终连接：**
```
Multiply 节点：
  ├─ A: Opacity Parameter
  └─ B: (1 - BreakProgress)
```

## 快速设置步骤（按顺序）

1. **创建所有参数**
   - Opacity (0.8)
   - BreakProgress (0.0)
   - ShieldStrength (1.0)

2. **连接 Emissive Color**
   - Constant3Vector(0.0, 0.8, 1.0) × ShieldStrength → Emissive Color
   - （可选）添加 Fresnel 效果

3. **连接 Opacity**
   - Opacity × (1 - BreakProgress) → Opacity

4. **连接 Base Color**
   - Constant3Vector(0.2, 0.5, 1.0) → Base Color

5. **保存并应用**
   - Ctrl + S 保存材质
   - 应用到网格上

## 节点查找位置

在材质编辑器中右键菜单：

- **Constants** → Constant3Vector, Constant
- **Parameters** → Scalar Parameter
- **Math** → Multiply, Add, OneMinus, Step
- **Material Functions** → Fresnel
- **Coordinates** → Panner
- **Textures** → Simple Noise, Noise
- **Utility** → Time

## 测试参数

在材质编辑器中：
1. 选中 BreakProgress 参数
2. 在 Details 面板中修改默认值（0.0 → 1.0）
3. 观察材质变化（应该逐渐变透明）
4. 改回 0.0

同样测试 ShieldStrength（1.0 → 0.0，发光应该减弱）

## 故障排除

**问题：参数没有效果？**
- 确保参数已连接到材质输入
- 检查参数名称拼写是否正确
- 确保使用了 Dynamic Material Instance（在C++代码中）

**问题：看不到发光效果？**
- 确保 Emissive Color 已连接
- 检查 ShieldStrength 值是否 > 0
- 尝试增加 Emissive Color 的强度

**问题：破碎效果不明显？**
- 确保 BreakProgress 连接到 Opacity
- 检查 OneMinus 节点是否正确连接
- 尝试调整 Opacity 的基础值
