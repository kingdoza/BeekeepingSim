# Beehive Comb Drag Flip/Shake 구현 프롬프트

## 목표

FocusEngaged 상태에서 들어 올린 소비장(`ABeehiveCombActor`)을 LMB drag로 조작한다.

- 좌우 drag: 소비장 180도 뒤집기
- 상하 반복 drag: 소비장 털기

click과 drag는 같은 LMB 입력을 공유하므로, 기존 Focus LMB gesture model을 유지하면서 소비장 전용 drag 해석만 추가한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

특히 `.md/QNA_ARCHITECTURE.md`의 `Beehive Comb Drag QnA` 섹션을 구현 기준으로 삼는다.

## 구현 전 확인

먼저 현재 Source에 Focus LMB gesture/drag lifecycle이 구현되어 있는지 확인한다.

확인 대상:

- `UBeekeepingSimFocusSettings::ClickCancelThresholdPixels`
- `UCursorPartFocusScopeComponent::HandlePartFocusPointerPressed`
- `UCursorPartFocusScopeComponent::HandlePartFocusPointerReleased`
- `UCursorPartFocusActionComponent`의 drag lifecycle API
  - `CanBeginPartFocusDrag`
  - `BeginPartFocusDrag`
  - `UpdatePartFocusDrag`
  - `EndPartFocusDrag`
  - `IsPartFocusDragInProgress`

위 기반이 없으면 이번 작업 범위 안에서 generic Focus drag lifecycle을 먼저 구현해도 된다. 단, 기존 BlueprintCallable API 삭제/rename 없이 추가 방식으로 구현한다.

## 구현 범위

주 수정 대상:

- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

필요 시 수정:

- `Source/BeekeepingSim/Public/Focus/BeekeepingSimFocusSettings.h`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

문서 갱신 대상:

- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/0_ARCHITECTURE.md`
- 필요 시 `.md/QNA_IMPLEMENTATION.md`

수정하지 말 것:

- `Content/` asset 직접 수정 또는 resave
- `Config/DefaultEngine.ini`
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable API 삭제 또는 rename
- Inventory/UI/Environment 도메인 로직
- Beehive lift movement의 slot transform 정책

## 확정 요구사항

### Drag 가능 조건

- 소비장 drag는 해당 소비장 part action이 engaged되어 있고, 그 소비장이 lifted 상태일 때만 허용한다.
- lid open + hover만으로는 drag를 시작하지 않는다.
- drag 불가능 상태에서 click cancel threshold를 넘으면 click만 취소되고 no-op이다.

### Flip

- 좌우 drag는 소비장 180도 뒤집기 동작이다.
- flip 판정:
  - 누적 X 이동량이 `CombFlipDragThresholdPixels` 이상
  - `Abs(X) > Abs(Y) * HorizontalDominanceRatio`
- `HorizontalDominanceRatio`는 대각선/애매한 drag가 flip으로 오인되지 않게 하는 방향 우세 조건이다.
- 한 drag session에서 mode가 flip으로 확정되면 release까지 다른 mode로 전환하지 않는다.

### Shake

- 상하 반복 drag는 소비장 털기 동작이다.
- shake 판정:
  - Y 이동이 `CombShakeStrokeThresholdPixels` 이상 누적된 뒤 방향이 반전될 때 stroke count 증가
  - `RequiredShakeStrokeCount`에 도달하면 털기 실행
- 1차 구현 효과는 `ABeehiveCombActor::ReduceTargetBeeCountByRatio`만 호출해 소비장 표면 벌 수를 줄인다.
- 꿀, 위생, 아이템 수확, Inventory 변경은 이번 범위에서 구현하지 않는다.

### No-op 허용

- click cancel threshold를 넘었지만 flip/shake threshold 또는 dominance 조건을 만족하지 못하면 아무 동작도 실행하지 않는다.
- release 시점 fallback으로 flip/shake를 억지 실행하지 않는다.
- mode가 확정되지 않은 drag session은 release 시 no-op이다.

### Animation/Visual

- C++은 상태 변경과 Blueprint event만 제공한다.
- 실제 flip/shake 애니메이션은 Blueprint에서 처리할 수 있게 이벤트를 노출한다.
- C++ Tick 기반 flip/shake 보간은 이번 범위에서 구현하지 않는다.

## Focus Generic Drag Lifecycle

이미 구현되어 있으면 기존 구조를 사용한다. 없으면 아래 방향으로 추가한다.

`UCursorPartFocusActionComponent`에 drag lifecycle API를 추가한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Drag")
virtual bool CanBeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const;

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Drag")
virtual bool BeginPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Drag")
virtual void UpdatePartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, float DeltaTime);

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Drag")
virtual bool EndPartFocusDrag(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter, bool bCanceled);

UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Drag")
bool IsPartFocusDragInProgress() const;
```

