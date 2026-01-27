# 太空 Roguelike 游戏 - MVP 系统设计文档

## 文档说明
本文档定义最小可行产品（MVP）的范围和实现细节。
**目标**: 用最少的功能验证核心玩法是否有趣。
**技术选型**: Gameplay Ability System (GAS) + 固定大地图 + World Partition

---

## MVP 核心目标

### 验证的核心问题
1. ✅ 飞船操控是否流畅？
2. ✅ 射击手感是否爽快？
3. ✅ 敌人生成和难度递增是否合理？
4. ✅ 5-10 分钟的游戏循环是否有趣？

### MVP 功能范围

#### ✅ 必须有的功能
- 飞船移动和射击
- 1 种主武器
- 1-2 种敌人类型
- 基础 AI（追踪玩家）
- 敌人生成系统（固定频率递增）
- 基础难度递增（数量增加）
- 简单 HUD（生命值、时间、击杀数）
- 死亡和重新开始
- 固定大地图（带障碍物）

#### ❌ MVP 不包含的功能
- ❌ 多种武器类型
- ❌ 技能系统
- ❌ 装备系统
- ❌ 升级选择
- ❌ BOSS 系统
- ❌ 元进度系统
- ❌ 存档系统
- ❌ 复杂特效
- ❌ 程序化地图生成
- ❌ 多种敌人变种

---

## 1. 技术栈确认

### 1.1 核心技术选型

| 系统 | 技术选择 | 说明 |
|------|----------|------|
| **技能系统** | Gameplay Ability System (GAS) | 处理伤害、属性、射击 |
| **地图** | 固定大地图 + World Partition | 10000x10000 单位，自动分块 |
| **输入** | Enhanced Input System | UE5 标准 |
| **AI** | Behavior Tree + AIController | 简单追踪逻辑 |
| **特效** | Niagara（基础） | 只做必要特效 |
| **UI** | UMG | 最简 HUD |

### 1.2 MVP 架构简化

```
GameMode (游戏规则)
  └─ GameState (游戏状态：时间、难度)
      
PlayerController (输入)
  └─ SpaceShip (Character)
      ├─ AbilitySystemComponent (GAS)
      │   ├─ AttributeSet (生命值、伤害)
      │   └─ GameplayAbility_Fire (射击能力)
      └─ CameraComponent (俯视相机)

EnemyBase (Character)
  ├─ AbilitySystemComponent (GAS)
  ├─ AttributeSet
  └─ AIController + BehaviorTree
  
World Subsystems:
  └─ EnemySpawnSubsystem (敌人生成)
```

---

## 2. Gameplay Ability System (GAS) 配置

### 2.1 为什么 MVP 使用 GAS？

虽然 GAS 复杂，但 MVP 只用其基础功能：
- ✅ **AttributeSet** - 统一管理生命值、伤害等属性
- ✅ **GameplayAbility** - 射击逻辑
- ✅ **GameplayEffect** - 伤害应用
- ❌ **不用 Buff/Debuff**（MVP 阶段不需要）
- ❌ **不用复杂技能**（只有射击）

### 2.2 AttributeSet 设计

**类**: `USpaceShipAttributeSet : public UAttributeSet`

**MVP 阶段只需要的属性**:
```cpp
// 伪代码
UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health)
FGameplayAttributeData Health;

UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth)
FGameplayAttributeData MaxHealth;

UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData Damage; // 攻击力

UPROPERTY(BlueprintReadOnly)
FGameplayAttributeData MoveSpeed; // 移动速度
```

**属性初始化**:
```cpp
// 构造函数中
Health = 100.0f;
MaxHealth = 100.0f;
Damage = 10.0f;
MoveSpeed = 600.0f;
```

### 2.3 射击能力 - GameplayAbility

**类**: `UGA_Fire : public UGameplayAbility`

**职责**: 发射子弹

**实现流程**:
```cpp
// 伪代码
ActivateAbility():
  1. 检查是否可以激活（弹药、冷却等 - MVP 阶段不检查）
  2. 获取玩家朝向
  3. 生成子弹 Actor
  4. 应用子弹速度和方向
  5. 播放射击音效（可选）
  6. 播放 Muzzle Flash 特效（可选）
  7. EndAbility()
```

**Ability 配置**:
```cpp
// Ability Tags (Gameplay Tags)
AbilityTags: "Ability.Fire"

// 不需要冷却（MVP）
CooldownGameplayEffectClass: nullptr

// 不需要消耗（MVP）
CostGameplayEffectClass: nullptr

// 激活策略
InstancingPolicy: InstancedPerExecution // 每次激活创建新实例
NetExecutionPolicy: LocalPredicted // 客户端预测（虽然是单机）
```

### 2.4 伤害 GameplayEffect

**类**: `UGE_Damage : public UGameplayEffect`

**用途**: 应用伤害到目标

