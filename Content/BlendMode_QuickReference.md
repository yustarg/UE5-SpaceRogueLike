# Blend Mode 快速参考指南

## 什么是 Blend Mode？

Blend Mode（混合模式）决定材质如何与背景混合显示。对于能量罩这样的半透明发光效果，选择正确的混合模式至关重要。

---

## 三种主要混合模式对比

### 1. Opaque（不透明）

```
外观：█████████
特点：完全实心，看不透
```

**适用场景：**
- 普通物体（墙壁、地面、角色）
- 不需要透明效果的材质

**能量罩效果：** ❌ 不适合（看起来像实心塑料球）

---

### 2. Translucent（透明/半透明）

```
外观：▓▓▓▓▓▓▓▓▓ ← 可以看到后面的物体
特点：标准透明，按透明度混合
```

**适用场景：**
- 玻璃
- 水面
- 标准透明物体
- 需要看穿的效果

**能量罩效果：** ✅ 可以用（标准透明能量罩）

**优点：**
- 可以清楚看到后面的物体
- 透明度可控
- 真实的透明感

**缺点：**
- 发光效果不如 Additive 明显
- 可能显得有点"沉闷"

---

### 3. Additive（加法混合）⭐ 推荐

```
外观：░░░░░░░░░ ← 发光叠加，边缘更亮
特点：颜色相加，自带发光效果
```

**适用场景：**
- 能量效果 ⭐
- 火焰、爆炸
- 激光、光束
- 魔法效果
- 全息投影

**能量罩效果：** ⭐⭐⭐ 最推荐（酷炫的发光能量罩）

**优点：**
- 发光效果非常明显
- 边缘自然发光
- 适合能量、魔法类效果
- 视觉冲击力强
- 适合 LinkedIn 展示

**缺点：**
- 不适合真实玻璃效果
- 可能过亮（但可以调整）

---

## 视觉效果对比

### Opaque（不透明）
```
     ███████
   ███████████
  █████████████
  █████████████  ← 完全实心，像塑料球
  █████████████
   ███████████
     ███████
```

### Translucent（透明）
```
     ▓▓▓▓▓▓▓
   ▓▓▓▓░░▓▓▓▓▓
  ▓▓▓░░░░░░▓▓▓
  ▓▓░░░░░░░░▓▓  ← 可以看穿，边缘透明
  ▓▓▓░░░░░░▓▓▓
   ▓▓▓▓░░▓▓▓▓▓
     ▓▓▓▓▓▓▓
```

### Additive（加法）⭐
```
     ▒▒▒▒▒▒▒
   ▒▒▒▒░░▒▒▒▒▒
  ▒▒▒░░·░░░▒▒▒
  ▒▒░░·····░░▒▒  ← 发光效果，边缘明亮
  ▒▒▒░░·░░░▒▒▒
   ▒▒▒▒░░▒▒▒▒▒
     ▒▒▒▒▒▒▒
   周围有光晕
```

---

## 如何设置 Blend Mode

### 在材质编辑器中：

1. **选中材质主节点**（左侧的灰色节点）
2. **打开Details面板**（右侧，或按 `Ctrl+Shift+D`）
3. **找到 Material 分类**
4. **找到 Blend Mode 下拉菜单**
5. **选择合适的模式**

### 自动切换：

如果你连接了 **Opacity** 节点到材质：
- Blend Mode 会**自动**从 Opaque 切换为 Translucent

---

## 能量罩项目推荐设置

### 方案A：标准透明能量罩

```
Blend Mode: Translucent
Shading Model: Unlit
Opacity: 0.8
效果: 清晰的透明能量罩
```

### 方案B：酷炫发光能量罩 ⭐⭐⭐ 推荐

```
Blend Mode: Additive
Shading Model: Unlit
Opacity: 可选（Additive模式不一定需要）
效果: 发光的能量罩，视觉冲击强
```

### 方案C：混合模式（进阶）

```
Blend Mode: Translucent
Shading Model: Unlit
Emissive Color: 强发光
Opacity: 0.6
效果: 既透明又发光
```

---

## 快速决策树

```
需要能量罩效果吗？
    │
    ├─→ 是，要酷炫发光效果
    │   └─→ 使用 Additive ⭐⭐⭐
    │
    ├─→ 是，要真实透明效果
    │   └─→ 使用 Translucent
    │
    └─→ 不是，普通实心物体
        └─→ 使用 Opaque
```

---

## 常见问题

### Q: 为什么推荐 Additive 而不是 Translucent？

A: 
- Additive 的发光效果更明显
- 更适合能量、魔法、科幻效果
- 视觉冲击力强，适合作品展示
- LinkedIn 展示效果更好

### Q: Additive 会不会太亮？

A: 
- 可以调整 Emissive Color 的强度
- 可以调整 ShieldStrength 参数
- 可以在后期处理中调整

### Q: 可以在运行时切换 Blend Mode 吗？

A: 
- 不能直接切换 Blend Mode
- 但可以通过材质实例切换不同材质
- 或使用材质参数调整透明度和发光强度

### Q: 我连接了 Opacity 但还是不透明？

A: 
- 检查 Blend Mode 是否为 Translucent 或 Additive
- 检查 Opacity 参数值（应该 < 1.0）
- 确保 Opacity 节点正确连接到材质输入

---

## 测试方法

### 在材质编辑器中测试不同模式：

1. 选中材质主节点
2. 在 Details 面板中切换 Blend Mode
3. 观察预览球的变化：
   - **Opaque**: 完全实心
   - **Translucent**: 半透明
   - **Additive**: 发光明显

4. 找到视觉效果最好的模式

---

## 最终建议

### 对于能量罩击碎效果项目：

**首选：Additive** ⭐⭐⭐
- 视觉效果最佳
- 适合作品展示
- 发光效果强

**备选：Translucent**
- 更真实的透明感
- 如果需要看清后面的物体

**不推荐：Opaque**
- 除非要做实心力场效果

---

## 完整材质设置总结

```
Material Settings:
├─ Shading Model: Unlit
├─ Blend Mode: Additive ⭐ (或 Translucent)
├─ Two Sided: 勾选
└─ Cast Shadow: 取消勾选

Inputs:
├─ Base Color: Constant3Vector (0.2, 0.5, 1.0)
├─ Emissive Color: Constant3Vector (0.0, 0.8, 1.0) × ShieldStrength
└─ Opacity: Opacity × (1 - BreakProgress)
```

---

**快速记忆：能量效果用 Additive，玻璃效果用 Translucent！**
