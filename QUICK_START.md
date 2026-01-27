# ⚡ 30分钟快速启动指南

## 🎯 目标
用最少的步骤，让游戏能跑起来进行基础测试。  
详细配置请参考 `GAME_SETUP_GUIDE.md`

---

## 📋 最小化配置步骤

### Step 1: 创建必要的蓝图（10分钟）

#### 1.1 创建文件夹结构
```
Content/
├── Blueprints/
│   ├── Core/
│   ├── Player/
│   ├── Enemy/
│   ├── Weapons/
│   └── Abilities/
├── Input/
├── Widgets/
└── Maps/
```

#### 1.2 创建 7 个蓝图
| 位置 | 蓝图名称 | 基于类 | 必须配置 |
|------|---------|--------|----------|
| `Core/` | `BP_GameMode` | `SpaceRoguelikeGameMode` | PlayerController, DefaultPawn, GameState |
| `Core/` | `BP_GameState` | `SpaceRoguelikeGameState` | 无 |
| `Core/` | `BP_PlayerController` | `SpaceRoguelikePlayerController` | Input资产, Widget类 |
| `Player/` | `BP_SpaceShip` | `SpaceShip` | 添加Mesh, FireAbilityClass |
| `Enemy/` | `BP_Enemy` | `EnemyBase` | 添加Mesh |
| `Weapons/` | `BP_Projectile` | `Projectile` | 添加Mesh |
| `Abilities/` | `BP_GA_Fire` | `GA_Fire` | ProjectileClass |

---

### Step 2: 创建输入系统（5分钟）

#### 2.1 创建 Input Actions (在 Input 文件夹)
1. **IA_Move** - Value Type: `Axis 2D`
2. **IA_Fire** - Value Type: `Digital`

#### 2.2 创建 Input Mapping Context
1. **IMC_Gameplay** - 绑定:
   - W/A/S/D → IA_Move (配置 Swizzle 和 Negate)
   - Left Mouse Button → IA_Fire

---

### Step 3: 创建简单 UI（5分钟）

#### 3.1 创建 WBP_GameHUD
- 添加一个 Text，显示 "GAME RUNNING"
- 暂时不做复杂布局

#### 3.2 创建 WBP_GameOver
- 添加 Text "GAME OVER"
- 添加 Button "RESTART"
- 按钮点击事件: 调用 GameMode.RestartGame()

---

### Step 4: 创建关卡（5分钟）

#### 4.1 创建 L_MainGame
1. File → New Level → Empty Level
2. 保存到 `Maps/L_MainGame`

#### 4.2 添加必需物体
- Player Start (位置 0,0,100)
- Directional Light
- Sky Sphere（可选，用于视觉效果）

#### 4.3 设置 GameMode
- World Settings → GameMode Override = BP_GameMode

---

### Step 5: 配置引用关系（5分钟）

**按此顺序配置，避免循环引用**：

#### 5.1 BP_GA_Fire
```
Projectile Class = BP_Projectile
```

#### 5.2 BP_SpaceShip
```
Static Mesh = Cone（旋转使尖端朝前）
Fire Ability Class = BP_GA_Fire
```

#### 5.3 BP_Enemy
```
Static Mesh = Cube（设为红色）
```

#### 5.4 BP_PlayerController
```
Default Mapping Context = IMC_Gameplay
Move Action = IA_Move
Fire Action = IA_Fire
HUD Widget Class = WBP_GameHUD
Game Over Widget Class = WBP_GameOver
```

#### 5.5 BP_GameMode
```
Player Controller Class = BP_PlayerController
Default Pawn Class = BP_SpaceShip
Game State Class = BP_GameState
```

#### 5.6 项目设置
```
Edit → Project Settings:
- Maps & Modes:
  - Editor Startup Map = L_MainGame
  - Game Default Map = L_MainGame
  - Default GameMode = BP_GameMode
```

---

### Step 6: 测试运行（立即！）

1. **保存所有** - `Ctrl + Shift + S`
2. **点击 Play** - 工具栏绿色播放按钮
3. **测试移动** - WASD 移动
4. **测试射击** - 鼠标左键射击

---

## ✅ 快速验证清单

**启动游戏后，立即检查**：
- [ ] 能看到飞船（即使是简单的锥形）
- [ ] WASD 能移动
- [ ] 飞船朝向鼠标（即使不够平滑）
- [ ] 鼠标左键能发射子弹（即使看不太清）

**如果以上全部通过 → 成功！**  
**如果任何一项失败 → 参考 `GAME_SETUP_GUIDE.md` 的故障排除章节**

---

## 🚨 最常见的 3 个问题

### 问题 1: 黑屏看不到任何东西
**快速修复**: 
- 添加 Directional Light 到关卡
- 检查 Player Start 的 Z 坐标（应该 > 50）

### 问题 2: 不能移动
**快速修复**:
- 打开 BP_PlayerController
- 确认 Default Mapping Context = IMC_Gameplay
- 打开 IMC_Gameplay
- 确认 WASD 绑定到 IA_Move

### 问题 3: 不能射击
**快速修复**:
- BP_SpaceShip → Fire Ability Class = BP_GA_Fire
- BP_GA_Fire → Projectile Class = BP_Projectile
- 检查 Output Log 是否有 GAS 错误

---

## 🎨 临时美术资源

**如果没有美术资源，使用以下引擎自带形状**：

- **玩家飞船**: `Cone`（旋转 90° 使尖端朝前）
- **敌人**: `Cube`（缩放到 0.5，材质改为红色）
- **子弹**: `Sphere`（缩放到 0.2，材质改为黄色）
- **地面**: `Plane`（缩放到 50x50）
- **障碍物**: `Cube` 或 `Sphere`

**设置材质颜色的快速方法**：
1. 选中 Static Mesh 组件
2. Details → Materials → Element 0
3. 点击下拉菜单
4. 搜索并选择引擎自带材质（如 `M_Basic_Floor` 等）
5. 或创建 Material Instance，设置 Base Color

**💡 为什么用 Static Mesh 而不是 Skeletal Mesh？**
- 2D 俯视角游戏不需要骨骼动画
- Static Mesh 性能更好（渲染开销小 50%+）
- C++ 代码已自动隐藏默认的 Skeletal Mesh Component

---

## 💡 调试技巧

### 控制台命令（按 ~ 键打开）
```
stat fps          # 显示帧率
stat game         # 显示游戏统计
show collision    # 显示碰撞体
pause             # 暂停游戏
slomo 0.5         # 慢动作（0.5倍速）
slomo 1           # 恢复正常速度
```

### 查看日志
- Window → Developer Tools → Output Log
- 搜索关键词：`SpaceShip`, `Enemy`, `GameMode`, `Error`

### 调试视图
- 工具栏 → Show → Collision
- 工具栏 → Show → Navigation（如果用了导航网格）

---

## 🎯 30分钟目标

**最小目标**（必须完成）:
- ✅ 游戏能启动
- ✅ 能看到玩家飞船
- ✅ WASD 能移动
- ✅ 鼠标左键能射击

**达到这个目标后，再参考完整指南添加更多功能！**

---

## 📚 下一步

30分钟快速版跑通后，继续完善：

1. **阅读** `GAME_SETUP_GUIDE.md` 完整配置所有功能
2. **配置** 敌人生成系统
3. **完善** UI 数据绑定
4. **优化** 视觉效果和音效
5. **调整** 游戏平衡性

---

**准备好了？打开 Unreal Editor，开始 30 分钟挑战！** 🚀