**配置**:
```cpp
// 伪代码
DurationPolicy: Instant // 瞬时效果

Modifiers:
  - Attribute: Health
  - ModifierOp: Add
  - Magnitude: -Damage (从 Ability 传入)
```

**应用伤害**:
```cpp
// 在子弹碰撞时
OnProjectileHit(HitResult):
  // 创建 Effect Context
  FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
  
  // 创建 Effect Spec
  FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
    UGE_Damage::StaticClass(), 
    1.0f, // Level
    EffectContext
  );
  
  // 设置伤害值
  SpecHandle.Data->SetSetByCallerMagnitude(
    FGameplayTag::RequestGameplayTag("Data.Damage"),
    DamageAmount
  );
  
  // 应用到目标
  TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
```

---

## 3. 角色系统

### 3.1 飞船 - `ASpaceShip`

**继承**: `ACharacter`

**组件**:
```cpp
// 伪代码
Components:
  - UAbilitySystemComponent* AbilitySystemComponent
  - USpaceShipAttributeSet* AttributeSet
  - UCameraComponent* Camera (俯视角)
  - USpringArmComponent* CameraBoom (相机臂)
  - UStaticMeshComponent* ShipMesh (飞船模型，替换 Character 的 Mesh)
```

**关键配置**:
```cpp
// 构造函数
ASpaceShip():
  // GAS 初始化
  AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("ASC");
  AttributeSet = CreateDefaultSubobject<USpaceShipAttributeSet>("AttributeSet");
  
  // 相机设置（俯视角）
  CameraBoom->SetRelativeRotation(FRotator(-60, 0, 0)); // 俯视角度
  CameraBoom->TargetArmLength = 1500.0f; // 相机距离
  Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
  
  // Character Movement 配置
  GetCharacterMovement()->MaxWalkSpeed = 600.0f;
  GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f; // 快速停止
  GetCharacterMovement()->GravityScale = 0.0f; // 太空中无重力
  GetCharacterMovement()->SetPlaneConstraintEnabled(true); // 限制在 XY 平面
  GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 1)); // 只在水平面移动
```

**移动逻辑**:
```cpp
// 在 PlayerController 或 Pawn 中
OnMove(FVector2D Input):
  // WASD 输入转换为世界空间移动
  FVector Forward = FVector(1, 0, 0); // X 轴
  FVector Right = FVector(0, 1, 0);   // Y 轴
  
  AddMovementInput(Forward, Input.Y);
  AddMovementInput(Right, Input.X);
```

**朝向逻辑**:
```cpp
// 鼠标指向飞船朝向
Tick(DeltaTime):
  // 获取鼠标世界位置
  FHitResult Hit;
  PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
  
  // 计算朝向
  FVector Direction = (Hit.Location - GetActorLocation()).GetSafeNormal2D();
  FRotator NewRotation = Direction.Rotation();
  
  // 平滑旋转
  SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 10.0f));
```

### 3.2 敌人 - `AEnemyBase`

**继承**: `ACharacter`

**组件**:
```cpp
// 伪代码
Components:
  - UAbilitySystemComponent* AbilitySystemComponent
  - UEnemyAttributeSet* AttributeSet (可以复用 SpaceShipAttributeSet)
  - UStaticMeshComponent* EnemyMesh
```

**AI 控制器**: `AEnemyAIController`

**Behavior Tree**: MVP 阶段只需要简单逻辑

```
BT_Enemy_Simple:
  Sequence
    ├─ Task: Find Player
    ├─ Task: Move To Player (使用 MoveToActor)
    └─ Task: Attack Player (如果在范围内)
```

**移动配置**:
```cpp
// 构造函数
GetCharacterMovement()->MaxWalkSpeed = 400.0f; // 比玩家慢
GetCharacterMovement()->GravityScale = 0.0f;
GetCharacterMovement()->SetPlaneConstraintEnabled(true);
```

---

## 4. 子弹系统

### 4.1 子弹 Actor - `AProjectile`

**继承**: `AActor`

**组件**:
```cpp
Components:
  - USphereComponent* CollisionComponent (碰撞)
  - UStaticMeshComponent* MeshComponent (子弹模型，MVP 可以用简单 Sphere)
  - UProjectileMovementComponent* MovementComponent (UE5 原生)
```

**配置**:
```cpp
// 构造函数
AProjectile():
  // 碰撞设置
  CollisionComponent->SetCollisionProfileName("Projectile");
  CollisionComponent->OnComponentHit.AddDynamic(this, &OnHit);
  
  // 移动组件
  MovementComponent->InitialSpeed = 2000.0f;
  MovementComponent->MaxSpeed = 2000.0f;
  MovementComponent->ProjectileGravityScale = 0.0f; // 无重力
  MovementComponent->bRotationFollowsVelocity = true;
```

**碰撞处理**:
```cpp
// 伪代码
OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit):
  if OtherActor->IsA(AEnemyBase):
    // 获取目标的 ASC
    UAbilitySystemComponent* TargetASC = OtherActor->GetAbilitySystemComponent();
    
    // 应用伤害 (使用 GameplayEffect)
    ApplyDamageEffect(TargetASC, DamageAmount);
    
  // 销毁子弹（MVP 不用对象池）
  Destroy();
```

