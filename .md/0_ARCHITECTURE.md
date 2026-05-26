# BeekeepingSim Architecture Map

## 문서 기준

- 기준일: 2026-05-09(KST) Source 재대조 기준
- 상태: 1차 구조 리팩토링과 보류 리팩토링 완료 후 현재 Source 구조 기준
- 정본 문서: `.md/0_ARCHITECTURE.md`와 `.md/Architecture/*.md`
- legacy 문서: `Source/ARCHITECTURE.md`는 정본이 아니며 이 문서로 연결하는 안내 파일로만 유지한다.

## 분석 범위

- 주 분석 범위:
  - `Source/BeekeepingSim/Public`
  - `Source/BeekeepingSim/Private`
- 현재 대상 C++ 파일 수: 100개
  - Public header: 55개
  - Private cpp/header: 45개
- `Content`는 Blueprint 참조 검증 범위로만 다룬다. C++ 시스템 책임의 정본은 Source 하위 문서에 둔다.
- `Config/DefaultEngine.ini`는 Core Redirect가 필요한 rename 호환 경로로만 문서화한다.

## 시스템 문서

- [CharacterSystem.md](Architecture/CharacterSystem.md): 캐릭터 입력, 컨트롤러, 이동, held item 시각화, clock UI binding
- [CameraSystem.md](Architecture/CameraSystem.md): 이동/착지 기반 카메라 셰이크
- [FocusSystem.md](Architecture/FocusSystem.md): PreviewFocus/EngagedFocus, 타겟, 액션, 크로스헤어 정책
- [InteractionSystem.md](Architecture/InteractionSystem.md): pickup/storage 상호작용 액션
- [InventorySystem.md](Architecture/InventorySystem.md): hotbar, storage, item model, stack 이동 계산
- [UISystem.md](Architecture/UISystem.md): slot widget, storage widget, time clock widget, drag/drop payload와 routing
- [WorldActorsSystem.md](Architecture/WorldActorsSystem.md): beehive, pickup, storage box actor 구성
- [EnvironmentSystem.md](Architecture/EnvironmentSystem.md): 24시간 가속 시간, 하늘/조명, 태양/달, 에디터 프리뷰, gameplay time bucket
- [CoreSystem.md](Architecture/CoreSystem.md): 공통 문서 규칙, Core Redirect, 시스템 경계

## Source 구조

```text
Source/BeekeepingSim/
  Public/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
    Environment/
  Private/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
    Environment/
```

- `Core`는 소스 폴더가 아니라 문서상 공통 경계다.
- 시스템 하위 폴더명은 include 경로의 1차 네임스페이스 역할을 한다.
- 현재 파일 분포:
  - Camera: Public 1 / Private 1
  - Character: Public 5 / Private 5
  - Environment: Public 5 / Private 2
  - Focus: Public 13 / Private 8
  - Interaction: Public 2 / Private 2
  - Inventory: Public 11 / Private 9
  - UI: Public 7 / Private 6
  - WorldActors: Public 11 / Private 12

## 시스템 간 책임 흐름

- Character는 로컬 플레이어 입력과 컴포넌트 조립 지점이다.
- Controller는 active storage, active drag operation, runtime clock widget binding 같은 로컬 UI 세션 컨텍스트를 보관한다.
- Focus는 현재 타겟, engaged action, prompt, item rule, crosshair visibility의 단일 상태 오너다.
- Interaction은 FocusAction의 구체 구현이며 pickup/storage 같은 도메인별 상호작용을 실행한다.
- Inventory는 hotbar/storage/item instance의 실제 상태 변경과 stack 이동 결과를 소유한다.
- UI는 위젯 상태, drag payload, drop 라우팅, runtime clock 표시, Blueprint 표시 API를 제공한다.
- WorldActors는 Focus/Interaction/Inventory 컴포넌트를 조합해 월드 배치 가능한 actor를 만든다.
- WorldActors의 `ABeehiveDualSwarmActor`는 outgoing/ingoing Niagara 2개를 소유하고, `ABeehive`가 전달한 spline reference와 계산된 parameter를 적용한다.
- `ABeehive`는 single dual-swarm child actor와 `SwarmSpline`을 직접 소유하고 `ColonyBeeCount`, common/directional settings, `Hour24`로 spawn/speed/shape 값을 계산해 주입한다.
- `ABeehive`는 `CombRackRoot` + `MaxCombCount` 슬롯(`UChildActorComponent`)을 소유하며, 활성 슬롯(`CurrentCombCount`)에만 `ABeehiveCombActor`를 생성한다.
- `ABeehive`는 `QueenBeeChildActor`를 소유하고 시간 bucket 구독(`QueenBeeLocation`)을 통해 기본 60분마다 여왕벌 위치를 자동 갱신한다.
- `ABeehive`는 시간 bucket 구독(`ColonyPopulation`)을 통해 기본 60분마다 `ColonyBeeCount`를 자동 갱신한다.
- `ABeehive`는 시간 bucket 구독(`HoneyProduction`)을 통해 기본 60분마다 꿀 생산을 처리한다.
- 여왕벌 위치 후보는 active comb 중 현재 lifted comb slot을 제외하며, 중앙 comb일수록 높은 확률로 선택된다.
- 선택된 comb에서는 front/back attach point를 50:50으로 고르고, attach point 기준으로 `0..360` 랜덤 yaw를 추가 적용한다.
- `ABeehiveCombActor`는 `FrontFaceBeeNiagara`/`BackFaceBeeNiagara`에 `User.PlaneSize`, `User.SpawnAmount`, `User.TargetBeeCount`를 동일 값으로 적용한다.
- `AQueenBeeActor`는 Tick마다 `AddActorLocalRotation`으로 `[-YawJitterDegreesPerTick, +YawJitterDegreesPerTick]` yaw를 누적해 떨림을 만든다.
- `AQueenBeeActor`는 `BaseEggLayingPower`를 소유하며 colony population 증가량 계산의 기본 산란력으로 사용된다.
- colony population 계산식 요약:
  - `Increase = QueenBaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * BeeIncreaseCoefficient`
  - `Decrease = ColonyBeeCount * BeeDecreaseCoefficient / ItemLifespanBonus / TemperatureScore`
  - 1차 구현 기본값: `ItemEggLayingBonus=1.0`, `ItemLifespanBonus=1.0`, `TemperatureScore=1.0`
