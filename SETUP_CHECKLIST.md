# 🎯 游戏配置快速清单

## 使用说明
这是一个简化的清单，配合 `GAME_SETUP_GUIDE.md` 使用。  
详细步骤请参考配置指南。

---

## 第一步：创建蓝图类（7个）

### Core 核心系统
- [ ] `Blueprints/Core/BP_SpaceRoguelikeGameMode`
  - 基于: `SpaceRoguelikeGameMode` C++ 类
  - 配置: Player Controller Class, Default Pawn Class, Game State Class

- [ ] `Blueprints/Core/BP_SpaceRoguelikeGameState`
  - 基于: `SpaceRoguelikeGameState` C++ 类

- [ ] `Blueprints/Core/BP_SpaceRoguelikePlayerController`
  - 基于: `SpaceRoguelikePlayerController` C++ 类
  - 配置: Input Actions, Mapping Context, Widget Classes

### Player 玩家系统
- [ ] `Blueprints/Player/BP_SpaceShip`
  - 基于: `SpaceShip` C++ 类
  - 添加: Static Mesh 组件
  - 配置: Fire Ability Class, Fire Rate, Rotation Speed
  - 网格: Cone/Arrow（尖端朝 X 轴正方向）
  - 碰撞: Capsule (Height=50, Radius=30)

### Enemy 敌人系统
- [ ] `Blueprints/Enemy/BP_EnemyBase`
  - 基于: `EnemyBase` C++ 类
  - 添加: Static Mesh 组件（红色 Cube/Sphere）
  - 配置: Contact Damage = 10.0
  - 碰撞: Capsule (Height=40, Radius=25)
  - 重要: Generate Overlap Events = True

### Weapons 武器系统
- [ ] `Blueprints/Weapons/BP_Projectile`
  - 基于: `Projectile` C++ 类
  - 添加: Static Mesh 组件（小球体）
  - 配置: Speed = 2000, Damage = 10, Lifetime = 3
  - 碰撞: Sphere (Radius=15)

### Abilities GAS能力
- [ ] `Blueprints/Abilities/BP_GA_Fire`
  - 基于: `GA_Fire` C++ 类
  - 配置: Projectile Class = BP_Projectile
  - 配置: Spawn Offset = 100.0

---

## 第二步：创建输入系统（4个）

### Input Actions
- [ ] `Input/IA_Move`
  - Value Type: Axis 2D (Vector 2D)

- [ ] `Input/IA_Fire`
  - Value Type: Digital (bool)

- [ ] `Input/IA_Pause`
  - Value Type: Digital (bool)

### Input Mapping Context
- [ ] `Input/IMC_Gameplay`
  - 绑定 W/S/A/D → IA_Move
  - 绑定 Left Mouse Button → IA_Fire
  - 绑定 Escape 或 P → IA_Pause

---

## 第三步：创建 UI（2个）

### HUD 界面
- [ ] `Widgets/WBP_GameHUD`
  - Progress Bar: HealthBar (左上角)
  - Text: HealthText（显示 HP: 100/100）
  - Text: KillCountText（右上角，显示 Kills: 0）
  - Text: TimeText（顶部中央，显示 Time: 00:00）
  - Image: Crosshair（屏幕中心，可选）
  - 重要: 所有组件勾选 `Is Variable`

### Game Over 界面
- [ ] `Widgets/WBP_GameOver`
  - Image: Background（半透明黑色遮罩）
  - Text: GameOverText（大标题 "GAME OVER"）
  - Text: StatsText（统计信息）
  - Button: RestartButton（重启按钮）
  - 事件: RestartButton.OnClicked → 调用 GameMode.RestartGame()

---

## 第四步：创建关卡（1个）

- [ ] `Maps/L_MainGame`
  - World Settings: GameMode Override = BP_SpaceRoguelikeGameMode
  - 放置: Player Start (位置 0,0,100)
  - 添加: Directional Light
  - 添加: Sky Sphere 或 Sky Atmosphere
  - （可选）添加: Plane 地面
  - （可选）添加: 几个 Cube 作为障碍物

---

## 第五步：项目设置

- [ ] **Project Settings → Maps & Modes**
  - Editor Startup Map = L_MainGame
  - Game Default Map = L_MainGame
  - Default GameMode = BP_SpaceRoguelikeGameMode

- [ ] **Project Settings → Input**
  - Default Input Component Class = Enhanced Input Component
  - Default Player Input Class = Enhanced Player Input

- [ ] **Project Settings → Collision**（可选）
  - 添加 Projectile 碰撞预设

---

## 第六步：功能测试