### 4.2 射击逻辑（在 GAS Ability 中）

```cpp
// UGA_Fire::ActivateAbility()
伪代码:
  // 1. 获取射击位置（飞船前方）
  FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.0f;
  FRotator SpawnRotation = Owner->GetActorRotation();
  
  // 2. 生成子弹
  AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
    ProjectileClass,
    SpawnLocation,
    SpawnRotation
  );
  
  // 3. 设置子弹伤害（从 AttributeSet 读取）
  float DamageValue = AttributeSet->GetDamage();
  Projectile->SetDamage(DamageValue);
  
  // 4. 播放音效（可选）
  UGameplayStatics::PlaySoundAtLocation(this, FireSound, SpawnLocation);
  
  // 5. 结束 Ability
  EndAbility();
```

---

## 5. 敌人生成系统

### 5.1 敌人生成器 - `UEnemySpawnSubsystem`

**类型**: `UWorldSubsystem`

**MVP 简化逻辑**:
```cpp
// 伪代码
UEnemySpawnSubsystem:

Members:
  - TArray<AEnemyBase*> ActiveEnemies
  - FTimerHandle SpawnTimerHandle
  - float CurrentSpawnInterval = 2.0f // 初始 2 秒一个敌人
  - float GameTime = 0.0f
  
Initialize():
  // 启动生成 Timer
  GetWorld()->GetTimerManager().SetTimer(
    SpawnTimerHandle,
    this,
    &UEnemySpawnSubsystem::SpawnEnemy,
    CurrentSpawnInterval,
    true // 循环
  );

Deinitialize():
  // 清理 Timer
  GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
  
Tick(DeltaTime):
  GameTime += DeltaTime;
  
  // 难度递增：每 30 秒减少 10% 生成间隔
  if (FMath::Fmod(GameTime, 30.0f) < DeltaTime):
    CurrentSpawnInterval *= 0.9f;
    CurrentSpawnInterval = FMath::Max(CurrentSpawnInterval, 0.5f); // 最小 0.5 秒
    
    // 更新 Timer
    GetWorld()->GetTimerManager().SetTimer(
      SpawnTimerHandle,
      this,
      &UEnemySpawnSubsystem::SpawnEnemy,
      CurrentSpawnInterval,
      true
    );
  
  // 清理远距离敌人
  CleanupDistantEnemies();

SpawnEnemy():
  // 1. 计算生成位置（玩家周围屏幕外）
  FVector SpawnLocation = GetRandomSpawnLocation();
  
  // 2. 生成敌人
  AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(
    EnemyClass,
    SpawnLocation,
    FRotator::ZeroRotator
  );
  
  // 3. 应用属性缩放（根据游戏时间）
  ApplyEnemyScaling(Enemy, GameTime);
  
  // 4. 添加到活跃列表
  ActiveEnemies.Add(Enemy);

GetRandomSpawnLocation():
  // 获取玩家位置
  APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
  FVector PlayerLocation = Player->GetActorLocation();
  
  // 在玩家周围随机生成（屏幕外）
  float SpawnDistance = 2000.0f; // 距离玩家 2000 单位
  float RandomAngle = FMath::RandRange(0.0f, 360.0f);
  
  FVector Offset = FVector(
    FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
    FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
    0
  ) * SpawnDistance;
  
  return PlayerLocation + Offset;

ApplyEnemyScaling(Enemy, GameTime):
  // 简单缩放：每分钟 +10% 血量
  float HealthMultiplier = 1.0f + (GameTime / 60.0f) * 0.1f;
  
  UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
  if (ASC):
    // 创建临时 GameplayEffect 增加最大生命值
    // 或直接修改 AttributeSet (MVP 简化)
    UEnemyAttributeSet* Attributes = ASC->GetSet<UEnemyAttributeSet>();
    Attributes->SetMaxHealth(100.0f * HealthMultiplier);
    Attributes->SetHealth(100.0f * HealthMultiplier);

CleanupDistantEnemies():
  APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
  FVector PlayerLocation = Player->GetActorLocation();
  
  for (int i = ActiveEnemies.Num() - 1; i >= 0; i--):
    if (!IsValid(ActiveEnemies[i])):
      ActiveEnemies.RemoveAt(i);
      continue;
    
    float Distance = FVector::Dist(ActiveEnemies[i]->GetActorLocation(), PlayerLocation);
    if (Distance > 5000.0f): // 超过 5000 单位
      ActiveEnemies[i]->Destroy();
      ActiveEnemies.RemoveAt(i);
```

---

## 6. 地图系统

### 6.1 World Partition 设置

**创建大地图**:
1. 创建新 Level
2. 在 World Settings 中启用 World Partition
3. 配置网格大小

