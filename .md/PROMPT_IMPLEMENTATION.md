# Beekeeper Flashlight Toggle 구현 프롬프트

## 목표

키보드 `T` 키로 On/Off 토글할 수 있는 1인칭 손전등 기능을 구현한다.

손전등은 `ABeekeeperCharacter`의 입력으로 토글되지만, 조명 상태와 조명 컴포넌트 관리는 별도 캐릭터 컴포넌트가 소유한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 범위

추가 대상:

- `Source/BeekeepingSim/Public/Character/BeekeeperFlashlightComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperFlashlightComponent.cpp`

수정 대상:

- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `.md/Architecture/CharacterSystem.md`
- 필요 시 `.md/0_ARCHITECTURE.md`
- 필요 시 `.md/USER_UNREAL.md`

수정하지 말 것:

- `Content/` asset 직접 수정 또는 resave
- `Config/DefaultEngine.ini`
- Focus/Inventory/UI/Environment 시스템 로직
- 기존 입력 action property rename/delete

## 확정 요구사항

### 입력

- 손전등 토글 키는 `T`다.
- C++에는 `FlashlightToggleAction` 입력 액션 property를 추가한다.
- 실제 `T` 키 매핑은 Unreal Editor에서 Input Action/Mapping Context asset에 설정하는 수동 작업으로 남긴다.
- 기존 `PartFocusFAction`, `PartFocusRAction`, `PartFocusCAction` 등 기존 입력 property는 변경하지 않는다.

### 책임 분리

- `ABeekeeperCharacter`는 입력 라우터와 컴포넌트 조립 지점이다.
- 손전등 상태와 `USpotLightComponent` 관리는 `UBeekeeperFlashlightComponent`가 담당한다.
- `ABeekeeperCharacter`는 입력을 받아 `UBeekeeperFlashlightComponent::ToggleFlashlight()`만 호출한다.

### 부착 기준

- 손전등 조명은 `FirstPersonCamera` 기준으로 부착한다.
- 카메라 방향을 따라가야 하므로 mesh hand/head socket 기준 부착을 기본값으로 사용하지 않는다.
- `FirstPersonCamera`가 focus camera override 중 detach/reattach 되더라도, 손전등은 카메라 하위 컴포넌트로 유지되어 현재 카메라 방향을 따라가야 한다.

### 로컬/복제 정책

- 현재 단계는 로컬 1인칭 손전등으로 구현한다.
- replication, 서버 RPC, 다른 플레이어용 3인칭 손전등은 구현하지 않는다.
- 손전등 조명이 gameplay authority나 AI 감지 source가 되지 않도록 한다.

### Focus 입력 정책

- 손전등 토글은 focus interaction input lock 중에도 허용한다.
- `MoveInput`, `LookInput`, `SprintStartInput`, `DoJumpStart`처럼 `bIsFocusInteractionInputLocked`로 차단하지 않는다.
- UI modal에서 입력을 막아야 하는 경우는 Mapping Context 우선순위/asset 설정으로 처리한다.

## 신규 컴포넌트 설계

`UBeekeeperFlashlightComponent`를 `UActorComponent` 또는 `USceneComponent` 중 하나로 구현한다.

권장:

- `UActorComponent`로 만들고 내부에서 `USpotLightComponent` default subobject를 생성한다.
- `InitializeFlashlightAttachment(USceneComponent* AttachParent)` 또는 `InitializeFlashlightAttachment(UCameraComponent* Camera)`를 제공해 캐릭터가 카메라를 넘긴다.

대안:

- `USceneComponent`로 만들고 자신을 카메라에 붙인 뒤 자식 `USpotLightComponent`를 가진다.

어느 방식을 선택하든 아래 외부 계약은 유지한다.

```cpp
void ToggleFlashlight();
void SetFlashlightEnabled(bool bEnabled);
bool IsFlashlightEnabled() const;
```

## 주요 property