기본 구현:

- `CanBeginPartFocusDrag()`는 false.
- `BeginPartFocusDrag()`는 false.
- `UpdatePartFocusDrag()`는 no-op.
- `EndPartFocusDrag()`는 false.

`UCursorPartFocusScopeComponent`는 press 이후 click cancel threshold 초과 시 press action의 drag 가능 여부를 확인하고 drag lifecycle을 호출한다.

추가로 drag action이 현재 마우스 delta를 해석할 수 있어야 한다. 가장 단순한 계약은 scope가 drag 중 screen delta를 제공하는 것이다.

권장 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Drag")
FVector2D GetPartFocusDragDeltaFromPress() const;

UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Drag")
FVector2D GetPartFocusDragDeltaSinceLastUpdate() const;
```

Scope 책임:

- press screen position 저장
- 현재 screen position 추적
- press 기준 누적 delta 제공
- 직전 update 기준 delta 제공
- drag update 전에 delta 값을 갱신

## BeehiveCombActor 변경

`ABeehiveCombActor`에 소비장 flip/shake 상태와 Blueprint event를 추가한다.

### Component 구조

QnA 기준으로 `CombPivotRoot`를 추가한다.

권장 구조:

```text
Root
  CombPivotRoot
    CombMesh
      QueenFrontAttachPoint
      QueenBackAttachPoint
      FrontHoneyPlane
      BackHoneyPlane
    FrontFaceBeeNiagara
    BackFaceBeeNiagara
```

주의:

- 기존 `Root`와 actor/slot transform은 `UBeehiveCombLiftComponent`가 간접적으로 사용하므로 유지한다.
- slot `UChildActorComponent` relative transform을 flip 용도로 회전하지 않는다.
- `CombPivotRoot`는 소비장 내부 visual pivot이다.
- 기존 component 이름은 가능한 유지한다. 새 component 추가는 허용한다.
- 기존 child Blueprint에서 component hierarchy 변경 영향이 있을 수 있으므로 최종 보고에 Blueprint compile/save 필요성을 적는다.

### 상태/API

권장 enum:

```cpp
UENUM(BlueprintType)
enum class EBeehiveCombVisibleFace : uint8
{
	Front,
	Back
};
```

UENUM 추가가 부담되면 `bool bIsBackFaceVisible`로 구현해도 된다. 새 enum 추가는 rename이 아니므로 Core Redirect는 필요하지 않다.

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void FlipCombFace();

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void SetVisibleCombFace(EBeehiveCombVisibleFace NewFace);

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
EBeehiveCombVisibleFace GetVisibleCombFace() const;

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ApplyCombShakeByRatio(float ReductionRatio);
```

Blueprint events:

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb")
void ReceiveCombFlipped(EBeehiveCombVisibleFace NewVisibleFace);

UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb")
void ReceiveCombShaken(int32 StrokeCount, float ReductionRatio);
```

동작:

- `FlipCombFace()`는 visible face 상태를 토글한다.
- `SetVisibleCombFace()`는 visible face 상태만 저장한다.
- C++은 `CombPivotRoot` yaw/rotation을 즉시 적용하지 않는다. 실제 flip 회전 연출은 Blueprint event에서 구현한다.
- `ApplyCombShakeByRatio()`는 `ReduceTargetBeeCountByRatio(ReductionRatio)`를 호출하고 `ReceiveCombShaken(...)` 이벤트를 발생시킨다.

front/back 의미:

- 기존 `FrontFaceBeeNiagara`, `BackFaceBeeNiagara`, `QueenFrontAttachPoint`, `QueenBackAttachPoint` 데이터 이름은 유지한다.
- flip은 visible face 상태를 바꾼다.
- 데이터 자체를 swap하지 않는다.
- 필요하면 `GetVisibleCombFace()`를 통해 현재 보이는 face를 해석한다.

## BeehiveCombPartFocusActionComponent 변경

`UBeehiveCombPartFocusActionComponent`를 소비장 drag 해석 owner로 확장한다.

### Tuning property

`UBeehiveCombPartFocusActionComponent` Details에서 조정 가능하게 둔다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
float CombFlipDragThresholdPixels = 120.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
float HorizontalDominanceRatio = 1.5f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
float CombShakeStrokeThresholdPixels = 60.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "1"))
int32 RequiredShakeStrokeCount = 3;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float ShakeBeeReductionRatio = 0.25f;
```