**World Settings 配置**:
```cpp
World Partition Settings:
  - Enable Streaming: true
  - Grid Size: 12800 units (UE5 默认推荐)
  - Loading Range: 51200 units (4 个 Grid)
  - Editor Cell Size: 12800
```

**工作流程**:
```
1. 在编辑器中打开 World Partition 窗口（Window -> World Partition）
2. 可以看到地图被自动分成网格
3. 只编辑当前加载的区域
4. 运行时自动加载/卸载网格
```

### 6.2 地图内容（MVP）

**地图尺寸**: 10000 x 10000 单位

**内容**:
- 简单的背景（太空星空 Skybox）
- 随机分布的小行星（静态 Mesh）
- 边界（隐形墙或循环传送）

**放置小行星**:
```cpp
// 在编辑器中或使用 Blueprint 放置
// MVP: 手动放置 50-100 个小行星即可

// 如果要脚本生成（可选）:
GenerateAsteroids():
  for (int i = 0; i < 100; i++):
    FVector RandomLocation = FVector(
      FMath::RandRange(-5000.0f, 5000.0f),
      FMath::RandRange(-5000.0f, 5000.0f),
      0
    );
    
    GetWorld()->SpawnActor<AAsteroid>(
      AsteroidClass,
      RandomLocation,
      FRotator::ZeroRotator
    );
```

### 6.3 地图边界处理

**方案 A: 隐形墙（最简单）**

```cpp
// 在地图四周放置 Box Collision
地图边界:
  - 北: Y = 5000, Box Collision (Block All)
  - 南: Y = -5000, Box Collision (Block All)
  - 东: X = 5000, Box Collision (Block All)
  - 西: X = -5000, Box Collision (Block All)
```

**方案 B: 循环传送（更有趣）**

```cpp
// 在 SpaceShip 的 Tick 中
Tick(DeltaTime):
  FVector Location = GetActorLocation();
  
  // 超出边界时传送到对侧
  if (Location.X > 5000.0f):
    SetActorLocation(FVector(-5000.0f, Location.Y, Location.Z));
  else if (Location.X < -5000.0f):
    SetActorLocation(FVector(5000.0f, Location.Y, Location.Z));
    
  if (Location.Y > 5000.0f):
    SetActorLocation(FVector(Location.X, -5000.0f, Location.Z));
  else if (Location.Y < -5000.0f):
    SetActorLocation(FVector(Location.X, 5000.0f, Location.Z));
```

---

## 7. UI 系统

### 7.1 HUD Widget - `UW_GameHUD`

**继承**: `UUserWidget`

**显示元素**:
```
Canvas Panel
  ├─ Health Bar (UProgressBar)
  │   └─ Text: "HP: 100 / 100"
  │
  ├─ Timer Text (UTextBlock)
  │   └─ "Time: 00:00"
  │
  ├─ Kill Count (UTextBlock)
  │   └─ "Kills: 0"
  │
  └─ Crosshair (UImage) - 中心准星
```

**数据绑定**:
```cpp
// UW_GameHUD.cpp (伪代码)

// 绑定到 SpaceShip 的 AttributeSet
OnConstruct():
  ASpaceShip* Player = Cast<ASpaceShip>(GetOwningPlayerPawn());
  if (Player):
    UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
    
    // 监听生命值变化
    ASC->GetGameplayAttributeValueChangeDelegate(
      USpaceShipAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &UW_GameHUD::OnHealthChanged);

OnHealthChanged(FOnAttributeChangeData Data):
  // 更新 Health Bar
  float HealthPercent = Data.NewValue / AttributeSet->GetMaxHealth();
  HealthBar->SetPercent(HealthPercent);
  
  // 更新文本
  HealthText->SetText(FText::FromString(
    FString::Printf(TEXT("HP: %.0f / %.0f"), Data.NewValue, AttributeSet->GetMaxHealth())
  ));

NativeTick(DeltaTime):
  // 更新计时器
  UpdateTimer();
  
  // 更新击杀数（从 GameState 读取）
  UpdateKillCount();

UpdateTimer():
  AGameStateBase* GameState = GetWorld()->GetGameState();
  float GameTime = GameState->GetServerWorldTimeSeconds();
  
  int Minutes = (int)(GameTime / 60.0f);
  int Seconds = (int)GameTime % 60;
  
  TimerText->SetText(FText::FromString(
    FString::Printf(TEXT("Time: %02d:%02d"), Minutes, Seconds)
  ));
```

### 7.2 死亡界面 - `UW_GameOver`

**显示元素**:
```
Canvas Panel
  ├─ Background (Black with opacity)
  ├─ Title Text: "YOU DIED"
  ├─ Stats:
  │   ├─ "Survival Time: XX:XX"
  │   └─ "Kills: XXX"
  └─ Restart Button
```

**逻辑**:
```cpp
// 伪代码
OnRestartButtonClicked():
  // 重新加载当前关卡
  UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
```

---

## 8. GameMode 和 GameState