`UBeekeeperFlashlightComponent`에 Details 조절용 property를 둔다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
bool bStartEnabled = false;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (ClampMin = "0.0"))
float Intensity = 5000.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (ClampMin = "0.0"))
float AttenuationRadius = 1200.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (ClampMin = "0.0", ClampMax = "89.0"))
float InnerConeAngle = 18.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (ClampMin = "0.0", ClampMax = "89.0"))
float OuterConeAngle = 32.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
FLinearColor LightColor = FLinearColor::White;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
bool bCastShadows = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
FVector RelativeLocation = FVector::ZeroVector;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight")
FRotator RelativeRotation = FRotator::ZeroRotator;
```

내부 상태:

```cpp
UPROPERTY(Transient)
bool bIsFlashlightEnabled = false;
```

SpotLight 참조:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight", meta = (AllowPrivateAccess = "true"))
TObjectPtr<USpotLightComponent> FlashlightSpotLight;
```

필요하면 이름은 기존 스타일에 맞춰 조정해도 되지만, UCLASS/UPROPERTY rename이 필요한 구조는 만들지 않는다.

## 권장 함수 구조

`UBeekeeperFlashlightComponent`:

```cpp
UBeekeeperFlashlightComponent();
virtual void BeginPlay() override;
void InitializeFlashlightAttachment(USceneComponent* AttachParent);
void ToggleFlashlight();
void SetFlashlightEnabled(bool bEnabled);
bool IsFlashlightEnabled() const;
void ApplyFlashlightSettings();
```

동작:

1. 생성자에서 `FlashlightSpotLight`를 생성한다.
2. 기본 visibility는 꺼진 상태로 둔다.
3. `BeginPlay()`에서 `bStartEnabled`를 `SetFlashlightEnabled()`로 적용한다.
4. `InitializeFlashlightAttachment()`에서 카메라에 attach하고 relative transform을 적용한다.
5. `ApplyFlashlightSettings()`에서 intensity, radius, cone angle, color, shadow 설정을 `USpotLightComponent`에 반영한다.
6. `SetFlashlightEnabled()`는 내부 상태와 `FlashlightSpotLight->SetVisibility(...)`를 함께 갱신한다.

`ABeekeeperCharacter`:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
TObjectPtr<UBeekeeperFlashlightComponent> BeekeeperFlashlight;

UPROPERTY(EditDefaultsOnly, Category = "Inputs")
TObjectPtr<UInputAction> FlashlightToggleAction;

void FlashlightToggleInput();