필요 시 vertical dominance도 추가한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Comb Drag", meta = (ClampMin = "0.0"))
float VerticalDominanceRatio = 1.25f;
```

### Drag mode

권장 private enum:

```cpp
enum class EBeehiveCombDragMode : uint8
{
	None,
	Flip,
	Shake
};
```

private state:

```cpp
EBeehiveCombDragMode ActiveDragMode = EBeehiveCombDragMode::None;
float AccumulatedShakeDistanceInCurrentDirection = 0.0f;
float LastShakeDirectionSign = 0.0f;
int32 ShakeStrokeCount = 0;
bool bFlipExecutedThisDrag = false;
bool bShakeExecutedThisDrag = false;
```

### Drag lifecycle

`CanBeginPartFocusDrag`:

- owner가 `ABeehiveCombActor`인지 확인한다.
- action이 이미 engaged인지 확인한다.
- 소비장이 lifted 상태인지 확인한다.
  - 현재 action engaged가 lifted 상태의 기준이면 `IsPartActionEngaged()`로 충분하다.
  - `ABeehive` membership/lift 상태 확인이 필요하면 관련 API가 있는지 먼저 검색한다.
  - 명확한 lifted 상태 API가 없고 action engaged만으로 부족하면 구현을 중단하고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

`BeginPartFocusDrag`:

- drag state 초기화
- base drag in progress 상태 설정
- true 반환

`UpdatePartFocusDrag`:

- scope에서 press 기준 누적 delta와 frame delta를 가져온다.
- mode가 `None`이면 flip 또는 shake mode 확정 조건을 검사한다.
- flip 조건이 먼저 만족되면 mode를 `Flip`으로 lock하고 `ABeehiveCombActor::FlipCombFace()`를 1회 호출한다.
- shake 조건이 먼저 만족되면 mode를 `Shake`로 lock한다.
- shake mode에서는 Y 방향 stroke count를 갱신한다.
- `RequiredShakeStrokeCount`에 도달하면 `ABeehiveCombActor::ApplyCombShakeByRatio(ShakeBeeReductionRatio)`를 1회 호출한다.
- 한 drag session에서 flip과 shake를 동시에 실행하지 않는다.
- mode가 끝까지 확정되지 않으면 no-op이다.

`EndPartFocusDrag`:

- drag state 초기화
- click 실행 금지
- true 반환

## ABeehive / descriptor 등록 주의

현재 `ABeehiveCombActor`는 `PartFocusAction`을 기본 `UCursorPartFocusActionComponent`로 생성하고 있을 수 있다.

구현 방식 선택:

- 권장: `ABeehiveCombActor`의 `PartFocusAction` default subobject class를 `UBeehiveCombPartFocusActionComponent`로 변경한다. subobject 이름 `PartFocusAction`과 UPROPERTY 이름은 유지한다.
- 대안: `ABeehive`가 active comb actor descriptor 등록 시 `UBeehiveCombPartFocusActionComponent`가 있는 경우 우선 사용하고, 없으면 기존 generic action을 유지한다.

주의:

- UPROPERTY 이름, component 이름, UCLASS 이름을 rename하지 않는다.
- Blueprint native parent에서 default subobject class 변경 영향이 있으면 최종 보고에 Blueprint compile/save 필요성을 명시한다.
- class 변경이 기존 Blueprint serialized component와 충돌하면 구현을 중단하고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## Blueprint/API 영향

- 기존 UFUNCTION/UPROPERTY 삭제/rename 금지.
- 새 UFUNCTION/UPROPERTY/UENUM 추가는 허용.
- UCLASS rename 없음.
- Core Redirect는 필요하지 않아야 한다.
- `Content/` asset 직접 수정 금지.
- BP 애니메이션 이벤트를 추가한 경우 Editor에서 Blueprint 구현/compile/save가 필요하다.

## 문서 반영

구현 후 갱신:

- `.md/Architecture/FocusSystem.md`
  - PartFocus drag lifecycle API
  - scope가 drag delta를 제공하는 계약
  - no-op 허용 정책
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehiveCombActor`의 `CombPivotRoot`
  - visible face state
  - comb flip/shake API
  - `UBeehiveCombPartFocusActionComponent`의 drag gesture 해석 책임
  - shake 효과가 `TargetBeeCount` 감소만 수행하는 1차 구현임을 기록

필요 시 갱신:

