# 能量罩材质节点 - 可视化连接图

## 完整材质节点布局

```
材质编辑器布局：
┌────────────────────────────────────────────────────────────────┐
│                                                                │
│  参数区（左侧）                    材质主节点（右侧）         │
│                                                                │
│  ┌────────────────┐                                           │
│  │ Opacity        │                                           │
│  │ (0.8)          │────────┐                                  │
│  └────────────────┘        │                                  │
│                            │                                  │
│  ┌────────────────┐        │      ┌─────────────────┐        │
│  │ BreakProgress  │        │      │                 │        │
│  │ (0.0)          │───┐    │      │                 │        │
│  └────────────────┘   │    │      │                 │        │
│                       │    │      │   M_EnergyShield│        │
│  ┌────────────────┐   │    │      │                 │        │
│  │ ShieldStrength │   │    │      │  ├─ Base Color  │        │
│  │ (1.0)          │───┼────┼──┐   │  ├─ Emissive    │        │
│  └────────────────┘   │    │  │   │  └─ Opacity     │        │
│                       │    │  │   │                 │        │
│  常量区               │    │  │   └─────────────────┘        │
│                       │    │  │                              │
│  ┌────────────────┐   │    │  │                              │
│  │ Constant3Vec   │   │    │  │                              │
│  │ (0.2,0.5,1.0)  │───┼────┼──┼───► Base Color               │
│  └────────────────┘   │    │  │                              │
│                       │    │  │                              │
│  ┌────────────────┐   │    │  │   ┌──────┐                  │
│  │ Constant3Vec   │   │    │  └───┤ ×    ├─► Emissive       │
│  │ (0.0,0.8,1.0)  │───┼────┼──────┤      │                  │
│  └────────────────┘   │    │      └──────┘                  │
│                       │    │                                  │
│  数学节点             │    │      ┌──────┐   ┌──────┐        │
│                       │    └──────┤ ×    ├───┤      │        │
│  ┌────────────────┐   └───────────┤      │   │      │        │
│  │ OneMinus       │   ┌───────────┤      │   │      ├─► Opacity│
│  │ (1 - X)        │───┘           └──────┘   └──────┘        │
│  └────────────────┘                                           │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

## 详细连接步骤（按顺序）

### 步骤1：Base Color 连接

```
┌─────────────────┐
│ Constant3Vector │
│  R: 0.2         │
│  G: 0.5         │────────────────► Base Color
│  B: 1.0         │
└─────────────────┘
```

**操作：**
1. 创建 Constant3Vector
2. 设置颜色 (0.2, 0.5, 1.0)
3. 拖动输出引脚到 Base Color

---

### 步骤2：Emissive Color 连接（发光）

```
┌─────────────────┐           ┌──────────┐
│ Constant3Vector │           │          │
│  R: 0.0         │──────────►│ A        │
│  G: 0.8         │           │          │
│  B: 1.0         │           │ Multiply │────► Emissive Color
└─────────────────┘           │          │
                              │ B        │
┌─────────────────┐           │          │
│ ShieldStrength  │──────────►│          │
│ Parameter       │           └──────────┘
└─────────────────┘
```

**操作：**
1. 创建 Constant3Vector (0.0, 0.8, 1.0)
2. 创建 Multiply 节点
3. Constant3Vector 输出 → Multiply 的 A
4. ShieldStrength 输出 → Multiply 的 B
5. Multiply 输出 → Emissive Color

**为什么这样连接？**
- ShieldStrength 控制发光强度
- 当 ShieldStrength = 1.0 时，全发光
- 当 ShieldStrength = 0.0 时，不发光
- 击碎时可以让发光逐渐减弱

---

### 步骤3：Opacity 连接（透明 + 破碎）

```
┌─────────────────┐
│ BreakProgress   │
│ Parameter       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐           ┌──────────┐
│ OneMinus        │           │          │
│ (1 - X)         │──────────►│ B        │
└─────────────────┘           │          │
                              │ Multiply │────► Opacity