- honey production 계산식 요약:
  - `TotalHoneyIncrease = ColonyBeeCount * HoneyProductionCoefficient`
  - 같은 60분 bucket 경계에서는 `HoneyProduction`을 `ColonyPopulation`보다 먼저 처리한다.
  - `ABeehiveCombActor`는 절대 꿀 양(`CurrentHoney`)을 저장하고, 시각값은 `Clamp(CurrentHoney/MaxHoneyPerComb, 0..1)` fill ratio를 사용한다.
- FocusEngaged host 내부 파츠 상호작용은 `UCursorPartFocusScopeComponent`가 담당하고, 전역 focus 단일 오너(`UBeekeeperFocusComponent`)와 분리된다.
- 파츠별 동작은 전용 C++ subclass 대신 공통 `UCursorPartFocusActionComponent` + BP Begin/Cancel/Abort 이벤트 구현을 기본 경로로 사용한다.
- Host FocusEngaged 이후 파츠 입력은 `LMB`(begin/cancel)와 `R/F/C`(hover preview key action)로 분리한다.
- Host FocusEngaged 이후 `LMB Completed` 입력은 active host action으로 전달되며, item-use session 종료(`EndUse`)에 사용된다.
- FocusEngaged item-use area는 벌통 전용이 아니라 host actor가 선택적으로 제공하는 generic 기능으로 설계한다.
- Host가 item-use-area scope/provider를 지원하고 선택 아이템이 있으면 LMB는 item-use action으로 처리하고, host가 지원하지 않거나 선택 아이템이 없으면 기존 FocusAction/PartFocus 입력 정책을 따른다.
- Anchored cursor FocusEngaged 진입 시 hotbar 선택은 빈손으로 전환하며, item-use area는 engaged 이후 대상 아이템을 다시 선택했을 때 활성화된다.
- 사용영역 표시/점멸은 LMB와 무관하며, 선택 아이템과 area tag query가 매칭된 active descriptor 기준으로 처리한다.
- `ABeehive`는 `UCursorItemUseAreaScopeComponent` + `IItemUseAreaProvider`를 통해 item-use-area first host를 구현한다.
- 기존 `ABeeSplineSwarmActor`/`BP_BeeSplineSwarm` 워크플로우는 별도로 유지된다.
- Environment는 24시간 가속 시간과 하늘/조명/태양/달 연출을 단일 시간 값에서 평가한다.
- Environment의 `UGameTimeBucketSubsystem`은 월드 공용 시간 bucket 이벤트를 제공하며, listener interface를 구현한 actor들에 n분 경계 이벤트를 dispatch한다.
- Runtime clock UI는 bucket system을 사용하지 않는다. `ABeekeeperController`가 `ITimeOfDayProvider`를 resolve하고, `AGameTimeOfDayActor`를 우선 구독해 `UTimeOfDayClockWidget`에 `Hour24`를 주입한다. (`AEnvironmentTimeOfDayActor`는 compatibility fallback)

## 주요 의존 방향

- Character -> Camera, Focus, Inventory, UI, Environment
- Camera -> Character
- Focus -> Character, Camera, Inventory
- Interaction -> Focus, Inventory, UI, WorldActors
- Inventory -> Focus, UI
- UI -> Character, Inventory
- WorldActors -> Focus, Interaction, Inventory, Environment
- Environment -> Core