- `.md/0_ARCHITECTURE.md`
  - 전체 시스템 책임 흐름이나 Source 파일 수 기준이 바뀌는 경우
- `.md/QNA_IMPLEMENTATION.md`
  - lifted 상태 판정, component class 변경 충돌, asset 축 문제 등 구현 중 애매한 점이 생긴 경우

## 검증 기준

### 빌드

가능하면 아래 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

### 코드 검색 검증

- 기존 `UCursorPartFocusActionComponent` begin/cancel/abort API가 삭제되지 않았는지 확인한다.
- `UBeehiveCombPartFocusActionComponent`가 drag lifecycle을 override하는지 확인한다.
- `ABeehiveCombActor`에 `CombPivotRoot` 또는 동등한 pivot root가 추가되었는지 확인한다.
- `ABeehiveCombActor`가 visible face state와 flip/shake API를 제공하는지 확인한다.
- shake 성공 시 `ReduceTargetBeeCountByRatio` 경로를 사용하는지 확인한다.
- flip/shake 둘 다 확정되지 않은 drag release에서 fallback 실행이 없는지 확인한다.

### PIE 수동 검증

1. 벌통 FocusEngaged 진입 후 lid open 상태에서 소비장을 들어 올린다.
2. 들어 올린 소비장 위에서 좌우 drag를 명확히 수행하면 소비장이 180도 뒤집힌다.
3. 대각선/애매한 drag는 flip/shake를 실행하지 않는다.
4. 들어 올린 소비장 위에서 상하 drag를 `RequiredShakeStrokeCount`만큼 반복하면 소비장 털기가 실행된다.
5. 털기 성공 시 해당 소비장의 `TargetBeeCount`가 감소하고 Niagara target bee count가 갱신된다.
6. 한 drag session에서 flip과 shake가 동시에 실행되지 않는다.
7. drag 불가능 상태에서는 threshold 초과 시 click만 취소되고 no-op이다.
8. 기존 PartFocus click begin/cancel, lid open/close, comb lift/restore 동작이 유지된다.
9. item-use-area hold-use가 선택 아이템 상태에서 기존처럼 동작한다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 소비장이 lifted 상태인지 판정할 신뢰 가능한 C++ 기준이 없는 경우
- `ABeehiveCombActor` default subobject class를 `UBeehiveCombPartFocusActionComponent`로 바꾸는 과정에서 Blueprint serialized component 충돌 가능성이 확인되는 경우
- C++에서 즉시 yaw/rotation 적용이 다시 필요하다는 요구가 확인되는 경우
- `CombPivotRoot` 추가가 기존 Blueprint component hierarchy나 serialized override를 깨뜨릴 가능성이 큰 경우
- shake 효과가 벌 수 감소 외에 꿀/위생/수확/Inventory 변경을 요구하는 것으로 확인되는 경우
- drag lifecycle API를 BlueprintCallable virtual로 둘지 BlueprintImplementableEvent로 둘지 기존 BP 사용과 충돌하는 경우
- Public API 삭제/rename 없이는 구현이 불가능한 경우
- Content asset compile/save 없이는 안정적으로 반영되지 않는 문제가 발생하는 경우

## 주의사항

- 이번 작업은 소비장 flip/shake drag 조작만 수행한다.
- 실제 화분떡/소독약 item-use effect 구현은 범위 밖이다.
- 소비장 lift movement의 slot transform 계산은 변경하지 않는다.
- `ABeehiveCombLiftComponent`의 들기/내리기 보간 책임을 침범하지 않는다.
- `ABeehive`의 queen 위치 bucket, colony population, honey production 로직을 변경하지 않는다.
- Config/Core Redirect 작업은 하지 않는다.

## 추가 구현 프롬프트: flip 방향 전달

### 전제

위 `Beehive Comb Drag Flip/Shake 구현 프롬프트` 내용은 이미 현재 Source에 구현되어 있다고 가정한다.

이번 추가 작업은 기존 소비장 flip/shake 구현을 유지한 상태에서, 좌우 drag 방향을 flip API와 Blueprint animation event까지 전달하도록 확장한다.

### 목표

소비장 flip은 여전히 front/back visible face를 toggle한다.

추가로, flip을 유발한 drag 방향을 함께 전달해 Blueprint animation이 입력 방향과 일치하는 회전 연출을 선택할 수 있게 한다.

### 확정 요구사항

- flip 방향은 screen-space drag X 방향을 따른다.
  - 누적 X가 양수이면 right drag 방향 flip
  - 누적 X가 음수이면 left drag 방향 flip