┌─────────────────┐           │          │
│ Opacity         │           │ A        │
│ Parameter       │──────────►│          │
└─────────────────┘           └──────────┘
```

**操作：**
1. 创建 OneMinus 节点
2. BreakProgress 输出 → OneMinus 输入
3. 创建 Multiply 节点
4. Opacity Parameter 输出 → Multiply 的 A
5. OneMinus 输出 → Multiply 的 B
6. Multiply 输出 → Opacity

**为什么这样连接？**
- BreakProgress = 0.0 时：
  - OneMinus 输出 = 1.0
  - Opacity = 0.8 × 1.0 = 0.8（正常透明度）
- BreakProgress = 0.5 时：
  - OneMinus 输出 = 0.5
  - Opacity = 0.8 × 0.5 = 0.4（更透明）
- BreakProgress = 1.0 时：
  - OneMinus 输出 = 0.0
  - Opacity = 0.8 × 0.0 = 0.0（完全透明，消失）

---

## 节点查找速查表

### 在材质编辑器中右键菜单：

| 节点类型 | 菜单路径 | 说明 |
|---------|---------|------|
| **Constant3Vector** | Constants → Constant3Vector | 3D颜色常量 (RGB) |
| **Constant** | Constants → Constant | 单一数值常量 |
| **Scalar Parameter** | Parameters → Scalar Parameter | 可在外部修改的数值参数 |
| **Multiply** | Math → Multiply | 乘法：A × B |
| **Add** | Math → Add | 加法：A + B |
| **OneMinus** | Math → OneMinus | 1 - X |
| **Fresnel** | Material Functions → Fresnel | 边缘发光效果 |
| **Time** | Utility → Time | 时间（用于动画） |
| **Panner** | Coordinates → Panner | 纹理平移（流动效果） |
| **Simple Noise** | Textures → Simple Noise | 程序化噪声 |

---

## 材质预览测试

### 在材质编辑器中测试参数：

1. **测试 BreakProgress**
   ```
   选中 BreakProgress 参数节点
   在 Details 面板中修改 Default Value:
   - 0.0 → 材质球应该正常显示
   - 0.5 → 材质球应该更透明
   - 1.0 → 材质球应该几乎消失
   ```

2. **测试 ShieldStrength**
   ```
   选中 ShieldStrength 参数节点
   在 Details 面板中修改 Default Value:
   - 1.0 → 发光最亮
   - 0.5 → 发光减半
   - 0.0 → 不发光
   ```

3. **测试 Opacity**
   ```
   选中 Opacity 参数节点
   在 Details 面板中修改 Default Value:
   - 1.0 → 不透明
   - 0.5 → 半透明
   - 0.0 → 完全透明
   ```

---

## 常见连接错误

### ❌ 错误1：参数没有连接

```
❌ 错误示例：
┌─────────────────┐
│ BreakProgress   │
│ Parameter       │
└─────────────────┘
（没有任何连线）

这样参数不会起作用！
```

### ✅ 正确做法：

```
✅ 正确示例：
┌─────────────────┐
│ BreakProgress   │ ───┐
└─────────────────┘    │
                       ▼
                  ┌──────────┐
                  │ OneMinus │ ──► ...
                  └──────────┘
```

---

### ❌ 错误2：Opacity 没有连接

```
❌ 错误：
材质的 Opacity 输入引脚没有任何连接
→ 材质会是不透明的
→ Blend Mode 不会自动切换
```

### ✅ 正确做法：

```
✅ 正确：
任何值 ──► Opacity 输入引脚
→ 连接后 Blend Mode 会自动变为 Translucent
→ 材质会显示透明效果
```

---

### ❌ 错误3：Emissive Color 没有连接

```
❌ 错误：
Shading Model = Unlit
但 Emissive Color 没有连接
→ 材质会是黑色的
```

### ✅ 正确做法：

```
✅ 正确：
任何颜色 ──► Emissive Color 输入引脚
→ 材质会发光
→ 预览球应该显示发光效果
```

---

## 快速诊断清单

在材质编辑器中检查：

- [ ] 材质主节点的 **Base Color** 有连线（蓝色）
- [ ] 材质主节点的 **Emissive Color** 有连线（亮蓝色 × ShieldStrength）
- [ ] 材质主节点的 **Opacity** 有连线（Opacity × (1-BreakProgress)）
- [ ] **Shading Model** = Unlit
- [ ] **Blend Mode** = Translucent 或 Additive
- [ ] **Two Sided** = 勾选
- [ ] 预览球显示蓝色发光效果
- [ ] 调整 BreakProgress 参数时预览球变化
- [ ] 材质已保存（Ctrl+S）

---

## 进阶连接（可选）

### 添加 Fresnel 边缘发光

```
┌─────────────┐     ┌──────────┐
│ Fresnel     │────►│ ×        │
└─────────────┘     │          │     ┌──────────┐
                    │ Multiply │────►│ +        │
┌─────────────┐     │          │     │          │
│ Constant    │────►│          │     │ Add      │──► Emissive
│ (2.0)       │     └──────────┘     │          │
└─────────────┘                      │          │
                                     │          │
（原始发光）────────────────────────►│          │
                                     └──────────┘
```

**效果：**
- 能量罩边缘更亮
- 更明显的发光效果

### 添加流动效果

```
┌─────────┐     ┌─────────┐     ┌─────────────┐
│ Time    │────►│ Panner  │────►│ SimpleNoise │──► 添加到 Emissive
└─────────┘     └─────────┘     └─────────────┘
```

**效果：**
- 能量在表面流动
- 动态效果

---

**保存此文档供参考！**
