# 能量罩材质 - 可视化设置指南

## 如何在UE5材质编辑器中设置混合模式

### 方法1：通过 Details 面板设置

1. **打开材质编辑器**
   - 双击材质资源打开

2. **找到材质节点**
   - 在材质编辑器左侧，有一个大的材质球图标
   - 点击这个节点

3. **查看 Details 面板**
   - 右侧应该显示 Details 面板
   - 如果没有，按 `Ctrl + Shift + D` 或点击 View → Details

4. **找到 Material 设置**
   - 在 Details 面板中，找到 **Material** 分类
   - 展开这个分类

5. **设置 Blend Mode**
   - 找到 **Blend Mode** 属性
   - 点击下拉菜单，选择：
     - **Opaque** - 不透明（默认）
     - **Masked** - 遮罩（用于镂空）
     - **Translucent** - 透明（我们要用的）
     - **Additive** - 加法混合（也适合发光效果）
     - **Modulate** - 调制混合

### 方法2：通过连接 Opacity 节点自动切换

1. **创建 Opacity 节点**
   - 在材质编辑器中右键 → Parameters → Scalar Parameter
   - 命名为 "Opacity"
   - 连接到材质节点的 **Opacity** 输入

2. **自动切换**
   - 连接 Opacity 后，材质会自动从 Opaque 切换到 Translucent
   - 这是最简单的方法！

### 方法3：使用 Additive 混合模式（推荐用于能量效果）

如果找不到 Translucent，**Additive** 模式也很适合能量罩效果：

1. 设置 Blend Mode 为 **Additive**
2. 优点：
   - 发光效果更明显
   - 适合能量、光效
   - 不需要设置 Opacity（会自动发光）

## 完整的材质节点连接图

```
材质节点 (Material Expression)
│
├─ Base Color ← Constant3Vector (0.2, 0.5, 1.0) [蓝色]
│
├─ Emissive Color ← Constant3Vector (0.0, 0.8, 1.0) [亮蓝色发光]
│                   或
│                   ← Scalar Parameter "ShieldStrength" × Constant3Vector
│
└─ Opacity ← Scalar Parameter "Opacity" (0.8)
            或
            ← (1.0 - Scalar Parameter "BreakProgress") × Scalar Parameter "Opacity"
```

## 快速检查清单

- [ ] 材质节点已选中
- [ ] Details 面板已打开
- [ ] Blend Mode 已设置为 Translucent 或 Additive
- [ ] Shading Model 设置为 Unlit
- [ ] Opacity 节点已连接
- [ ] Emissive Color 已连接（用于发光）
- [ ] Two Sided 已勾选（如果需要双面）
- [ ] 材质已保存（Ctrl + S）

## 常见问题解决

**问题：Details 面板中没有 Material 分类**
- 确保选中了材质节点（左侧的材质球）
- 尝试关闭并重新打开材质编辑器

**问题：Blend Mode 下拉菜单是灰色的**
- 检查材质是否使用了某些特殊功能
- 尝试先断开所有连接，再设置 Blend Mode

**问题：材质在视口中不显示透明效果**
- 确保 Opacity 值 < 1.0
- 检查视口设置（可能需要启用透明度显示）
- 确保材质已应用到网格上

**问题：想要更亮的发光效果**
- 使用 **Additive** 混合模式
- 增加 Emissive Color 的强度
- 使用 Scalar Parameter 控制发光强度