### 8.1 GameMode - `ASpaceRoguelikeGameMode`

**职责**:
- 定义默认 Pawn、PlayerController、HUD 类
- 处理玩家死亡

```cpp
// 伪代码
ASpaceRoguelikeGameMode():
  // 设置默认类
  DefaultPawnClass = ASpaceShip::StaticClass();
  PlayerControllerClass = ASpaceRoguelikePlayerController::StaticClass();
  GameStateClass = ASpaceRoguelikeGameState::StaticClass();

StartPlay():
  Super::StartPlay();
  
  // 初始化 Subsystems（自动初始化）
  // 显示 HUD
  // ...

OnPlayerDeath(AController* Player):
  // 显示死亡界面
  ASpaceRoguelikePlayerController* PC = Cast<ASpaceRoguelikePlayerController>(Player);
  if (PC):
    PC->ShowGameOverUI();
```

### 8.2 GameState - `ASpaceRoguelikeGameState`

**职责**: 存储游戏状态（供 UI 读取）

```cpp
// 伪代码
ASpaceRoguelikeGameState:

UPROPERTY(Replicated)
int32 TotalKills = 0;

void AddKill():
  TotalKills++;
  OnKillCountChanged.Broadcast(TotalKills);

float GetGameTime():
  return GetServerWorldTimeSeconds();
```

---

## 9. 输入系统

### 9.1 Enhanced Input 配置

**Input Actions**:
```
IA_Move (Input Action)
  - Value Type: Axis2D (Vector2D)
  - Triggers: None
  - Modifiers: None

IA_Fire (Input Action)
  - Value Type: Digital (bool)
  - Triggers: Pressed, Released
  - Modifiers: None

IA_Pause (Input Action)
  - Value Type: Digital (bool)
  - Triggers: Pressed
  - Modifiers: None
```

**Input Mapping Context**:
```
IMC_Gameplay (Input Mapping Context)
  - IA_Move -> WASD, Arrow Keys
  - IA_Fire -> Left Mouse Button, Space
  - IA_Pause -> Esc
```

### 9.2 PlayerController 绑定

```cpp
// ASpaceRoguelikePlayerController (伪代码)

SetupInputComponent():
  Super::SetupInputComponent();
  
  UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
  
  // 绑定移动
  EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &OnMove);
  
  // 绑定射击
  EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &OnFirePressed);
  EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &OnFireReleased);
  
  // 绑定暂停
  EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &OnPause);

BeginPlay():
  Super::BeginPlay();
  
  // 添加 Input Mapping Context
  UEnhancedInputLocalPlayerSubsystem* Subsystem = 
    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
  
  if (Subsystem):
    Subsystem->AddMappingContext(IMC_Gameplay, 0);

OnMove(const FInputActionValue& Value):
  FVector2D MoveVector = Value.Get<FVector2D>();
  
  // 传递给 Pawn
  APawn* ControlledPawn = GetPawn();
  if (ControlledPawn):
    // 在世界空间中移动
    ControlledPawn->AddMovementInput(FVector(1, 0, 0), MoveVector.Y);
    ControlledPawn->AddMovementInput(FVector(0, 1, 0), MoveVector.X);

OnFirePressed(const FInputActionValue& Value):
  ASpaceShip* Ship = Cast<ASpaceShip>(GetPawn());
  if (Ship):
    Ship->StartFiring();

OnFireReleased(const FInputActionValue& Value):
  ASpaceShip* Ship = Cast<ASpaceShip>(GetPawn());
  if (Ship):
    Ship->StopFiring();
```

### 9.3 飞船射击逻辑

```cpp
// ASpaceShip (伪代码)

StartFiring():
  // MVP: 持续射击（使用 Timer）
  if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle)):
    // 立即射击一次
    Fire();
    
    // 启动定时器（每 0.2 秒射击一次）
    GetWorld()->GetTimerManager().SetTimer(
      FireTimerHandle,
      this,
      &ASpaceShip::Fire,
      0.2f,
      true
    );

StopFiring():
  GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

Fire():
  // 通过 GAS 激活射击 Ability
  if (AbilitySystemComponent):
    AbilitySystemComponent->TryActivateAbilityByClass(UGA_Fire::StaticClass());
```

---

## 10. AI 系统

### 10.1 敌人 AI Controller

**类**: `AEnemyAIController`

```cpp
// 伪代码
AEnemyAIController:

OnPossess(APawn* InPawn):
  Super::OnPossess(InPawn);
  
  // 运行 Behavior Tree
  RunBehaviorTree(EnemyBehaviorTree);

// AI Perception 配置（可选，MVP 可以直接用 GetPlayerPawn）
```

### 10.2 Behavior Tree - `BT_Enemy_Chase`

**结构**（非常简单）:
```
Root
  └─ Sequence
      ├─ Service: Update Player Location (每 0.5 秒)
      └─ Task: Move To Player (使用 MoveToLocation 或 MoveToActor)
```