UBeekeeperFlashlightComponent* GetBeekeeperFlashlight() const;
```

동작:

1. 생성자에서 `BeekeeperFlashlight = CreateDefaultSubobject<UBeekeeperFlashlightComponent>(TEXT("BeekeeperFlashlight"));`
2. `BeginPlay()`에서 `BeekeeperFlashlight->InitializeFlashlightAttachment(FirstPersonCamera);`
3. `SetupPlayerInputComponent()`에서 `FlashlightToggleAction`이 있으면 `ETriggerEvent::Started`로 `FlashlightToggleInput()`을 바인딩한다.
4. `FlashlightToggleInput()`은 focus input lock 여부와 무관하게 `BeekeeperFlashlight->ToggleFlashlight()`를 호출한다.

## Blueprint/API 영향

- 새 component property와 새 input action property를 추가한다.
- 기존 Blueprint API rename/delete는 하지 않는다.
- 기존 component 이름은 변경하지 않는다.
- UCLASS/USTRUCT/UENUM rename이 없으므로 Core Redirect는 필요하지 않아야 한다.
- `BP_BeekeeperCharacter`에서 새 `FlashlightToggleAction` property 할당이 필요하다.

## Editor 수동 작업

구현 후 Unreal Editor에서 아래 작업이 필요하다.

1. `IA_FlashlightToggle` Input Action asset을 만든다.
2. 기존 player Mapping Context에 `IA_FlashlightToggle`을 추가한다.
3. 키를 `T`로 지정한다.
4. `BP_BeekeeperCharacter`의 `FlashlightToggleAction`에 `IA_FlashlightToggle`을 할당한다.
5. `BP_BeekeeperCharacter`에서 `BeekeeperFlashlight`의 intensity/radius/cone/shadow 값을 조정한다.
6. Blueprint compile/save를 수행한다.

Content asset은 구현 에이전트가 직접 수정하지 않는다. 위 작업은 사용자 또는 Unreal Editor 작업 프롬프트로 넘긴다.

## 문서 반영

구현 후 갱신:

- `.md/Architecture/CharacterSystem.md`
  - `UBeekeeperFlashlightComponent`를 Scope/Key Classes/Runtime Flow/Design Notes/Manual Review Points에 추가한다.
  - 손전등은 Character local input + camera-attached local visual component라고 기록한다.
- `.md/USER_UNREAL.md`
  - `IA_FlashlightToggle` 생성, Mapping Context에 `T` 추가, `BP_BeekeeperCharacter` property 할당 절차를 기록한다.

필요 시 갱신:

- `.md/0_ARCHITECTURE.md`
  - Character source 파일 수 또는 Source 구조 설명이 실제 파일 수와 맞지 않게 되면 갱신한다.

## 검증 기준

### 빌드

가능하면 아래 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

Live Coding이 활성화되어 빌드가 실패하면 Editor를 종료하거나 Live Coding을 끄고 다시 빌드한다.

### 코드 검증

- `ABeekeeperCharacter`에 `BeekeeperFlashlight` component가 생성되어야 한다.
- `ABeekeeperCharacter`에 `FlashlightToggleAction`이 추가되어야 한다.
- `SetupPlayerInputComponent()`에서 `FlashlightToggleAction` null check 후 `Started` 이벤트에 바인딩되어야 한다.
- `FlashlightToggleInput()`은 `bIsFocusInteractionInputLocked`로 차단되면 안 된다.
- `UBeekeeperFlashlightComponent`는 `USpotLightComponent`를 소유해야 한다.
- 손전등 spotlight는 `FirstPersonCamera`에 attach되어야 한다.
- `SetFlashlightEnabled(false)` 상태에서 spotlight visibility가 꺼져야 한다.
- `SetFlashlightEnabled(true)` 상태에서 spotlight visibility가 켜져야 한다.
- replication 코드를 추가하지 않아야 한다.

### 수동 검증

Editor/PIE에서 확인:

1. `T`를 누르면 손전등이 켜진다.
2. 다시 `T`를 누르면 손전등이 꺼진다.
3. 마우스로 시점을 움직이면 손전등 방향이 카메라 방향을 따라간다.
4. FocusEngaged 또는 focus input lock 상태에서도 `T` 토글이 동작한다.
5. 이동/점프/스프린트/focus/hotbar 입력이 기존처럼 동작한다.
6. `BeekeeperFlashlight` Details 값 변경으로 밝기, 거리, 콘 각도, 그림자 여부를 조절할 수 있다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 프로젝트에 이미 손전등 또는 torch 관련 컴포넌트/입력 구조가 존재한다.
- `T` 키가 기존 필수 gameplay 입력과 충돌하는 것으로 확인된다.
- `USpotLightComponent` 생성/소유 위치가 Blueprint native parent와 충돌한다.
- 손전등을 다른 플레이어에게도 보여야 하는 multiplayer 요구가 확인된다.
- 손전등이 AI 감지, 벌 행동, 밤낮 gameplay 같은 domain logic에 영향을 줘야 하는 요구가 확인된다.
- 기존 Blueprint asset compile/save 없이는 C++ property 추가가 안정적으로 반영되지 않는 문제가 발생한다.

## 주의사항

- 이번 작업은 손전등 토글 기능만 수행한다.
- DynamicSky, TimeOfDay, Environment 시스템은 변경하지 않는다.
- 기존 입력 property, component property, UCLASS 이름은 rename하지 않는다.
- Content asset은 직접 수정하지 않는다.
- Config는 수정하지 않는다.
