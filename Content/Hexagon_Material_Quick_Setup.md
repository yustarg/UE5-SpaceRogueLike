# 六边形能量罩 - 5分钟快速设置

## 最简单的实现方法（使用 Voronoi）

不需要任何纹理，直接用 UE5 内置节点实现六边形图案。

---

## 步骤1：打开材质

打开 `M_EnergyShield` 材质编辑器

---

## 步骤2：添加 Voronoi 节点（创建六边形）

### 2.1 创建基础六边形图案

```
1. 右键 → Math → Voronoi
   
2. 在 Voronoi 节点上：
   - 展开节点（点击箭头）
   - 使用 "Distance to Edge 1" 输出（第二个输出）
   
3. 右键 → Coordinates → Texture Coordinate

4. 右键 → Math → Multiply
   - Texture Coordinate → Multiply A
   - 创建 Constant2Vector (8, 8) → Multiply B
   - Multiply 输出 → Voronoi 的 Position 输入

5. 右键 → Math → OneMinus
   - Voronoi "Distance to Edge 1" → OneMinus
   - 这会反转颜色（让边缘变亮）

6. 右键 → Math → Power
   - OneMinus → Power Base
   - Constant (3.0) → Power Exp
   - 这会增强对比度
```

### 2.2 连接图

```
[Texture Coordinate] → [Multiply × (8, 8)]
                              ↓
                       [Voronoi 节点]
                              ↓
                  Distance to Edge 1 输出
                              ↓
                       [OneMinus (1-X)]
                              ↓
                       [Power ^ 3.0]
                              ↓
                    (这就是六边形图案)
```

---

## 步骤3：添加 Fresnel 边缘发光

```
7. 右键 → Material Functions → Fresnel
   
8. 右键 → Math → Multiply
   - Fresnel → Multiply A
   - Constant (3.0) → Multiply B
```

---

## 步骤4：合并到 Emissive Color

### 4.1 找到原来的发光部分

找到现有的连接：
```
Constant3Vector (0.0, 0.8, 1.0) × ShieldStrength → Emissive Color
```

### 4.2 添加六边形图案

```
9. 右键 → Math → Multiply
   - (原来的发光) → Multiply A
   - (六边形图案 Power 输出) → Multiply B

10. 右键 → Math → Add
    - (上一步的 Multiply) → Add A
    - (Fresnel × 3.0) → Add B
    
11. 将 Add 输出连接到 Emissive Color
    - 断开原来的连接
    - Add 输出 → Emissive Color
```

### 4.3 最终 Emissive 连接

```
基础发光：Constant3Vector(0.0, 0.8, 1.0) × ShieldStrength
六边形：Voronoi → OneMinus → Power
边缘光：Fresnel × 3.0

最终：(基础发光 × 六边形) + 边缘光 → Emissive Color
```

---

## 步骤5：增强颜色

### 让蓝色更亮更鲜艳：

```
12. 找到 Emissive Color 的 Constant3Vector
    - 修改颜色从 (0.0, 0.8, 1.0)
    - 改为 (0.0, 1.0, 1.5) 或 (0.0, 1.2, 2.0)
    - 注意：可以超过 1.0 以获得 HDR 发光
```

---

## 步骤6：调整 Blend Mode

```
13. 选中材质主节点
14. Details 面板 → Blend Mode
15. 选择 Additive（推荐）或 Translucent
```

---

## 步骤7：保存并测试

```
16. Ctrl + S 保存
17. 运行游戏测试效果
```

---

## 完整节点图（简化版）

```
┌──────────────────────────────────────────────────────┐
│                                                      │
│  六边形图案                                          │
│  TexCoord × (8,8) → Voronoi → OneMinus → Power^3.0  │
│                                    ↓                 │
│                              [六边形图案]            │
│                                    ↓                 │
│  基础发光                           ↓                │
│  Constant3Vec(0,1.2,2.0) × ShieldStrength           │
│                    ↓               ↓                │
│                    └───── × ───────┘                │
│                           ↓                         │
│                      [带图案的发光]                  │
│                           ↓                         │
│  边缘发光                  ↓                         │
│  Fresnel × 3.0 ───────→ + ─────→ Emissive Color    │
│                                                     │
│  透明度（不变）                                      │
│  Opacity × (1-BreakProgress) ─→ Opacity            │
│                                                     │
└─────────────────────────────────────────────────────┘
```

---

## 调整参数（可选）

### 调整六边形大小：

修改 Multiply 中的 Constant2Vector 值：
- `(5, 5)` → 大六边形
- `(15, 15)` → 小六边形
- `(8, 8)` → 中等（推荐）

### 调整六边形锐利度：

修改 Power 节点的指数：
- `2.0` → 柔和
- `5.0` → 锐利
- `3.0` → 推荐

### 调整边缘发光强度：

修改 Fresnel × 后面的 Constant 值：
- `1.0` → 微弱
- `5.0` → 强烈
- `3.0` → 推荐

### 调整发光强度：

修改 Emissive Color 的颜色值：
- `(0.0, 0.8, 1.0)` → 标准
- `(0.0, 1.0, 1.5)` → 亮
- `(0.0, 1.2, 2.0)` → 很亮（推荐）
- `(0.0, 1.5, 3.0)` → 超亮

---

## 节点查找快速参考

| 需要的节点 | 如何找到 |
|-----------|---------|
| **Voronoi** | 右键 → Math → Voronoi |
| **Texture Coordinate** | 右键 → Coordinates → Texture Coordinate |
| **Multiply** | 右键 → Math → Multiply |
| **OneMinus** | 右键 → Math → OneMinus |
| **Power** | 右键 → Math → Power |
| **Fresnel** | 右键 → Material Functions → Fresnel |
| **Add** | 右键 → Math → Add |
| **Constant2Vector** | 右键 → Constants → Constant2Vector |
| **Constant** | 右键 → Constants → Constant |

---

## 预期效果

完成后你应该看到：
- ✅ 蓝色发光能量罩
- ✅ 清晰的六边形蜂窝图案
- ✅ 边缘强烈发光
- ✅ 击碎时保持效果

---

## 故障排除

**问题1：看不到六边形图案**
- 检查 Voronoi 是否使用了 "Distance to Edge 1" 输出
- 检查是否添加了 OneMinus 节点
- 增加 Power 节点的指数值

**问题2：六边形太大或太小**
- 调整 Multiply 中的 Constant2Vector 值
- 增大数值 → 更多更小的六边形
- 减小数值 → 更少更大的六边形

**问题3：不够亮**
- 增加 Emissive Color 的颜色值（可以超过1.0）
- 检查 Blend Mode 是否为 Additive
- 增加 ShieldStrength 参数值

**问题4：边缘不明显**
- 增加 Fresnel × Constant 的值
- 尝试 3.0 → 5.0

---

## 完成！

按照这些步骤，你就能快速实现六边形蜂窝能量罩效果了！

**总共只需要添加约10个节点。**