**Blackboard Keys**:
```
BB_Enemy:
  - TargetActor (Object) - 玩家 Actor
  - TargetLocation (Vector) - 玩家位置
```

**Service - `BTService_UpdatePlayerLocation`**:
```cpp
// 伪代码
TickNode(DeltaTime):
  // 获取玩家
  APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
  
  // 更新 Blackboard
  Blackboard->SetValueAsObject("TargetActor", Player);
  Blackboard->SetValueAsVector("TargetLocation", Player->GetActorLocation());
```

**Task - Move To Player**:
- 使用 UE5 内置的 `Move To` Task
- Target: Blackboard Key "TargetActor"
- Acceptable Radius: 100.0f

### 10.3 敌人攻击逻辑（MVP 简化）

**方案**: 碰撞伤害（最简单）

```cpp
// 在 AEnemyBase 中
BeginPlay():
  // 设置碰撞检测
  GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
    this,
    &AEnemyBase::OnOverlapBegin
  );

OnOverlapBegin(OtherActor):
  if (OtherActor->IsA(ASpaceShip::StaticClass())):
    // 造成伤害（通过 GAS）
    UAbilitySystemComponent* TargetASC = OtherActor->GetAbilitySystemComponent();
    
    // 应用伤害 Effect
    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
      UGE_Damage::StaticClass(),
      1.0f,
      Context
    );
    
    // 设置伤害值
    Spec.Data->SetSetByCallerMagnitude(
      FGameplayTag::RequestGameplayTag("Data.Damage"),
      10.0f // MVP: 固定伤害
    );
    
    TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    
    // 敌人自毁
    Destroy();
```

---

## 11. 生命值和死亡系统

### 11.1 监听生命值变化

```cpp
// 在 ASpaceShip::BeginPlay() 中
BeginPlay():
  Super::BeginPlay();
  
  if (AbilitySystemComponent && AttributeSet):
    // 监听生命值变化
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
      USpaceShipAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &ASpaceShip::OnHealthChanged);

OnHealthChanged(const FOnAttributeChangeData& Data):
  UE_LOG(LogTemp, Log, TEXT("Health changed: %.2f"), Data.NewValue);
  
  // 检查死亡
  if (Data.NewValue <= 0.0f):
    Die();

Die():
  // 禁用输入
  APlayerController* PC = Cast<APlayerController>(GetController());
  if (PC):
    DisableInput(PC);
  
  // 通知 GameMode
  ASpaceRoguelikeGameMode* GM = Cast<ASpaceRoguelikeGameMode>(
    UGameplayStatics::GetGameMode(this)
  );
  if (GM):
    GM->OnPlayerDeath(GetController());
  
  // 播放死亡动画/特效（MVP 可选）
  
  // 不销毁 Actor（保持在场景中）
```

### 11.2 敌人死亡

```cpp
// AEnemyBase (伪代码)
BeginPlay():
  // 监听生命值
  AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    UEnemyAttributeSet::GetHealthAttribute()
  ).AddUObject(this, &AEnemyBase::OnHealthChanged);

OnHealthChanged(const FOnAttributeChangeData& Data):
  if (Data.NewValue <= 0.0f):
    Die();

Die():
  // 通知 GameState 增加击杀数
  ASpaceRoguelikeGameState* GS = GetWorld()->GetGameState<ASpaceRoguelikeGameState>();
  if (GS):
    GS->AddKill();
  
  // 播放死亡特效（可选）
  
  // 移除自己从 Subsystem 的列表
  UEnemySpawnSubsystem* SpawnSys = GetWorld()->GetSubsystem<UEnemySpawnSubsystem>();
  if (SpawnSys):
    SpawnSys->OnEnemyDied(this);
  
  // 销毁
  Destroy();
```

---

## 12. 性能优化（MVP 阶段）

### 12.1 基本优化

**必须做的**:
- ✅ 使用 World Partition（自动优化）
- ✅ 限制同屏敌人数量（在 SpawnSubsystem 中）
- ✅ 清理远距离敌人

**MVP 不需要的**:
- ❌ 对象池（先让系统跑起来）
- ❌ LOD（敌人简单模型）
- ❌ 复杂的 Occlusion Culling

### 12.2 同屏敌人限制

```cpp
// UEnemySpawnSubsystem
MaxActiveEnemies = 50; // MVP 限制

SpawnEnemy():
  if (ActiveEnemies.Num() >= MaxActiveEnemies):
    return; // 不生成新敌人
  
  // ... 生成逻辑
```

---

## 13. MVP 开发路线

### 13.1 第一周目标（基础框架）

**Day 1-2: 项目设置和角色**
- [ ] 创建 UE5 项目
- [ ] 启用 GAS Plugin
- [ ] 启用 Enhanced Input Plugin
- [ ] 创建 SpaceShip Character（基础移动）
- [ ] 创建 EnemyBase Character
- [ ] 配置 World Partition