### 基础测试（9项）
1. [ ] 玩家生成 - 飞船出现在场景中
2. [ ] WASD 移动 - 四个方向都能移动
3. [ ] 鼠标朝向 - 飞船跟随鼠标旋转
4. [ ] 射击功能 - 按住左键发射子弹
5. [ ] 敌人生成 - 自动生成敌人
6. [ ] 敌人 AI - 敌人追踪玩家
7. [ ] 战斗系统 - 子弹杀敌，敌人伤害玩家
8. [ ] UI 显示 - HUD 和数据显示
9. [ ] 游戏结束 - 死亡后显示 Game Over 并能重启

### 性能测试
- [ ] 帧率稳定在 60 FPS 以上
- [ ] 无明显卡顿
- [ ] 内存使用正常

---

## 🔗 GameMode 配置总结

**BP_SpaceRoguelikeGameMode 必须设置的类引用**:

```
Classes:
├─ Game State Class: BP_SpaceRoguelikeGameState
├─ Player Controller Class: BP_SpaceRoguelikePlayerController
└─ Default Pawn Class: BP_SpaceShip
```

---

## 🔗 PlayerController 配置总结

**BP_SpaceRoguelikePlayerController 必须设置的引用**:

```
Input:
├─ Default Mapping Context: IMC_Gameplay
├─ Move Action: IA_Move
├─ Fire Action: IA_Fire
└─ Pause Action: IA_Pause

UI:
├─ HUD Widget Class: WBP_GameHUD
└─ Game Over Widget Class: WBP_GameOver
```

---

## 🔗 SpaceShip 配置总结

**BP_SpaceShip 必须设置的引用和参数**:

```
Components:
└─ Static Mesh: 任意网格（尖端朝 X 轴）

Abilities:
└─ Fire Ability Class: BP_GA_Fire

Combat:
└─ Fire Rate: 0.2

Movement:
└─ Rotation Speed: 10.0
```

---

## 🔗 GA_Fire 配置总结

**BP_GA_Fire 必须设置的引用**:

```
Projectile:
├─ Projectile Class: BP_Projectile
└─ Spawn Offset: 100.0
```

---

## 📁 最终文件夹结构

```
Content/
├── Blueprints/
│   ├── Core/
│   │   ├── BP_SpaceRoguelikeGameMode
│   │   ├── BP_SpaceRoguelikeGameState
│   │   └── BP_SpaceRoguelikePlayerController
│   ├── Player/
│   │   └── BP_SpaceShip
│   ├── Enemy/
│   │   └── BP_EnemyBase
│   ├── Weapons/
│   │   └── BP_Projectile
│   └── Abilities/
│       └── BP_GA_Fire
├── Input/
│   ├── IA_Move
│   ├── IA_Fire
│   ├── IA_Pause
│   └── IMC_Gameplay
├── Widgets/
│   ├── WBP_GameHUD
│   └── WBP_GameOver
└── Maps/
    └── L_MainGame
```

---

## ⏱️ 预计时间

- **蓝图创建**: 45-60 分钟
- **输入配置**: 15-20 分钟
- **UI 创建**: 30-45 分钟
- **关卡设置**: 15-30 分钟
- **测试调试**: 30-60 分钟

**总计**: 2-3.5 小时

---

## 🚨 关键注意事项

### ⚠️ 必须正确配置的项（否则游戏无法运行）

1. **GameMode 的三个类引用**（PlayerController, Pawn, GameState）
2. **PlayerController 的 Input 引用**（Mapping Context 和 Actions）
3. **SpaceShip 的 Fire Ability Class**
4. **GA_Fire 的 Projectile Class**
5. **项目设置的默认地图和 GameMode**

### 💡 配置技巧

- **从上到下配置**：先 GameMode，再 PlayerController，最后 Pawn
- **逐步测试**：每完成一个阶段就测试一次
- **保存频繁**：每修改几个参数就 Save 一次
- **使用日志**：Output Log 是你的好朋友
- **先简单后复杂**：先用基础形状，功能正常后再美化

---

## 🆘 紧急故障排除

**如果完全无法运行，按此顺序检查**：

1. ✅ C++ 代码已编译成功（检查 Binaries 文件夹）
2. ✅ GameplayAbilities 插件已在 .uproject 中启用
3. ✅ GameMode 已设置 PlayerController 和 DefaultPawn
4. ✅ PlayerController 已设置 Input Mapping Context
5. ✅ 关卡中已放置 Player Start
6. ✅ 项目设置中的默认地图正确

**检查日志**：
- 打开 `Output Log`
- 搜索关键词：`Error`, `Warning`, `SpaceShip`, `GameMode`
- 根据错误信息定位问题

---

🎮 **准备好了吗？开始配置你的游戏吧！**