- 최종 visible face는 기존처럼 front/back toggle이다.
- front/back 데이터 swap은 하지 않는다.
- shake gesture에는 flip direction을 적용하지 않는다.
- 기존 `FlipCombFace()` 호출 경로가 있으면 호환 wrapper로 유지한다.
- 기존 `ReceiveCombFlipped(NewVisibleFace)` Blueprint event가 이미 노출되어 있으면 삭제/rename하지 않는다. 새 event 또는 overload 호환 경로를 추가한다.

### 구현 범위

주 수정 대상:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`

필요 시 문서 갱신:

- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/QNA_IMPLEMENTATION.md`

### 권장 API

`ABeehiveCombActor`에 flip direction enum을 추가한다.

```cpp
UENUM(BlueprintType)
enum class EBeehiveCombFlipDirection : uint8
{
	Left,
	Right
};
```

`ABeehiveCombActor` flip API를 확장한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void FlipCombFaceWithDirection(EBeehiveCombFlipDirection FlipDirection);
```

기존 API가 이미 있다면 유지한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void FlipCombFace();
```

권장 동작:

- `FlipCombFace()`는 `FlipCombFaceWithDirection(EBeehiveCombFlipDirection::Right)` 또는 기존 기본 방향을 호출하는 wrapper로 둔다.
- `FlipCombFaceWithDirection(...)`은 visible face를 toggle하고, 방향 포함 Blueprint event를 호출한다.

Blueprint event는 기존 event 삭제 없이 새 이름으로 추가한다.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb")
void ReceiveCombFlippedWithDirection(EBeehiveCombVisibleFace NewVisibleFace, EBeehiveCombFlipDirection FlipDirection);
```

기존 event가 있다면 유지한다.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Beehive|Comb")
void ReceiveCombFlipped(EBeehiveCombVisibleFace NewVisibleFace);
```

권장 호출 순서:

1. visible face 상태 toggle/apply
2. 기존 `ReceiveCombFlipped(NewVisibleFace)` 호출
3. 새 `ReceiveCombFlippedWithDirection(NewVisibleFace, FlipDirection)` 호출

이렇게 하면 기존 Blueprint 구현을 깨지 않고, 새 Blueprint에서는 방향 포함 event를 사용할 수 있다.

### `UBeehiveCombPartFocusActionComponent` 변경

flip mode 확정 시 누적 drag X 부호로 direction을 결정한다.

```cpp
const EBeehiveCombFlipDirection FlipDirection =
	DragDeltaFromPress.X >= 0.0f
		? EBeehiveCombFlipDirection::Right
		: EBeehiveCombFlipDirection::Left;
```

이후 기존 `ABeehiveCombActor::FlipCombFace()` 호출을 direction 포함 API로 교체한다.

```cpp
CombActor->FlipCombFaceWithDirection(FlipDirection);
```

주의:

- flip 판정 조건 자체는 바꾸지 않는다.
- `CombFlipDragThresholdPixels`, `HorizontalDominanceRatio`, no-op 정책은 유지한다.
- 한 drag session에서 flip은 여전히 최대 1회만 실행한다.

### 검증 기준

코드 검색:

- `EBeehiveCombFlipDirection`이 추가되어야 한다.
- 기존 `FlipCombFace()`가 삭제되지 않아야 한다.
- 방향 포함 API 또는 event가 추가되어야 한다.
- `UBeehiveCombPartFocusActionComponent`가 누적 drag X 부호로 left/right를 결정해야 한다.

PIE/Blueprint 수동 검증:

1. 들어 올린 소비장에서 오른쪽 drag로 flip하면 `ReceiveCombFlippedWithDirection(..., Right)`가 호출된다.
2. 왼쪽 drag로 flip하면 `ReceiveCombFlippedWithDirection(..., Left)`가 호출된다.
3. visible face toggle 결과는 기존 flip 구현과 동일하다.
4. 기존 `ReceiveCombFlipped(NewVisibleFace)` 기반 Blueprint가 깨지지 않는다.
5. shake 동작과 no-op 정책은 기존과 동일하다.

### QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 구현에서 `ReceiveCombFlipped(NewVisibleFace)`가 이미 Blueprint에서 핵심 애니메이션을 수행 중이고, 새 event 동시 호출이 중복 애니메이션을 유발할 가능성이 큰 경우
- 기존 `FlipCombFace()` 시그니처를 변경하지 않고는 방향 전달이 어렵다고 판단되는 경우
- drag delta 기준이 screen-space가 아닌 world/local-space로 이미 구현되어 있어 방향 해석 기준이 불명확한 경우