**Day 3-4: GAS 和战斗**
- [ ] 创建 AttributeSet（生命值、伤害）
- [ ] 创建 GA_Fire（射击 Ability）
- [ ] 创建 GE_Damage（伤害 Effect）
- [ ] 创建 Projectile Actor
- [ ] 测试射击和伤害

**Day 5-6: AI 和生成**
- [ ] 创建 EnemyAIController
- [ ] 创建简单 Behavior Tree
- [ ] 创建 EnemySpawnSubsystem
- [ ] 测试敌人生成和追踪

**Day 7: UI 和整合**
- [ ] 创建 HUD Widget
- [ ] 创建 GameOver Widget
- [ ] 实现死亡逻辑
- [ ] 整合测试

### 13.2 第二周目标（完善 MVP）

**Day 8-9: 地图和环境**
- [ ] 创建 10000x10000 地图
- [ ] 放置小行星障碍物
- [ ] 配置地图边界
- [ ] 添加背景和光照

**Day 10-11: 难度系统**
- [ ] 实现生成频率递增
- [ ] 实现敌人属性缩放
- [ ] 调整难度曲线
- [ ] 测试 10 分钟游戏

**Day 12-13: 音效和特效**
- [ ] 添加射击音效
- [ ] 添加基础 Niagara 特效（子弹轨迹、爆炸）
- [ ] 添加背景音乐
- [ ] UI 动画（可选）

**Day 14: 优化和测试**
- [ ] 性能测试
- [ ] Bug 修复
- [ ] 平衡性调整
- [ ] MVP 完成！

---

## 14. MVP 成功标准

### 14.1 核心指标

**技术指标**:
- ✅ 稳定 60 FPS（50+ 敌人同屏）
- ✅ 无崩溃运行 10 分钟
- ✅ 无明显内存泄漏

**玩法指标**:
- ✅ 飞船操控流畅响应
- ✅ 射击手感良好
- ✅ 敌人 AI 正常工作
- ✅ 难度曲线合理（5 分钟能存活，10 分钟有挑战）

**可玩性指标**:
- ✅ 玩 5 分钟不觉得无聊
- ✅ 有挑战性，但不过分困难
- ✅ 有想再玩一次的欲望

### 14.2 测试清单

**基础功能测试**:
- [ ] WASD 移动正常
- [ ] 鼠标朝向正常
- [ ] 点击射击正常
- [ ] 子弹击中敌人造成伤害
- [ ] 敌人追踪玩家
- [ ] 敌人碰撞玩家造成伤害
- [ ] 生命值归零触发死亡
- [ ] 死亡显示结算界面
- [ ] 可以重新开始游戏

**性能测试**:
- [ ] 开局帧率稳定
- [ ] 5 分钟后帧率稳定
- [ ] 10 分钟后帧率稳定
- [ ] 内存使用正常

**难度测试**:
- [ ] 0-2 分钟很轻松
- [ ] 3-5 分钟开始有压力
- [ ] 5-10 分钟有明显挑战
- [ ] 10 分钟后很困难

---

## 15. MVP 之后的迭代方向

### 15.1 短期迭代（MVP + 1）

**优先添加**:
- 多种武器类型（激光、导弹）
- 简单升级系统（3 选 1）
- 第二种敌人类型
- 更好的特效和音效

### 15.2 中期迭代（MVP + 2）

**功能扩展**:
- 技能系统（护盾、闪避）
- 装备掉落
- BOSS 系统
- 更多敌人变种

### 15.3 长期迭代（完整版本）

**完整功能**:
- 元进度系统
- 多种飞船
- 成就系统
- 存档系统
- 完整内容（30+ 装备、10+ 敌人）

---

## 16. 常见问题和解决方案

### 16.1 GAS 相关

**Q: GAS 太复杂，MVP 需要全部吗？**
A: 不需要。MVP 只用：
- AttributeSet（属性）
- GameplayAbility（射击）
- GameplayEffect（伤害）
不用 Buff、预测、Tag 系统等高级功能。

**Q: 如何调试 GAS？**
A: 使用 Console Command：
```
showdebug abilitysystem
```

### 16.2 World Partition 相关

**Q: World Partition 会自动工作吗？**
A: 是的。只需在 World Settings 中启用，运行时自动加载/卸载。

**Q: 如何看到当前加载的网格？**
A: 打开 World Partition 窗口（Window -> World Partition），绿色区域是已加载的。

### 16.3 性能相关

**Q: 敌人太多卡顿怎么办？**
A: 
1. 降低 MaxActiveEnemies
2. 增加 DespawnDistance
3. 使用简单的敌人 Mesh

**Q: 子弹太多卡顿怎么办？**
A:
1. 减少子弹生命周期
2. 增加 Fire Interval（降低射速）
3. 使用更简单的子弹模型

---

## 17. 文件结构

### 17.1 推荐的项目文件夹结构