WorldActors의 Environment 의존은 concrete actor 직접 참조/polling이 아니라 `IGameTimeBucketListener` interface + `UGameTimeBucketSubsystem` dispatch 경로로 제한한다. local runtime clock UI binding은 `ABeekeeperController`의 provider resolve(`ITimeOfDayProvider`, `AGameTimeOfDayActor` 우선) 경로를 사용한다.

의존 방향은 완전한 단방향 레이어가 아니라 Unreal 컴포넌트 조합을 반영한다. 새 기능을 추가할 때는 "상태 오너"와 "표시/입력 라우터"를 먼저 구분한다.

## Blueprint/API 변경 주의

- Blueprint native parent, BlueprintCallable API, serialized `UPROPERTY`/component 이름은 Content asset 참조와 분리해 판단하지 않는다.
- `UCLASS`/`USTRUCT`/`UENUM`/`UFUNCTION`/`UPROPERTY` rename 또는 삭제는 Core Redirect 필요 여부, Editor 재시작, Blueprint compile/save, post-migration scan까지 함께 계획한다.
- 현재 Blueprint/API 계약의 상세 목록은 관련 시스템 문서의 `Blueprint/API Contracts` 또는 `Design Notes`를 우선 확인한다.
- Core Redirect와 cross-system migration 규칙은 [CoreSystem.md](Architecture/CoreSystem.md)를 따른다.

## 현재 설계 원칙

- 상태 변경의 owner와 표시/입력 router를 먼저 구분한다.
- C++ domain state mutation은 각 시스템의 컴포넌트/actor가 맡고, Widget은 입력 의도와 표시 상태를 라우팅한다.
- UI 편의 흐름을 제외한 gameplay actor 간 시간 반응은 직접 polling보다 interface/subsystem event 경로를 우선 사용한다.
- 새 시스템, 새 의존 방향, Blueprint/API 계약 변경은 이 문서와 관련 `.md/Architecture/*System.md`를 함께 갱신한다.

## Update 2026-05-24

- Environment time ownership is split:
  - `AGameTimeOfDayActor` owns runtime `Hour24` progression and `OnGameTimeOfDayChanged(float)`.
  - `ADynamicSky` consumes `ITimeOfDayProvider` and applies sky visuals only.
- `ITimeOfDayProvider` is the shared time-source contract for runtime consumers.
- `UGameTimeBucketSubsystem` now supports provider binding via `SetTimeOfDayProvider(AActor*)` and keeps `SetTimeOfDayActor(...)` as compatibility wrapper.
- `ABeekeeperController` clock binding resolves `ITimeOfDayProvider` first (prefers `AGameTimeOfDayActor` delegate path).
- `AEnvironmentTimeOfDayActor` remains for compatibility/transition and now also exposes `OnGameTimeOfDayChanged(float)`.

## Update 2026-05-27

- Focus preview target secondary input 경로를 추가했다.
  - `ABeekeeperCharacter::FocusSecondaryAction` 입력(`Started`) -> `FocusSecondaryInput()` -> `UBeekeeperFocusComponent::HandleSecondaryInput()`
  - Focus는 preview target owner의 `UFocusSecondaryActionComponent` 실행만 위임한다.
- WorldActors에 generic placed item 회수 흐름을 추가했다.
  - `AItemPlacementSlotActor`가 spawn한 `APlacedItemActor`를 `InitializePlacedItem(SourceItemInstance, this)`로 초기화한다.
  - hover + secondary input 시 `UPlacedItemRetrieveFocusActionComponent`가 `TryAcquireItem(ItemDefinition, 1)` 수행
  - `bSuccess && AddedQuantity == 1`일 때만 성공
  - 성공 시 `IItemPlacementSlot::Execute_ClearPlacedItem`로 slot 점유를 해제한다.

## Update 2026-05-27 (PartFocus Provider)

- placed item 회수 경로를 global preview focus가 아니라 FocusEngaged host 내부 PartFocus로 전환했다.
- 새 흐름:
  - `FocusSecondaryAction` 입력 -> `UBeekeeperFocusComponent::HandleSecondaryInput()`
  - engaged action(`UAnchoredFocusCursorActionComponent`) -> `UCursorPartFocusScopeComponent::HandleSecondaryInput()`
  - hovered part action의 `HandleSecondaryPartFocusAction(...)` 실행
- PartFocus descriptor 공급/등록:
  - 공급: `ICursorPartFocusProvider`
  - host 등록: `UCursorPartFocusRegistrationComponent`
  - child 수집: `UChildCursorPartFocusProviderComponent`
- `APlacedItemActor`는 global `UFocusTargetComponent` 대상이 아니며 host 내부 PartFocus part로만 취급한다.
