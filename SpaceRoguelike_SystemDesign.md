# 太空 Roguelike 游戏 - 系统设计文档

## 文档说明
本文档基于游戏设计文档，详细规划如何使用 Unreal Engine 5 的原生系统和架构来实现游戏功能。
**设计原则**: 优先使用 UE5 内置系统，避免重复造轮子，遵循 Epic 推荐的最佳实践。

---

## 目录
1. [核心架构设计](#1-核心架构设计)
2. [Gameplay Framework](#2-gameplay-framework)
3. [输入系统](#3-输入系统)
4. [战斗与武器系统](#4-战斗与武器系统)
5. [敌人与AI系统](#5-敌人与ai系统)
6. [地图生成系统](#6-地图生成系统)
7. [难度递增系统](#7-难度递增系统)
8. [成长与装备系统](#8-成长与装备系统)
9. [UI系统](#9-ui系统)
10. [特效与音效系统](#10-特效与音效系统)
11. [存档与元进度系统](#11-存档与元进度系统)
12. [性能优化系统](#12-性能优化系统)

---

## 1. 核心架构设计

### 1.1 Gameplay Framework 层级

```
GameInstance (持久化，跨关卡)
  └─ GameInstanceSubsystem(s) (元进度、存档、统计等)
      
World
  └─ GameMode (单局游戏规则)
      ├─ GameState (单局游戏状态，复制到客户端)
      │   └─ PlayerState(s) (玩家状态)
      │
      └─ WorldSubsystem(s) (地图生成、敌人生成、难度管理等)
      
PlayerController (玩家输入控制)
  └─ Pawn/Character (飞船实体)
      └─ ActorComponent(s) (武器、生命值、技能等)
```

### 1.2 使用的核心 UE5 系统

| 系统 | 用途 |
|------|------|
| **Enhanced Input System** | 输入处理（键鼠映射、输入上下文） |
| **Gameplay Ability System (GAS)** 或 **原生组件** | 技能、buff、属性管理 |
| **Niagara VFX** | 粒子特效（爆炸、尾焰、子弹轨迹） |
| **World Partition** 或 **Level Streaming** | 大地图分块加载（如果使用大地图方案） |
| **Data Assets & Data Tables** | 数据驱动设计（敌人、武器、装备配置） |
| **Chaos Destruction** (可选) | 可破坏障碍物 |
| **UMG (Unreal Motion Graphics)** | UI界面 |
| **MetaSounds** | 音效系统 |
| **Behavior Tree & EQS** | 敌人AI |
| **Save Game System** | 存档和元进度 |
| **Object Pool Component** (UE5.1+) | 对象池管理 |

### 1.3 关键 Subsystems 设计

#### GameInstance Subsystems（持久化，跨关卡）
- `UMetaProgressionSubsystem` - 元进度管理（结晶、解锁、永久升级）
- `USaveGameSubsystem` - 存档管理
- `UStatisticsSubsystem` - 统计数据（最长生存、击杀数等）
- `UAudioManagerSubsystem` - 音频管理（BGM、音效池）

#### World Subsystems（单局游戏，World生命周期）
- `UDifficultyManagerSubsystem` - 难度递增管理
- `UEnemySpawnSubsystem` - 敌人生成管理
- `UMapGeneratorSubsystem` - 地图生成和管理
- `ULootSubsystem` - 掉落物管理
- `UObjectPoolSubsystem` - 对象池管理（子弹、敌人、特效）

---

## 2. Gameplay Framework

### 2.1 GameMode - `ASpaceRoguelikeGameMode`

**职责**:
- 游戏规则定义
- 生成 Pawn/HUD
- 游戏开始/结束逻辑
- 时限 BOSS 触发

**关键函数**:
```cpp
// 不写具体代码，只列出关键函数职责
- InitGame(): 初始化游戏，注册 Subsystems
- StartPlay(): 游戏开始，启动计时器
- RestartPlayer(): 玩家复活逻辑（如果支持）
- OnPlayerDeath(): 玩家死亡，触发结算
- TriggerBossSpawn(): 时限 BOSS 生成（通过 Timer 调用）
```

**使用的 UE5 特性**:
- `FTimerManager` - 用于时限 BOSS 触发
- `GameState` - 存储当前游戏时间、BOSS 状态等

### 2.2 GameState - `ASpaceRoguelikeGameState`

**职责**: 存储可复制的游戏状态（即使是单机也推荐用 GameState）

**存储的状态**:
```cpp
// 使用 UPROPERTY(Replicated) 标记可复制变量
- GameTime (float): 游戏运行时间
- CurrentDifficultyMultiplier (float): 当前难度倍数
- TotalEnemiesKilled (int32): 总击杀数
- CurrentBossPhase (enum): 当前是否有 BOSS，BOSS 阶段
- NextBossTime (float): 下一个 BOSS 出现时间
- PlayerLevel (int32): 玩家等级
```

**UE5 特性**:
- 使用 `GetGameTimeSinceCreation()` 获取游戏时间
- 通过 `OnRep_` 函数处理变量复制（即使单机也是好习惯）

### 2.3 PlayerState - `ASpaceRoguelikePlayerState`

**职责**: 存储玩家个人状态

**存储的状态**:
```cpp
- CurrentExperience (float): 当前经验值
- ExperienceToNextLevel (float): 升级所需经验
- Level (int32): 当前等级
- KillCount (int32): 击杀数
- DamageDealt (float): 造成的总伤害
- CollectedItems (TArray<FItemData>): 收集的装备
```

### 2.4 PlayerController - `ASpaceRoguelikePlayerController`

**职责**: 
- 处理输入（通过 Enhanced Input）
- UI 交互
- 相机控制

**不做的事情**:
- ❌ 不直接处理游戏逻辑
- ❌ 不存储游戏状态（放在 PlayerState）
- ❌ 不处理角色移动（放在 Pawn）

**使用的 UE5 系统**:
- `Enhanced Input Component`
- `UInputMappingContext`

---

## 3. 输入系统

### 3.1 Enhanced Input System

**推荐使用 Enhanced Input**（UE5 标准输入系统）

**架构**:
```
Input Mapping Context (IMC_Spaceship)
  ├─ Input Action (IA_Move) - WASD 移动
  ├─ Input Action (IA_Look) - 鼠标瞄准
  ├─ Input Action (IA_Fire) - 射击
  ├─ Input Action (IA_Skill1) - 技能1
  ├─ Input Action (IA_Skill2) - 技能2
  ├─ Input Action (IA_Dash) - 闪避
  └─ Input Action (IA_Pause) - 暂停
```

**Input Modifiers**:
- `Smooth Delta` - 平滑输入
- `Dead Zone` - 摇杆死区（如果支持手柄）

**Input Triggers**:
- `Hold` - 长按触发（持续射击）
- `Tap` - 点击触发（单次射击）
- `Hold And Release` - 蓄力释放

**在 PlayerController 中绑定**:
```cpp
// 伪代码
SetupInputComponent():
  - EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, ...)
  - EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, ...)
  // 等等
```

### 3.2 输入上下文切换

使用 `Enhanced Input Local Player Subsystem` 动态切换输入上下文：

**上下文**:
- `IMC_Gameplay` - 游戏中（移动、射击）
- `IMC_Menu` - 菜单中（UI 导航）
- `IMC_Upgrade` - 升级界面（选择升级）

**切换示例**:
```cpp
// 伪代码
void ShowUpgradeUI():
  - RemoveInputMappingContext(IMC_Gameplay)
  - AddInputMappingContext(IMC_Upgrade)
  - PauseGame()
```

---

## 4. 战斗与武器系统

### 4.1 武器系统架构

**方案 A: 使用组件系统（推荐，轻量级）**

```
ASpaceShip (Character)
  └─ UWeaponComponent (ActorComponent)
      ├─ WeaponDataAsset (配置数据)
      ├─ 射击逻辑
      ├─ 弹药管理
      └─ 使用 Object Pool 生成子弹
```

**方案 B: 使用 Gameplay Ability System（复杂项目推荐）**

```
ASpaceShip (Character)
  └─ UAbilitySystemComponent
      ├─ UGameplayAbility_PrimaryFire
      ├─ UGameplayAbility_SecondaryFire
      ├─ UGameplayAbility_Skill1
      └─ UAttributeSet (生命值、护盾、伤害等)
```

**推荐**: 如果团队熟悉 GAS，使用方案 B；否则方案 A 更简单。

### 4.2 武器组件设计 - `UWeaponComponent`

**数据驱动**: 使用 `UDataAsset` 定义武器配置

**武器数据 - `UWeaponDataAsset`**:
```cpp
// 伪代码
UWeaponDataAsset:
  - WeaponName (FText)
  - ProjectileClass (TSubclassOf<AProjectile>)
  - Damage (float)
  - FireRate (float)
  - BulletSpeed (float)
  - ProjectilesPerShot (int32) - 一次发射多少子弹
  - SpreadAngle (float) - 散射角度
  - MuzzleFlashEffect (UNiagaraSystem*)
  - FireSound (USoundBase*)
```

**WeaponComponent 职责**:
```cpp
// 伪代码
- Fire(): 发射子弹
  - 从 ObjectPoolSubsystem 获取子弹
  - 应用武器配置
  - 播放特效和音效
  
- StopFire(): 停止射击

- ApplyModifiers(TArray<FWeaponModifier>): 应用武器模组
  - 例: 增加射速、改变子弹类型等
```

### 4.3 子弹系统 - `AProjectile`

**使用 UE5 的 `ProjectileMovementComponent`**

```
AProjectile (Actor)
  ├─ UStaticMeshComponent - 子弹模型
  ├─ UProjectileMovementComponent - 移动逻辑（原生）
  ├─ USphereComponent - 碰撞检测
  └─ UNiagaraComponent (可选) - 拖尾特效
```

**ProjectileMovementComponent 配置**:
- `InitialSpeed` - 初始速度
- `MaxSpeed` - 最大速度
- `bRotationFollowsVelocity` - 旋转跟随速度
- `ProjectileGravityScale` - 重力缩放（太空中设为 0）
- `Bounciness` - 弹性（如果需要子弹反弹）

**碰撞处理**:
```cpp
// 伪代码
OnHit(HitResult):
  - 应用伤害（使用 UGameplayStatics::ApplyDamage）
  - 播放命中特效
  - 返回对象池（不 Destroy）
```

### 4.4 伤害系统

**使用 UE5 原生伤害系统**:
- `UGameplayStatics::ApplyDamage()` - 应用伤害
- `UGameplayStatics::ApplyRadialDamage()` - 范围伤害
- `TakeDamage()` 在 Actor 中重写 - 接收伤害

**伤害类型 - `UDamageType`**:
```cpp
// 创建不同的 DamageType 子类
- UDamageType_Laser
- UDamageType_Explosion
- UDamageType_Energy
// 用于区分伤害类型，实现护甲抗性等
```

### 4.5 技能系统

**方案 A: 简单组件系统**

```
ASpaceShip
  └─ UAbilityManagerComponent
      ├─ Ability Slots (TArray<UAbilityBase*>)
      └─ 冷却管理
```

**方案 B: Gameplay Ability System**

使用 UE5 官方的 GAS（推荐用于复杂技能系统）:
- `UAbilitySystemComponent` - 技能系统核心
- `UGameplayAbility` - 技能基类
- `UGameplayEffect` - Buff/Debuff
- `UAttributeSet` - 属性集合

**GAS 优势**:
- ✅ 原生支持网络复制
- ✅ 强大的 Buff/Debuff 系统
- ✅ 属性计算和修改器
- ✅ 技能标签系统（Gameplay Tags）
- ✅ 技能预测和延迟补偿

**技能示例 - 护盾技能**:
```cpp
// 伪代码（GAS 版本）
UGameplayAbility_Shield:
  - ActivateAbility():
    - 应用 GameplayEffect_Shield (提供护盾值)
    - 播放特效
    - 启动持续时间 Timer
  
  - EndAbility():
    - 移除 GameplayEffect
```

---

## 5. 敌人与AI系统

### 5.1 敌人架构

```
AEnemyBase (Character)
  ├─ UHealthComponent (或 AttributeSet if using GAS)
  ├─ UAIPerceptionComponent - AI 感知
  ├─ UPawnSensingComponent - 可选，简单感知
  └─ 行为树 (Behavior Tree)
      └─ 黑板 (Blackboard)
```

### 5.2 AI 系统 - 使用 Behavior Tree

**UE5 原生 AI 系统**:
- `Behavior Tree` - AI 行为逻辑
- `Blackboard` - AI 数据存储
- `AIController` - AI 控制器
- `EQS (Environment Query System)` - 环境查询（寻找最佳位置）

**敌人 AI 控制器 - `AEnemyAIController`**:
```cpp
// 伪代码
- OnPossess(Pawn):
  - RunBehaviorTree(EnemyBehaviorTree)
  
- 使用 AI Perception Component 感知玩家
  - 视觉感知
  - 听觉感知（可选）
```

**Behavior Tree 结构示例 - 追踪型敌人**:
```
Selector (选择器)
  ├─ Sequence: 攻击玩家
  │   ├─ [Decorator] 玩家在射程内？
  │   ├─ Task: 面向玩家
  │   └─ Task: 射击
  │
  └─ Sequence: 追踪玩家
      ├─ [Decorator] 检测到玩家？
      ├─ Task: 使用 EQS 找到最佳移动位置
      └─ Task: 移动到位置 (MoveToLocation)
```

**使用 EQS (Environment Query System)**:
- 寻找距离玩家最佳射击位置
- 避开障碍物
- 保持与其他敌人的距离（避免重叠）

### 5.3 敌人生成系统 - `UEnemySpawnSubsystem`

**World Subsystem 管理敌人生成**

**职责**:
```cpp
// 伪代码
- Initialize(): 
  - 注册敌人生成点
  - 从 DataTable 加载敌人配置
  
- Tick(DeltaTime):
  - 根据难度曲线调整生成频率
  - 调用 SpawnEnemy()
  
- SpawnEnemy(EnemyClass, Location):
  - 从 ObjectPool 获取敌人（如果可能）
  - 或 SpawnActor
  - 应用属性缩放（根据游戏时间）
  - 添加到活跃敌人列表
  
- DespawnEnemy(Enemy):
  - 返回对象池
  - 从活跃列表移除
  
- GetSpawnLocation():
  - 计算屏幕外生成位置
  - 多方向随机
```

**敌人配置 - Data Table**:
```cpp
// FEnemySpawnConfig (DataTable Row)
- EnemyClass (TSubclassOf<AEnemyBase>)
- BaseHealth (float)
- BaseDamage (float)
- MoveSpeed (float)
- UnlockTime (float) - 何时解锁该敌人类型
- SpawnWeight (float) - 生成权重
- MinGameTime (float) - 最小游戏时间要求
```

### 5.4 敌人属性缩放

**在 DifficultyManagerSubsystem 中计算缩放**

```cpp
// 伪代码
CalculateEnemyScaling(BaseValue, GameTime):
  - ScaledValue = BaseValue * (1 + GameTime * ScalingRate)
  - return ScaledValue
```

**应用缩放**:
- 生成时应用到敌人的 AttributeSet 或 HealthComponent
- 实时查询 DifficultyManager

---

## 6. 地图生成系统

### 6.1 地图方案选择

**方案 A: 固定大地图 + World Partition（推荐）**

使用 UE5 的 `World Partition` 系统：
- 自动分块加载/卸载
- 无需手动管理 Level Streaming
- 适合 10000x10000 单位的大地图

**方案 B: 程序生成 + Level Streaming**

使用 `Level Streaming` 动态加载地图块：
- 手动控制 Chunk 加载
- 更灵活但需要更多代码

**方案 C: 循环地图（最简单）**

不需要分块加载，到达边界时传送到对侧

### 6.2 使用 World Partition（方案 A）

**World Partition 配置**:
```cpp
// 在 World Settings 中启用 World Partition
- Enable World Partition
- Grid Size: 设置网格大小（例如 12800 单位）
- Loading Range: 设置加载范围
```

**Data Layers**:
- 使用 Data Layers 管理不同生物群系
- 可以动态切换 Layer

**地图生成流程**:
```cpp
// UMapGeneratorSubsystem (WorldSubsystem)
- GenerateMap():
  - 生成障碍物布局（使用 PCG - Procedural Content Generation）
  - 放置特殊地点
  - 配置生物群系

- 使用 PCG (Procedural Content Generation) Plugin:
  - UE5 官方的程序化内容生成工具
  - 可以程序化放置小行星、碎片等
```

### 6.3 使用 PCG (Procedural Content Generation)

**UE5.0+ 内置 PCG Plugin**

**PCG Graph 示例 - 生成小行星带**:
```
PCG Graph: "AsteroidField"
  ├─ Input: Bounding Box (定义生成区域)
  ├─ Point Sampler: 随机采样生成点
  ├─ Density Filter: 控制密度
  ├─ Spawn Actors: 生成小行星 Actors
  └─ Random Rotation/Scale: 随机旋转和缩放
```

**优势**:
- ✅ 可视化编辑
- ✅ 高性能
- ✅ 支持运行时生成
- ✅ 可以保存生成的种子

### 6.4 障碍物系统

**小行星 Actor**:
```
AAsteroid (Actor)
  ├─ UStaticMeshComponent - 模型
  ├─ UDestructibleComponent (可选) - 可破坏
  └─ USphereComponent - 碰撞
```

**可破坏障碍物**:
- 使用 `Chaos Destruction` 系统（UE5 原生）
- 或使用 `UDestructibleComponent`（旧系统）
- 掉落资源时使用 LootSubsystem

---

## 7. 难度递增系统

### 7.1 难度管理器 - `UDifficultyManagerSubsystem`

**World Subsystem，管理单局难度**

**职责**:
```cpp
// 伪代码
- Initialize():
  - 从 DataTable 加载难度曲线配置
  - 注册 BOSS 时间表
  
- Tick(DeltaTime):
  - 更新当前难度倍数
  - 检查是否触发 BOSS
  - 通知 EnemySpawnSubsystem 调整生成频率
  
- GetDifficultyMultiplier(GameTime):
  - 根据游戏时间计算难度倍数
  - 使用 Curve Float 资产定义曲线
  
- GetEnemyScaling(BaseValue, GameTime):
  - 计算敌人属性缩放
  
- TriggerBoss(BossClass):
  - 通知 GameMode 生成 BOSS
  - 播放警告音效和 UI
```

**使用 Curve Assets**:
- `UCurveFloat` - 定义难度曲线
- 在编辑器中可视化编辑曲线
- 示例: `Curve_DifficultyMultiplier`
  - X 轴: 游戏时间
  - Y 轴: 难度倍数

**难度配置 - Data Table**:
```cpp
// FDifficultyCurveConfig (DataTable Row)
- GameTime (float)
- SpawnRateMultiplier (float)
- EnemyHealthMultiplier (float)
- EnemyDamageMultiplier (float)
- EnemySpeedMultiplier (float)
```

### 7.2 BOSS 系统

**BOSS 触发流程**:
```cpp
// 在 GameMode 中使用 Timer
- StartPlay():
  - SetTimer(TimerHandle, &TriggerFirstBoss, 300.0f, false) // 5分钟
  
- TriggerBoss():
  - 通知 DifficultyManagerSubsystem
  - DifficultyManager 选择 BOSS 类型
  - 生成 BOSS（从对象池或 SpawnActor）
  - 播放警告 UI（通过 PlayerController）
  - 播放 BOSS 音乐（通过 AudioManagerSubsystem）
  - 设置下一个 BOSS Timer
```

**BOSS 配置 - Data Table**:
```cpp
// FBossConfig (DataTable Row)
- BossClass (TSubclassOf<AEnemyBase>)
- BossName (FText)
- TriggerTime (float)
- WarningDuration (float) - 警告持续时间
- GuaranteedLoot (TArray<FItemData>) - 必掉装备
```

---

## 8. 成长与装备系统

### 8.1 经验与升级系统

**使用 PlayerState 存储经验**

```cpp
// ASpaceRoguelikePlayerState
- CurrentExperience (float)
- Level (int32)

- AddExperience(Amount):
  - CurrentExperience += Amount
  - CheckLevelUp()
  
- CheckLevelUp():
  - if CurrentExperience >= ExperienceRequired:
    - LevelUp()
    
- LevelUp():
  - Level++
  - 生成升级选项（通过 ProgressionManager）
  - 暂停游戏
  - 显示升级 UI
```

**升级选项生成 - `UProgressionManagerSubsystem`**

**GameInstance Subsystem（跨关卡持久化）**

```cpp
// 伪代码
- GenerateUpgradeOptions(PlayerLevel):
  - 从 UpgradePool 中随机选择 3 个
  - 考虑权重和解锁条件
  - 返回 TArray<FUpgradeOption>
  
- ApplyUpgrade(UpgradeOption, Player):
  - 根据 UpgradeType 应用升级
  - 修改 AttributeSet 或 Stats
  - 添加 Buff (如果使用 GAS)
```

**升级配置 - Data Asset**:
```cpp
// UUpgradeDataAsset
- UpgradeName (FText)
- Description (FText)
- Icon (UTexture2D*)
- UpgradeType (enum) - 增加属性、解锁技能、武器模组等
- Value (float) - 数值
- UnlockLevel (int32) - 解锁等级
- Weight (float) - 选择权重
```

### 8.2 装备系统

**装备数据 - Data Asset**

```cpp
// UItemDataAsset (Primary Data Asset)
- ItemID (FName)
- ItemName (FText)
- Description (FText)
- Icon (UTexture2D*)
- Rarity (enum) - 普通、稀有、史诗、传说
- ItemType (enum) - 武器模组、被动道具、主动道具
- Stats (TMap<FName, float>) - 属性加成
- SpecialEffects (TArray<FEffectData>) - 特殊效果
```

**使用 Primary Data Asset**:
- 可以在 Asset Manager 中管理
- 支持异步加载
- 更好的内存管理

**装备管理组件 - `UInventoryComponent`**

```cpp
// 伪代码
- EquippedItems (TArray<UItemDataAsset*>)

- AddItem(ItemData):
  - EquippedItems.Add(ItemData)
  - ApplyItemEffects(ItemData)
  
- RemoveItem(ItemData):
  - RemoveItemEffects(ItemData)
  - EquippedItems.Remove(ItemData)
  
- ApplyItemEffects(ItemData):
  - 遍历 ItemData.Stats
  - 应用属性加成（通过 GAS 的 GameplayEffect 或直接修改）
```

### 8.3 掉落系统 - `ULootSubsystem`

**World Subsystem 管理掉落**

```cpp
// 伪代码
- SpawnLoot(Location, LootTableID):
  - 从 LootTable 滚动掉落物
  - 生成 PickupActor
  
- RollLoot(LootTable, DifficultyMultiplier):
  - 根据权重和品质概率滚动
  - 考虑游戏时间（时间越长，高品质概率越高）
  - 返回 ItemDataAsset
```

**掉落配置 - Data Table**:
```cpp
// FLootTableEntry (DataTable Row)
- ItemData (UItemDataAsset*)
- Weight (float) - 掉落权重
- MinGameTime (float) - 最小游戏时间要求
- MaxGameTime (float) - 最大游戏时间（超过后不掉落）
```

**拾取物 Actor**:
```
APickupActor (Actor)
  ├─ UStaticMeshComponent - 模型
  ├─ USphereComponent - 拾取碰撞
  ├─ URotatingMovementComponent - 旋转动画（原生组件）
  └─ ItemData (UItemDataAsset*) - 数据
```

---

## 9. UI系统

### 9.1 UMG 架构

**使用 UE5 的 UMG (Unreal Motion Graphics)**

**UI 层级**:
```
UUserWidget (基类)
  ├─ UHUDWidget - 游戏中 HUD
  ├─ UUpgradeWidget - 升级选择界面
  ├─ UPauseMenuWidget - 暂停菜单
  ├─ UGameOverWidget - 结算界面
  └─ UMainMenuWidget - 主菜单
```

### 9.2 HUD Widget - `UHUDWidget`

**显示元素**:
```cpp
// 绑定到 PlayerState 和 GameState
- HealthBar (UProgressBar)
- ShieldBar (UProgressBar)
- ExpBar (UProgressBar)
- TimerText (UTextBlock) - 绑定到 GameState.GameTime
- KillCountText (UTextBlock)
- BossTimerText (UTextBlock) - 下一个 BOSS 倒计时
- WeaponIcon (UImage)
- SkillCooldownOverlay (UOverlay) - 技能冷却显示
```

**数据绑定**:
- 使用 UMG 的 `Binding` 功能
- 或在 C++ 中通过 Tick 更新

**推荐**: 使用 `Event-Driven` 方式更新 UI:
```cpp
// 伪代码
PlayerState.OnHealthChanged.AddDynamic(HUDWidget, &UpdateHealthBar)
PlayerState.OnExpChanged.AddDynamic(HUDWidget, &UpdateExpBar)
```

### 9.3 升级选择界面 - `UUpgradeWidget`

**交互流程**:
```cpp
// 伪代码
- ShowUpgradeOptions(TArray<FUpgradeOption> Options):
  - 显示 3 个选项按钮
  - 绑定点击事件
  - 暂停游戏（SetGamePaused）
  
- OnOptionSelected(UpgradeOption):
  - 应用升级（通过 ProgressionManager）
  - 隐藏界面
  - 恢复游戏
```

**使用 Common UI Plugin (可选)**:
- UE5 推荐的 UI 框架
- 更好的输入处理
- 支持手柄导航

### 9.4 性能优化

**UI 优化技巧**:
- 使用 `Invalidation Box` 减少重绘
- 避免每帧 Tick 更新 Text
- 使用 `Widget Pool` 复用 Widget（如击杀提示）
- 复杂 Widget 使用 `Lazy Widget` 延迟加载

---

## 10. 特效与音效系统

### 10.1 Niagara VFX 系统

**使用 UE5 的 Niagara（推荐，不要用旧的 Cascade）**

**常用特效**:
- 飞船引擎尾焰（持续特效）
- 武器射击特效（瞬时特效）
- 爆炸特效（瞬时特效）
- 命中反馈（瞬时特效）
- 护盾冲击波（范围特效）

**特效生成方式**:
```cpp
// 使用 UNiagaraFunctionLibrary
UNiagaraFunctionLibrary::SpawnSystemAtLocation(
  World, 
  NiagaraSystem, 
  Location, 
  Rotation
);

// 或使用 UNiagaraComponent 持续特效
ShipMesh->AddComponent(NiagaraComponent);
```

**对象池优化**:
- 特效也使用 Object Pool
- 特效完成后返回池中而不是销毁

### 10.2 音效系统

**使用 MetaSounds（UE5 新音频系统）**

**音效管理 - `UAudioManagerSubsystem`**

**GameInstance Subsystem 管理音频**

```cpp
// 伪代码
- PlaySound(SoundID, Location, VolumeMultiplier):
  - 从 SoundPool 获取 AudioComponent
  - 播放声音
  
- PlayMusic(MusicTrack, FadeTime):
  - 淡出当前音乐
  - 淡入新音乐
  
- StopMusic(FadeTime):
  - 淡出音乐
```

**音效分类**:
- `SoundClass_Master` - 主音量
- `SoundClass_SFX` - 音效
- `SoundClass_Music` - 音乐
- `SoundClass_UI` - UI 音效

**使用 Sound Cue 和 MetaSound**:
- `Sound Cue` - 简单音效组合
- `MetaSound` - 复杂音效处理（动态变化）

**音效衰减**:
- 使用 `SoundAttenuation` 资产
- 配置 3D 空间音效

---

## 11. 存档与元进度系统

### 11.1 Save Game 系统

**使用 UE5 的 `USaveGame` 类**

**存档类 - `USpaceRoguelikeSaveGame`**:
```cpp
// 伪代码
USaveGame:
  - SaveSlotName (FString)
  - UserIndex (int32)
  
  // 元进度数据
  - TotalCrystals (int32)
  - UnlockedShips (TArray<FName>)
  - PermanentUpgrades (TMap<FName, int32>) - 升级ID -> 等级
  - BestSurvivalTime (float)
  - TotalKills (int32)
  - Achievements (TArray<FName>)
```

**存档管理 - `USaveGameSubsystem`**

**GameInstance Subsystem**

```cpp
// 伪代码
- SaveGame():
  - 创建 SaveGame 对象
  - 填充数据（从 MetaProgressionSubsystem）
  - UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex)
  
- LoadGame():
  - UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)
  - 应用数据到 MetaProgressionSubsystem
  
- HasSaveGame():
  - UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)
```

**自动保存**:
- 游戏结束时自动保存
- 可选: 定期自动保存（每隔 N 分钟）

### 11.2 元进度管理 - `UMetaProgressionSubsystem`

**GameInstance Subsystem（持久化跨关卡）**

```cpp
// 伪代码
- Initialize():
  - 从 SaveGameSubsystem 加载数据
  
- AddCrystals(Amount):
  - 增加结晶数量
  - 触发存档
  
- UnlockShip(ShipID):
  - 添加到 UnlockedShips
  - 广播解锁事件（用于 UI 更新）
  
- UpgradePermanentStat(StatID):
  - 消耗结晶
  - 提升永久属性
  - 存档
  
- GetPermanentStats():
  - 返回当前永久属性加成
  - 用于应用到新局游戏
```

### 11.3 统计数据 - `UStatisticsSubsystem`

**GameInstance Subsystem 记录统计**

```cpp
// 伪代码
- RecordGameEnd(SurvivalTime, KillCount, BossesDefeated):
  - 更新最佳记录
  - 检查成就
  - 计算奖励
  
- GetBestSurvivalTime()
- GetTotalKills()
- GetAchievements()
```

---

## 12. 性能优化系统

### 12.1 对象池系统 - `UObjectPoolSubsystem`

**World Subsystem 管理对象池**

**UE5.1+ 可使用官方的 Object Pool Component**

```cpp
// 伪代码
- CreatePool(ObjectClass, PoolSize):
  - 预先创建对象
  - 设置为 Inactive
  
- AcquireObject(ObjectClass):
  - 从池中获取可用对象
  - 如果池为空，动态创建新对象
  - 激活对象
  - 返回对象
  
- ReleaseObject(Object):
  - 停用对象
  - 返回池中
  
- Pools (TMap<TSubclassOf<AActor>, TArray<AActor*>>)
```

**使用对象池的对象**:
- 子弹 (Projectiles)
- 敌人 (Enemies) - 可选
- 粒子特效 (VFX)
- 音效组件 (Audio Components)
- 掉落物 (Pickups)

### 12.2 LOD 系统

**使用 UE5 的 LOD 系统**

**敌人 LOD**:
- 近距离: 完整模型 + 完整 AI
- 中距离: 简化模型 + 简化 AI（降低更新频率）
- 远距离: 超简化模型 + 极简 AI

**动态 LOD 调整**:
```cpp
// 在 EnemyBase 中
- Tick(DeltaTime):
  - DistanceToPlayer = GetDistanceToPlayer()
  - if DistanceToPlayer > FarThreshold:
    - SetAIUpdateInterval(0.5f) // 降低 AI 更新频率
    - SetMeshLOD(2)
  - else if DistanceToPlayer > MidThreshold:
    - SetAIUpdateInterval(0.2f)
    - SetMeshLOD(1)
  - else:
    - SetAIUpdateInterval(0.05f)
    - SetMeshLOD(0)
```

### 12.3 距离剔除

**自动清理远距离敌人**:
```cpp
// 在 EnemySpawnSubsystem 的 Tick 中
- Tick(DeltaTime):
  - for Enemy in ActiveEnemies:
    - if GetDistanceToPlayer(Enemy) > DespawnDistance:
      - DespawnEnemy(Enemy)
```

### 12.4 可见性剔除

**使用 UE5 的 Occlusion Culling**:
- 自动剔除不可见对象
- 配置 `Cull Distance Volume`

**手动剔除**:
- 只更新屏幕内或附近的敌人
- 远离相机的敌人暂停 AI

### 12.5 Async Loading

**异步加载资源**:
```cpp
// 使用 Asset Manager 异步加载
FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
Streamable.RequestAsyncLoad(
  AssetPath,
  FStreamableDelegate::CreateUObject(this, &OnAssetLoaded)
);
```

**用于**:
- 动态加载敌人类
- 加载特效和音效
- 加载 UI Widget

---

## 13. 数据管理架构

### 13.1 Data Assets vs Data Tables

**Data Asset（主资产）**:
- 用于: 武器配置、装备定义、技能定义
- 优势: 可以引用其他资产、支持继承

**Data Table（数据表）**:
- 用于: 敌人配置、掉落表、难度曲线
- 优势: 易于批量编辑、导入导出 CSV

**推荐用法**:
```
Data Assets:
  - UWeaponDataAsset
  - UItemDataAsset
  - UAbilityDataAsset

Data Tables:
  - DT_EnemyConfigs
  - DT_LootTables
  - DT_DifficultyCurves
  - DT_UpgradePool
```

### 13.2 Gameplay Tags

**使用 UE5 的 Gameplay Tags**

**用途**:
- 标记敌人类型（小型、中型、重型、精英、BOSS）
- 标记装备效果（火焰伤害、冰冻、减速）
- 标记技能状态（冷却中、激活中）
- 成就标记

**定义 Tags**:
```
// 在 Project Settings -> Gameplay Tags
GameplayTags:
  - Enemy.Type.Small
  - Enemy.Type.Medium
  - Enemy.Type.Heavy
  - Enemy.Type.Elite
  - Enemy.Type.Boss
  
  - Item.Type.Weapon
  - Item.Type.Passive
  - Item.Type.Active
  
  - Item.Rarity.Common
  - Item.Rarity.Rare
  - Item.Rarity.Epic
  - Item.Rarity.Legendary
  
  - Ability.Status.Active
  - Ability.Status.Cooldown
```

---

## 14. 网络架构考虑（未来扩展）

虽然当前是单机游戏，但使用 UE5 的网络架构可以为未来多人模式做准备：

**已经网络友好的设计**:
- ✅ 使用 GameState 存储游戏状态
- ✅ 使用 PlayerState 存储玩家状态
- ✅ 使用 Gameplay Ability System（如果采用）
- ✅ 伤害通过 Server 验证（ApplyDamage）

**注意事项**:
- 所有 Subsystems 默认只在 Server 运行
- 需要网络复制时使用 `GetLifetimeReplicatedProps()`
- 客户端预测使用 GAS 的 Prediction 功能

---

## 15. 调试与开发工具

### 15.1 使用 UE5 内置调试工具

**Console Commands**:
```cpp
// 在代码中注册 Console Commands
#if !UE_BUILD_SHIPPING
  // 开发模式下的调试命令
  UFUNCTION(Exec)
  void CheatGodMode();
  
  UFUNCTION(Exec)
  void CheatAddExperience(float Amount);
  
  UFUNCTION(Exec)
  void CheatSpawnBoss();
#endif
```

**Gameplay Debugger**:
- 启用 Gameplay Debugger Plugin
- 按 `'` 键显示调试信息
- 显示 AI 行为、感知、导航等

**Visual Logger**:
- 记录游戏事件和状态
- 可视化 AI 决策过程

### 15.2 性能分析工具

**Unreal Insights**:
- 详细性能分析
- CPU 和 GPU 分析
- 网络性能分析

**Stat Commands**:
```
stat FPS - 显示帧率
stat Unit - 显示各系统耗时
stat Game - 游戏逻辑统计
stat GPU - GPU 统计
stat Memory - 内存使用
```

---

## 16. 开发优先级与依赖关系

### 16.1 系统依赖图

```
核心 Gameplay Framework (GameMode, GameState, PlayerState)
  ↓
输入系统 (Enhanced Input) + 飞船角色
  ↓
武器系统 (WeaponComponent, Projectile)
  ↓
敌人系统 (EnemyBase, AI Controller, Behavior Tree)
  ↓
生成系统 (EnemySpawnSubsystem, ObjectPoolSubsystem)
  ↓
难度系统 (DifficultyManagerSubsystem)
  ↓
成长系统 (经验、升级)
  ↓
装备系统 (ItemDataAsset, InventoryComponent)
  ↓
UI 系统 (HUD, 升级界面)
  ↓
地图生成 (MapGeneratorSubsystem, PCG)
  ↓
元进度 (SaveGame, MetaProgressionSubsystem)
  ↓
特效音效优化
```

### 16.2 原型阶段（第一周）

**必须实现**:
- ✅ GameMode + GameState
- ✅ 飞船角色 + Enhanced Input
- ✅ 基础武器组件 + 子弹
- ✅ 简单敌人 + AI
- ✅ 对象池基础
- ✅ 简单 HUD（生命、时间、击杀）

**可以延后**:
- ⏸ 复杂地图生成
- ⏸ 多种武器类型
- ⏸ 完整 UI
- ⏸ 元进度系统

---

## 17. 最佳实践总结

### 17.1 UE5 架构原则

1. **使用 Subsystems**: 不要在 GameMode 中塞太多逻辑
2. **数据驱动**: 使用 DataAsset 和 DataTable，不要硬编码数值
3. **组件化**: 功能封装成 ActorComponent
4. **事件驱动**: 使用 Delegate 解耦系统
5. **对象池**: 频繁创建销毁的对象使用对象池
6. **异步加载**: 大资源使用异步加载

### 17.2 避免的反模式

❌ **不要在 Tick 中做复杂计算**
  - 使用 Timer 或事件驱动

❌ **不要在 Blueprint 中写核心逻辑**
  - C++ 写核心系统，Blueprint 用于配置和快速原型

❌ **不要在客户端做权威判定**
  - 即使是单机，也用 Server 思维设计

❌ **不要忽视内存管理**
  - 使用智能指针、对象池、及时清理引用

❌ **不要过早优化**
  - 先实现功能，再用 Profiler 找瓶颈

---

## 18. 插件推荐

### 18.1 官方插件

- ✅ **Enhanced Input** - 输入系统（必须）
- ✅ **Gameplay Ability System** - 技能系统（推荐）
- ✅ **PCG (Procedural Content Generation)** - 地图生成
- ✅ **Common UI** - UI 框架（可选）
- ✅ **Gameplay Debugger** - 调试工具
- ✅ **Niagara** - 粒子系统（默认启用）
- ✅ **MetaSounds** - 音频系统（默认启用）

### 18.2 第三方插件（可选）

- **Easy AI** - 简化 AI 开发
- **Audio Analyzer** - 音频分析（音乐可视化）
- **Async Loading Screen** - 异步加载屏幕

---

## 19. 下一步行动

### 19.1 第一阶段准备工作

1. **项目配置**:
   - 启用 Enhanced Input Plugin
   - 启用 PCG Plugin
   - 配置 World Partition（如果使用）
   - 创建基础文件夹结构

2. **创建基础类**:
   - GameMode, GameState, PlayerState
   - PlayerController
   - 飞船 Character
   - 敌人 Character

3. **创建 Subsystems**:
   - ObjectPoolSubsystem
   - DifficultyManagerSubsystem
   - EnemySpawnSubsystem

4. **创建数据资产**:
   - WeaponDataAsset
   - EnemyConfig DataTable
   - DifficultyCurve

5. **输入配置**:
   - 创建 Input Actions
   - 创建 Input Mapping Context
   - 绑定到 PlayerController

---

**文档版本**: 1.0  
**创建日期**: 2026-01-25  
**适用 UE 版本**: 5.3+  
**下次更新**: 原型开发开始后根据实际情况调整