```
Content/
├─ SpaceRoguelike/
│   ├─ Core/
│   │   ├─ GameModes/
│   │   │   └─ BP_SpaceRoguelikeGameMode
│   │   ├─ GameStates/
│   │   │   └─ BP_SpaceRoguelikeGameState
│   │   └─ PlayerController/
│   │       └─ BP_SpaceRoguelikePlayerController
│   │
│   ├─ Characters/
│   │   ├─ SpaceShip/
│   │   │   ├─ BP_SpaceShip
│   │   │   ├─ ABP_SpaceShip (Animation Blueprint)
│   │   │   └─ M_SpaceShip (Material)
│   │   └─ Enemies/
│   │       ├─ BP_EnemyBase
│   │       └─ M_Enemy
│   │
│   ├─ GAS/
│   │   ├─ Attributes/
│   │   │   └─ DA_AttributeSet_SpaceShip (Data Asset)
│   │   ├─ Abilities/
│   │   │   ├─ GA_Fire
│   │   │   └─ GA_TakeDamage
│   │   └─ Effects/
│   │       └─ GE_Damage
│   │
│   ├─ Weapons/
│   │   └─ Projectiles/
│   │       └─ BP_Projectile
│   │
│   ├─ AI/
│   │   ├─ BehaviorTrees/
│   │   │   └─ BT_Enemy_Chase
│   │   ├─ Blackboards/
│   │   │   └─ BB_Enemy
│   │   └─ AIControllers/
│   │       └─ BP_EnemyAIController
│   │
│   ├─ UI/
│   │   ├─ HUD/
│   │   │   └─ WBP_GameHUD
│   │   └─ Menus/
│   │       └─ WBP_GameOver
│   │
│   ├─ Input/
│   │   ├─ Actions/
│   │   │   ├─ IA_Move
│   │   │   ├─ IA_Fire
│   │   │   └─ IA_Pause
│   │   └─ Contexts/
│   │       └─ IMC_Gameplay
│   │
│   ├─ Maps/
│   │   └─ L_MainGame (World Partition Enabled)
│   │
│   ├─ VFX/
│   │   └─ Niagara/
│   │       ├─ NS_MuzzleFlash
│   │       └─ NS_Explosion
│   │
│   └─ Audio/
│       ├─ SFX/
│       │   ├─ S_Fire
│       │   └─ S_Explosion
│       └─ Music/
│           └─ M_BGM
│
└─ ThirdParty/ (如有外部资源)
```

### 17.2 C++ 源码结构

```
Source/VibeCoding/
├─ SpaceRoguelike/
│   ├─ Core/
│   │   ├─ SpaceRoguelikeGameMode.h/cpp
│   │   ├─ SpaceRoguelikeGameState.h/cpp
│   │   └─ SpaceRoguelikePlayerController.h/cpp
│   │
│   ├─ Characters/
│   │   ├─ SpaceShip.h/cpp
│   │   └─ EnemyBase.h/cpp
│   │
│   ├─ GAS/
│   │   ├─ Attributes/
│   │   │   └─ SpaceShipAttributeSet.h/cpp
│   │   ├─ Abilities/
│   │   │   ├─ GA_Fire.h/cpp
│   │   │   └─ GameplayAbilityBase.h/cpp
│   │   └─ Effects/
│   │       └─ GE_Damage.h/cpp
│   │
│   ├─ Weapons/
│   │   └─ Projectile.h/cpp
│   │
│   ├─ AI/
│   │   ├─ EnemyAIController.h/cpp
│   │   └─ BTService_UpdatePlayerLocation.h/cpp
│   │
│   ├─ Subsystems/
│   │   └─ EnemySpawnSubsystem.h/cpp
│   │
│   └─ UI/
│       └─ GameHUDWidget.h/cpp
```

---

## 18. 总结

### 18.1 MVP 的核心价值

这个 MVP 的目标是用最少的功能验证：
1. **核心玩法是否有趣** - 飞船射击、躲避敌人
2. **难度曲线是否合理** - 能持续 5-10 分钟的挑战
3. **技术架构是否可行** - GAS + World Partition 是否适合

### 18.2 关键决策

**使用 GAS 的理由**:
- ✅ 统一的属性管理
- ✅ 为后续技能系统打基础
- ✅ 学习 UE5 推荐的架构

**使用 World Partition 的理由**:
- ✅ 自动分块加载，无需手动管理
- ✅ 为后续大型地图扩展打基础
- ✅ 编辑器性能更好

**MVP 简化的理由**:
- ✅ 快速验证核心玩法
- ✅ 避免过度设计
- ✅ 保持灵活性，根据测试结果调整

### 18.3 下一步

完成 MVP 后：
1. 测试玩法是否有趣
2. 收集数据（平均生存时间、击杀数）
3. 根据测试结果决定迭代方向
4. 逐步添加功能（升级系统、装备系统等）

---

**文档版本**: 1.0  
**创建日期**: 2026-01-25  
**适用 UE 版本**: 5.3+  
**预计开发时间**: 2 周  
**目标**: 可玩的 MVP，验证核心玩法
