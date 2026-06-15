# BeekeepingSim Architecture Map

## 문서 기준

- 기준일: 2026-06-10(KST) Source 재대조 기준
- 상태: Focus prompt multi-entry, placed item remaining, smoker/aggression, active-use durability drain, honey ripeness, uncapping table/capping mask, honey container/decanting table, BeeCarrier/QueenCage 포획 state, 분봉 최종 완료 조건 반영 후 현재 Source 구조 기준
- 정본 문서: `.md/0_ARCHITECTURE.md`와 `.md/Architecture/*.md`
- legacy 문서: `Source/ARCHITECTURE.md`는 정본이 아니며 이 문서로 연결하는 안내 파일로만 유지한다.

## 분석 범위

- 주 분석 범위:
  - `Source/BeekeepingSim/Public`
  - `Source/BeekeepingSim/Private`
- 현재 대상 C++ 파일 수: 192개
  - Public header: 107개
  - Private cpp/header: 85개
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
  - Environment: Public 9 / Private 4
  - Focus: Public 22 / Private 14
  - Interaction: Public 2 / Private 2
  - Inventory: Public 25 / Private 18
  - UI: Public 8 / Private 7
  - WorldActors: Public 35 / Private 34

## 시스템 간 책임 흐름

- Character는 로컬 플레이어 입력과 컴포넌트 조립 지점이다.
- Controller는 active storage, active drag operation, runtime clock widget binding 같은 로컬 UI 세션 컨텍스트를 보관한다.
- Focus는 현재 타겟, engaged action, prompt, item rule, crosshair visibility의 단일 상태 오너다.
- Interaction은 FocusAction의 구체 구현이며 pickup/storage 같은 도메인별 상호작용을 실행한다.
- Inventory는 hotbar/storage/item instance의 실제 상태 변경과 stack 이동 결과를 소유한다.
- Inventory는 꿀 용기 runtime state(`FHoneyContainerItemState`), BeeCarrier 포획 state(`FBeeCarrierItemState`), QueenCage 여왕벌 state(`FQueenCageItemState`)를 `UItemInstance`의 optional state로 보존하며, 정적 용량/default는 각 item definition subclass가 소유한다.
- UI는 위젯 상태, drag payload, drop 라우팅, runtime clock 표시, Blueprint 표시 API를 제공한다.
- `WBP_FocusPrompt` 런타임 바인딩/텍스트/visibility/위치 갱신은 `UFocusPromptWidget`(C++ base widget)이 담당하고, 다중 상호작용 row 생성/수직 정렬/disabled 스타일은 Blueprint 작업 영역으로 유지한다.
- Focus prompt entry 생성은 실제 실행 주체가 append API로 제공한다. 전역 Focus는 `UFocusActionComponent`, PartFocus는 `UCursorPartFocusActionComponent`가 context별 prompt entry와 availability를 제공한다.
- WorldActors는 Focus/Interaction/Inventory 컴포넌트를 조합해 월드 배치 가능한 actor를 만든다.
- WorldActors의 `ABeehiveDualSwarmActor`는 outgoing/ingoing Niagara 2개를 소유하고, `ABeehive`가 전달한 spline reference와 계산된 parameter를 적용한다.
- `ABeehive`는 single dual-swarm child actor와 `SwarmSpline`을 직접 소유하고 `ColonyBeeCount`, common/directional settings, `Hour24`로 spawn/speed/shape 값을 계산해 주입한다.
- `ABeehive`는 외부 Blueprint 테스트 호출용 `BeginSwarmingAtTransform`/`BeginSwarmingAtActor`를 제공하며, 기존 colony/queen/comb state를 변경하지 않고 `ABeeSwarmClusterActor`와 `ABeehiveSwarmRouteActor`만 spawn한다. 분봉 본진 생성 입력은 `SwarmClusterSpawnAmount`와 `SwarmClusterBeeDensityPerCubicMeter`(기본 `8000.0 bees/m^3`)다.
- `ABeeSwarmClusterActor`는 분봉 본진의 포획/잔여 벌 수 source of truth, `SpawnAmount`와 `BeeDensityPerCubicMeter`에서 파생한 `InitialAliveRadius`/`AliveRadius`/`SphereRadius` Niagara parameter, 별도 여왕벌 child actor, FocusEngaged item-use-area host를 소유하며, 최종 완료는 벌 전량 포획과 여왕벌 포획이 모두 끝난 뒤 발생한다.
- `ABeehiveSwarmRouteActor`는 `ABeeSplineSwarmActor` 기반 runtime route를 거리 기반 홀수 자동 중간점 spline으로 구성하고 기존 spline swarm Niagara parameter 계약을 유지한다.
- `ABeehive`는 `CombRackRoot` + `MaxCombCount` 슬롯(`UChildActorComponent`)을 소유하며, BeginPlay에서 `InitialCombCount`만큼 초기 소비장을 채우고 각 슬롯 child actor(`ABeehiveCombSlotActor`)의 placed comb를 active comb로 관리한다.
- `ABeehive`는 `QueenBeeChildActor`를 소유하고 시간 bucket 구독(`QueenBeeLocation`)을 통해 기본 60분마다 여왕벌 위치를 자동 갱신한다. 왕롱 포획으로 `bHasQueenBee=false`가 되면 여왕벌 child 재생성, 위치 갱신, 산란 증가량 계산은 비활성화된다.
- `ABeehive`는 시간 bucket 구독(`ColonyPopulation`)을 통해 기본 60분마다 `ColonyBeeCount`를 자동 갱신한다.
- `ABeehive`는 시간 bucket 구독(`HoneyProduction`)을 통해 기본 60분마다 꿀 생산을 처리한다.
- `ABeehive`는 시간 bucket 구독(`PollenPattyConsumption`)을 통해 화분떡을 고정량(`PollenPattyConsumptionAmountPerBucket`) 소모한다.
- `ABeehive`는 `SanitationValue` 기반 질병값(`GetSanitationDiseaseRatio()`, `0..1`)의 source of truth이며, 위생값 변경 시 자체 `DiseaseVfxNiagara.User.Disease`에 질병 시각값을 즉시 주입한다.
- `ABeehive`의 `ItemEggLayingBonus`는 화분떡 소모 대상 선택 정책과 동일하게 고른 active 화분떡 1개의 `UPollenPattyItemDefinition::EggLayingMultiplier`를 사용한다.
- 화분떡 소모량은 벌 수/온도/bucket 길이와 무관한 고정값이며, bucket 길이는 소모 주기만 바꾼다.
- 소모 대상은 `PollenPattyConsumptionAreaTags`와 slot의 configured `AreaTags`(`AItemPlacementSlotActor::GetSlotAreaTags`) 매칭으로 식별한다.
- 여러 후보가 있으면 벌통 local Y 기준으로 leftmost/rightmost 1개만 소모하고, 같은 bucket에서 spillover 분배는 하지 않는다.
- 여왕벌 위치 후보는 active comb 중 현재 lifted comb slot을 제외하며, 중앙 comb일수록 높은 확률로 선택된다.
- 선택된 comb에서는 front/back attach point를 50:50으로 고르고, attach point 기준으로 `0..360` 랜덤 yaw를 추가 적용한다.
- BeeBrush가 현재 visible face에 부착된 여왕벌을 털어내면 `ABeehive`가 같은 소비장을 제외한 다른 active comb 중 하나를 랜덤으로 골라 front/back attach point에 재부착한다. 다른 소비장이 없거나 여왕벌이 해당 face에 붙어 있지 않으면 여왕벌은 이동하지 않는다.
- `ABeehiveCombActor`는 소비장 전체 기준 `TotalSpawnAmount`/`TotalTargetBeeCount`를 상태로 소유하고, Niagara 주입 시 front/back face 분배값(`Front=(Total+1)/2`, `Back=Total/2`)으로 `User.SpawnAmount`/`User.TargetBeeCount`를 각각 적용한다.
- `AQueenBeeActor`는 Tick마다 `AddActorLocalRotation`으로 `[-YawJitterDegreesPerTick, +YawJitterDegreesPerTick]` yaw를 누적해 떨림을 만든다.
- `AQueenBeeActor`는 `BaseEggLayingPower`를 소유하며 colony population 증가량 계산의 기본 산란력으로 사용된다.
- `AQueenBeeActor`는 `QueenBeeMesh` 하위 `QueenCageUseAreaMesh`로 `Item.UseArea.QueenBee.QueenCage` 영역을 제공하고, 왕롱 포획 state export를 담당한다.
- colony population 계산식 요약:
  - `Increase = QueenBaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * BeeIncreaseCoefficient`
  - `EffectiveThreshold = Clamp(SanitationDiseaseThreshold, 0, MaxSanitationValue)`
  - `DiseaseRatio = 0 if EffectiveThreshold <= 0 or SanitationValue >= EffectiveThreshold, else Clamp((EffectiveThreshold - SanitationValue) / EffectiveThreshold, 0..1)`
  - `SanitationDecreaseMultiplier = Lerp(1.0, MaxSanitationBeeDecreaseMultiplier, DiseaseRatio)`
  - `Decrease = Min(ColonyBeeCount, (((ColonyBeeCount * BeeDecreaseCoefficient) + BeeDecreaseAbsoluteAmountPerBucket) / ItemLifespanBonus / TemperatureScore) * SanitationDecreaseMultiplier)`
  - `ItemEggLayingBonus`는 selected active pollen patty가 `UPollenPattyItemDefinition`이면 `Max(1.0, EggLayingMultiplier)`, 아니면 `1.0`
  - `BeeDecreaseCoefficient`는 현재 벌 수에 비례하는 감소율이고, `BeeDecreaseAbsoluteAmountPerBucket`는 population bucket 1회당 고정 감소량이며 기본값 `0.0f`로 기존 동작을 유지한다.
  - `ItemLifespanBonus`와 `TemperatureScore`는 비례 감소량과 절대 감소량을 더한 전체 감소량에 적용된다.
  - `SanitationDiseaseThreshold=0.0f` 또는 `MaxSanitationBeeDecreaseMultiplier=1.0f`이면 기존 감소 동작을 유지한다.
  - 감소량은 bucket 시작 시점의 기존 `ColonyBeeCount`를 초과하지 않으며, `ItemLifespanBonus`와 `TemperatureScore`의 1차 기본값은 각각 `1.0`
- honey production 계산식 요약:
  - `TotalHoneyIncrease = ColonyBeeCount * HoneyProductionCoefficient`
  - `HoneyProduction` bucket event에서는 꿀 생산 전에 이미 full 상태인 active comb의 숙성도(`CurrentHoneyRipeness`)를 증가시킨다.
  - 같은 60분 bucket 경계에서는 `HoneyProduction`을 `ColonyPopulation`보다 먼저 처리한다.
  - `ABeehiveCombActor`는 절대 꿀 양(`CurrentHoney`)을 저장하고, 시각값은 `Clamp(CurrentHoney/MaxHoneyPerComb, 0..1)` fill ratio를 사용한다.
  - `ABeehiveCombActor`는 절대 숙성도(`CurrentHoneyRipeness`)를 저장하고, honey plane과 wax/capping plane material에는 `Clamp(CurrentHoneyRipeness/MaxHoneyRipeness, 0..1)` ratio를 `HoneyRipeness`로 주입한다.
  - `ABeehiveCombActor`의 full honey 밀랍/capping 표시는 `IsHoneyFull()`과 face별 capping mask 완료 여부에서 파생되며, `FrontWaxCappingPlane`/`BackWaxCappingPlane` material에는 transient `WaxCappingMask` texture가 주입된다.
  - 벌통 honey ripeness/production 업데이트는 active comb mutation 직후 `TryRegenerateWaxCapping()`을 호출하며, full honey와 `WaxCappingRegenerationRipenessThreshold`를 만족한 벌통 내부 소비장만 제거된 face mask를 `255`로 재생성한다.
- FocusEngaged host 내부 파츠 상호작용은 `UCursorPartFocusScopeComponent`가 담당하고, 전역 focus 단일 오너(`UBeekeeperFocusComponent`)와 분리된다.
- 파츠별 동작은 전용 C++ subclass 대신 공통 `UCursorPartFocusActionComponent` + BP Begin/Cancel/Abort 이벤트 구현을 기본 경로로 사용한다.
- Host FocusEngaged 이후 파츠 입력은 `LMB`(begin/cancel)와 `R/F/C`(hover preview key action)로 분리한다.
- Host FocusEngaged 이후 `LMB Completed` 입력은 active host action으로 전달되며, item-use session 종료(`EndUse`)에 사용된다.
- FocusEngaged item-use area는 벌통 전용이 아니라 host actor가 선택적으로 제공하는 generic 기능으로 설계한다.
- Host가 item-use-area scope/provider를 지원하고 선택 아이템이 있으며 선택 action의 `CanBeginUse(Context)`가 true이면 LMB는 item-use action으로 처리한다. 선택 아이템이 없거나 `CanBeginUse(Context)`가 false이면 기존 FocusAction/PartFocus 입력 정책으로 fallback한다.
- Anchored cursor FocusEngaged 진입 시 hotbar 선택은 빈손으로 전환하며, item-use area는 engaged 이후 대상 아이템을 다시 선택했을 때 활성화된다.
- 사용영역 표시/점멸은 LMB와 무관하며, 선택 아이템과 area tag query가 매칭된 active descriptor 기준으로 처리한다.
- `ABeehive`와 `AUncappingTable`은 `UCursorItemUseAreaScopeComponent` + `UItemUseAreaMeshProviderComponent`를 통해 item-use-area host를 구현한다.
- `AUncappingTable`은 native WorldActor이며 anchored cursor FocusEngaged 상태에서 단일 `AUncappingTableCombSlot` child actor를 통해 소비장 배치/회수/뒤집기/밀도질 use-area를 제공한다.
- `ABeeSwarmClusterActor`는 generic `UCursorItemUseAreaScopeComponent` + `UItemUseAreaMeshProviderComponent` 경로로 `Item.UseArea.SwarmCluster.BeeCarrier` 영역과 child 여왕벌의 `Item.UseArea.QueenBee.QueenCage` 영역을 제공한다.
- 작업대 empty slot 배치는 `UItemPlacementUseAction` + item-use-area LMB 경로를 사용하고, occupied comb 회수는 `UCombUncappingPartFocusActionComponent`의 PartFocus secondary retrieve bridge가 hotbar state-aware acquire를 사용한다.
- `UCombUncappingUseAction`은 `Item.UseArea.UncappingTable.HoneyComb` use-area에서 LMB hold-use 중 현재 visible face의 capping mask를 원형 brush로 제거한다. 실제 mask pixel이 제거된 tick에만 `bSucceeded=true`를 반환하며 내구도 감소와 채밀/수확은 구현하지 않는다.
- 작업대 소비장 PartFocus `잡기/놓기` active 상태는 `AUncappingTableCombSlot`이 별도 bool로 소유하며, 잡기 상태에서는 occupied comb capping use-area와 `UCombUncappingUseAction` begin/apply가 비활성화된다.
- `AUncappingTableCombSlot`은 작업대 소비장 잡기/놓기/강제 종료 시점에 `ReceiveCombGrabbed`, `ReceiveCombReleased`, `ReceiveCombGrabAborted` Blueprint 이벤트를 제공하며, 실제 들어올림/원위치 애니메이션은 slot Blueprint가 `AttachComponent` relative transform으로 구현한다.
- 꿀 용기 배치 actor는 `AHoneyContainerActor`이며, current volume/density/ripeness runtime state, honey visual material/scale 갱신, source 배출용 `HoneyStreamNiagara`, nozzle/retrieve PartFocus action component를 소유한다.
- 꿀 용기 슬롯은 `AHoneyContainerSlotActor`이며, reusable `Source`/`Target` role과 accepted gameplay tag query로 배치 가능 item을 판정한다.
- 꿀 이송 진행 상태와 규칙은 `UHoneyTransferComponent`가 소유한다. DropLength grow가 목표 길이에 도달한 뒤 volume transfer가 시작되며, target density/ripeness는 volume-weighted average로 혼합된다. 목표 길이는 source container `HoneyStreamNiagara`와 target container/slot `PourTarget`의 world Z 차이로 계산한다.
- `AHoneyDecantingTable`은 native FocusEngaged 작업대 host이며 source/target slot child actor와 `UHoneyTransferComponent`를 조립하고 이송 계산은 구현하지 않는다.
- `FItemActionContext`는 item-use-area cursor hit fields(`bHasItemUseAreaHit`, `ItemUseAreaImpactPoint`, `ItemUseAreaImpactNormal`)를 포함하며, `UCursorItemUseAreaScopeComponent`가 hovered descriptor의 trace hit를 context로 전달한다.
- `UBeeCarrierUseAction`은 `Item.UseArea.SwarmCluster.BeeCarrier` use-area에서 LMB hold 중 context impact point 이동 속도로 bees/sec 포획량을 계산하고, 실제 포획량을 BeeCarrier item instance state에 누적한다.
- `UQueenCageUseAction`은 `Item.UseArea.QueenBee.QueenCage` use-area에서 `AQueenBeeActor`를 즉시 1회 포획하고, 결과를 `FQueenCageItemState`로 왕롱 item instance에 저장한다.
- 기존 `ABeeSplineSwarmActor`/`BP_BeeSplineSwarm` 워크플로우는 별도로 유지된다.
- Environment는 `AGameTimeOfDayActor`의 24시간 가속 시간과 `ADynamicSky`의 하늘/조명/태양/달 연출을 `ITimeOfDayProvider` 계약으로 분리한다.
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

- `FocusSecondaryAction` 입력 경로를 추가했고, 현재 의미는 FocusEngaged host 내부 PartFocus secondary 입력이다.
  - `ABeekeeperCharacter::FocusSecondaryAction` 입력(`Started`) -> `FocusSecondaryInput()` -> `UBeekeeperFocusComponent::HandleSecondaryInput()`
  - non-engaged preview 상태에서 `HandleSecondaryInput()`은 false를 반환한다.
- WorldActors에 generic placed item 회수 흐름을 추가했고, 현재 실행 경로는 host 내부 PartFocus secondary retrieve다.
  - `AItemPlacementSlotActor`가 spawn한 `APlacedItemActor`를 `InitializePlacedItem(SourceItemInstance, this)`로 초기화한다.
  - PartFocus hovered secondary input 시 retrieve action이 state-aware hotbar acquire(`TryAcquireItemBySpec`)를 수행한다.
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

## Update 2026-05-27 (ItemUseAreaMesh Provider)

- FocusEngaged item-use-area 등록 source를 actor/provider override에서 `UItemUseAreaMeshProviderComponent`로 통합했다.
- descriptor의 hit/visual/effect-target은 `UItemUseAreaMeshComponent`가 소유한다.
- component active 판단은 `IItemUseAreaActivationProvider`가 담당하며 inactive인 경우 descriptor는 유지하고 `AreaTags`를 비운다.
- child actor가 `IItemUseAreaMeshSource`를 구현하면 `UItemUseAreaMeshProviderComponent`가 추가 제공 mesh도 descriptor로 등록한다.

## Update 2026-05-28 (Generic Placement Occupant + Beehive Comb Slot)

- generic placement 점유/회수 계약을 component 기반으로 확장했다.
  - `UPlacementOccupantComponent`: 반환 item definition(runtime + authored fallback), owning slot, 회수 가능 판정 hook, clear 전 hook
  - `UPlacementSlotRetrievePartFocusActionComponent`: PartFocus secondary 입력에서 hotbar `TryAcquireItemBySpec(...)` + slot clear
- `AItemPlacementSlotActor`를 generic occupied actor 모델로 확장했다.
  - `InitialOccupantActor` preplaced claim (BeginPlay), attach/snap 옵션, occupied actor descriptor 공급
  - occupied actor의 `UItemUseAreaMeshComponent`를 host item-use-area provider에 노출
  - clear 시 occupant `PreClearPlacementOccupant` hook 호출 후 destroy
- `APlacedItemActor`는 새 generic component들로 migration했고 기존 getter/초기화 API는 wrapper로 유지한다.
- 벌통 소비장 구조를 slot 기반으로 전환했다.
  - `ABeehiveCombSlotActor`가 comb 전용 slot 역할을 수행한다.
  - `ABeehive`는 comb child actor를 직접 관리하지 않고, comb slot child actor의 placed comb를 active comb로 본다.
  - `InitialCombCount`는 에디터 authoring 값이며, `CurrentCombCount`는 slot occupancy에서 갱신되는 내부 캐시다.
  - honey/colony/queen 후보 계산은 placed comb 기준으로 동작한다.
- 소비장 회수 상태 계약:
  - 회수 가능 조건: `TotalTargetBeeCount == 0` && queen 미부착
  - 회수 시 `UItemInstance`에 `BeehiveCombState(꿀양, visible face)`를 기록해 상태를 보존한다.

## Update 2026-05-31 (Placed Item Durability Remaining)

- 배치 아이템 잔량은 신규 `RemainingAmount/MaxAmount` 필드 없이 durability를 재사용한다.
  - max 잔량 source: `UItemDefinition::MaxDurability`
  - inventory 잔량 source: `UItemInstance::Durability`
  - 배치 중 런타임 잔량 owner: `UPlacedItemRemainingComponent`
- `APlacedItemActor`는 기본 subobject로 `UPlacedItemRemainingComponent`를 항상 소유한다.
  - `bUseDurabilityAsPlacedRemaining=false`이거나 invalid config면 remaining은 비활성/무상태로 둔다.
- 회수 경로는 `FItemAcquireSpec` 기반 state-aware acquire를 사용한다.
  - durability stack 병합은 `FMath::IsNearlyEqual(A, B, 0.0001f)` 기준의 동일 durability끼리만 허용한다.
  - 회수 성공 후 반환 `LastModifiedItemInstance`에 placed remaining durability를 write-back한다.
- 화분떡 전용 actor 경로는 제거했다.
  - `APollenPattyActor`를 제거하고 공용 `APlacedItemActor` 경로를 사용한다.
  - Blueprint 호환을 위해 `DefaultEngine.ini` `[CoreRedirects]`에 `PollenPattyActor -> PlacedItemActor` class redirect를 추가했다.

## Update 2026-05-31 (Pollen Patty Fixed Consumption)

- `ABeehive`에 `PollenPattyConsumption` bucket 구독을 추가했다.
  - 기본값: `PollenPattyConsumptionBucketMinutes=60`, `bApplyPollenPattyConsumptionOnBeginPlayBucket=false`
- 소모량은 `PollenPattyConsumptionAmountPerBucket` 고정값만 사용한다.
  - bucket 길이, catch-up 횟수, 벌 수/온도/생산량 계산으로 소모량을 스케일하지 않는다.
- 소모 대상 탐색은 provider descriptor 대신 direct child `AItemPlacementSlotActor` 수집 경로를 사용한다.
  - 매칭 기준: slot configured tags가 `PollenPattyConsumptionAreaTags`를 모두 포함
  - 후보 조건: occupied actor에 active `UPlacedItemRemainingComponent`가 있고 `CurrentAmount > 0`
- 후보가 여러 개면 벌통 local Y 기준 `Leftmost/Rightmost` 1개를 선택해 `ConsumeAmount(...)`를 호출한다.

## Update 2026-05-31 (Pollen Patty Population Bonus)

- 화분떡 인구 가속효과는 colony population 증가 항(`ItemEggLayingBonus`)에만 적용한다.
- bonus 수치는 `UPollenPattyItemDefinition::EggLayingMultiplier`에서 읽는다.
- 대상 선택은 기존 `PollenPattyConsumptionSide` 기반 leftmost/rightmost active 화분떡 1개 정책을 재사용한다.
- 여러 active 화분떡이 있어도 bonus는 중첩하지 않으며, selected 1개의 multiplier만 사용한다.

## Update 2026-05-31 (Focus Prompt C++ Base Widget)

- `UFocusPromptWidget`를 UI 시스템에 추가했다.
- `WBP_FocusPrompt`는 layout/style 전용 Blueprint로 유지하고, prompt data binding/표시 갱신은 C++ base widget이 처리한다.
- `BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름은 유지한다.

## Update 2026-06-01 (Focus Prompt Anchor Mode + Position Policy)

- `FFocusPromptData`는 `EFocusPromptAnchorMode`(`ScreenCenter`, `MouseCursor`)를 포함한다.
- 일반 Focus prompt(`UFocusTargetComponent::GetPromptData`)는 기본 `ScreenCenter` anchor mode를 사용한다.
- PartFocus prompt(`UCursorPartFocusScopeComponent` engaged override)는 `MouseCursor` anchor mode로 변환되며, 표시 위치 갱신 책임은 `UFocusPromptWidget`에 있다.

## Update 2026-06-01 (Focus Prompt Multi Entry + Availability)

- `FFocusPromptData`는 다중 `FFocusPromptEntry` 배열을 포함하는 방향으로 확장한다.
- `FFocusPromptEntry`는 key/action 표시값, `bEnabled`, `DisabledReason`, `SortPriority`, `EntryId`를 포함하는 공통 상호작용 row DTO다.
- disabled entry는 pickup/회수 전용이 아니라 모든 표시 가능한 상호작용에 적용되는 공통 availability 상태다.
- `UBeekeeperFocusComponent`는 preview target owner의 `UFocusActionComponent`들이 제공하는 `AppendFocusPromptEntries(...)` 결과를 수집한다.
- `UCursorPartFocusScopeComponent`는 hovered part action의 `AppendPartFocusPromptEntries(...)` 결과를 수집해 engaged prompt override로 전달한다.
- 두 append API는 context 격리를 위해 분리하지만, UI로 내려가는 데이터 모델은 `FFocusPromptEntry`로 통일한다.
- action 이름은 실행 주체 action component가 소유한다. 전역 Focus는 `PromptActionText`/`EngagedPromptActionText` + `ResolveFocusPromptActionText()`, PartFocus primary는 `PrimaryPromptActionText`/`EngagedPrimaryPromptActionText` + `ResolvePrimaryPromptActionText()`를 Blueprint authoring 값 source로 사용한다.
- 공통 PartFocus primary action text는 action engaged 상태에 따라 시작/해제 텍스트를 전환한다. 예: `열기`/`닫기`, `들기`/`넣기`.
- `UFocusPromptWidget`은 entries를 Blueprint event로 전달하고, `WBP_FocusPrompt`가 수직 row 생성과 disabled alpha 스타일을 담당한다.
- hotbar 획득/회수 availability는 상태 변경 없는 `PreviewAcquireItemBySpec` dry-run query로 판정한다.

## Update 2026-06-01 (Smoker + Beehive Aggression)

- `ABeehive`는 sanitation과 분리된 aggression 상태(`MaxAggressionValue=100`, `AggressionValue=100` 기본)를 소유한다.
- `USmokerUseAction`(`UHoldItemUseAction` 기반)은 `Item.UseArea.Beehive.Smoker` 영역에서 hold-use 중 `ABeehive::DecreaseAggression`을 호출한다.
- 현재 범위에서 aggression 자동 회복과 공격력 계산/피해 시스템 연동은 구현하지 않는다.

## Update 2026-06-02 (Active-Use Durability Drain)

- FocusEngaged item-use-area hold-use 경로에 active-use durability drain 계약을 추가했다.
- `UItemDefinition` base class는 확장하지 않고 `UActiveUseDurabilityItemDefinition` subclass를 추가했다.
- `UActiveUseDurabilityItemDefinition::DrainPolicy`가 drain 조건을 결정한다: `WhenUseEffectSucceeded`, `WhileOverValidUseArea`, `WhileUseSessionActive`.
- 대상 아이템 invariant:
  - `bUsesDurability=true`
  - `MaxDurability>0`
  - `MaxStack==1`
- `FItemActionExecutionResult`는 `DurabilityDelta`를 포함하며 stack delta와 독립적으로 해석된다.
- 실제 selected item durability mutation authority는 `UBeekeeperHotbarComponent::ApplySelectedItemDurabilityDelta(...)`다.
- `UCursorItemUseAreaScopeComponent`는 `ApplyUseEffect` 결과와 `ResolveActiveUseDurabilityDelta(...)`를 합산해 durability를 적용하며, `WhileUseSessionActive` 정책은 hovered use-area 없이도 active use session 중 drain된다.
- durability 0 도달 시 현재 use session을 `EndUseSession(false)`로 종료한다.
- 훈연기/소독약은 DataAsset을 `UActiveUseDurabilityItemDefinition`으로 전환한 경우에만 drain이 적용된다.
- 벌솔(`UBeeBrushUseAction`)은 기존처럼 durability drain 대상이 아니다.

## Update 2026-06-02 (Honey Ripeness)

- 꿀 숙성도 상태 owner는 `ABeehiveCombActor`다.
  - `CurrentHoneyRipeness`: `0..MaxHoneyRipeness` 절대값
  - material parameter `HoneyRipeness`: `CurrentHoneyRipeness / MaxHoneyRipeness` normalized value
- `ABeehive`는 기존 `HoneyProduction` bucket subscription만 사용한다.
- `HoneyProduction` bucket event 처리 순서:
  - `ApplyHoneyRipenessUpdate()`
  - `ApplyHoneyProductionUpdate()`
- `ApplyHoneyProductionUpdate()` 직접 호출은 숙성 없이 꿀 생산만 수행한다.
- 숙성 대상은 벌통이 관리하는 active comb 전체이며 lifted comb도 포함한다. empty slot은 제외한다.
- 이번 bucket에서 처음 full이 된 comb는 같은 bucket에서 숙성되지 않고 다음 `HoneyProduction` bucket부터 숙성 대상이 된다.
- 소비장 회수/재배치 state(`FBeehiveCombItemState`)는 `HoneyAmount`, `HoneyRipeness`, visible face와 face별 capping mask byte buffer를 보존한다.

## Update 2026-06-03 (Sanitation Disease)

- `ABeehive`의 `SanitationValue`가 sanitation disease의 source of truth다.
- `GetSanitationDiseaseRatio()`는 `SanitationDiseaseThreshold`와 `MaxSanitationValue`로 `0..1` disease ratio를 계산한다.
- `GetSanitationBeeDecreaseMultiplier()`는 disease ratio를 colony population 감소 항에 적용할 배율로 변환한다.
- `SetSanitationValue()`는 `RefreshHiveDiseaseVisuals()`를 통해 `ABeehive::DiseaseVfxNiagara.User.Disease`에 `GetSanitationDiseaseRatio()` 값을 즉시 주입한다.
- attraction/outgoing/ingoing swarm, active comb front/back Niagara, queen material의 직접 `Disease` 적용 경로는 legacy path로 남기되 C++ 적용 코드는 주석처리되어 있다.
- disease 전용 Tick/subsystem/bucket subscription은 추가하지 않는다.

## Update 2026-06-08 (Uncapping Table + Comb Wax Capping Mask)

- 소비장 full honey 밀랍/capping 표시는 `ABeehiveCombActor`의 native 앞/뒤 plane component와 face별 byte mask로 처리한다.
- capping mask source of truth는 `FBeehiveCombItemState`와 `ABeehiveCombActor`의 `FrontWaxCappingMask`/`BackWaxCappingMask` `TArray<uint8>`다. `255`는 밀랍 남음, `0`은 제거됨이다.
- mask dimension은 `PlaneSize` X/Y 비율과 `CappingMaskLongSideResolution`에서 산출하며, 저장 mask dimension이 현재 actor dimension과 맞지 않으면 full mask로 fallback한다.
- runtime visual은 transient `UTexture2D`를 생성/갱신해 capping material parameter `WaxCappingMask`에 주입한다. `HoneyRipeness` scalar 주입은 유지한다.
- capping plane 표시 조건은 face별 `IsHoneyFull() && !IsWaxCappingFaceComplete(Face)`다. full honey가 아니어도 mask는 reset하지 않는다.
- `AUncappingTable`/`AUncappingTableCombSlot`/`UCombUncappingPartFocusActionComponent`가 작업대 FocusEngaged, 단일 소비장 slot, horizontal drag flip, secondary retrieve를 제공한다.
- `UCombUncappingUseAction`은 `Item.UseArea.UncappingTable.HoneyComb` tag를 all-tags-match query로 사용하고, context hit point 기반 brush stamp를 rate limit(`MinStampInterval`, `MinStampDistanceCm`)한다.
- 새 GameplayTag: `Item.UseArea.UncappingTable`, `Item.UseArea.UncappingTable.HoneyComb`.
- 이번 범위에서 채밀/수확, 꿀 아이템 생산, 밀도 도구 active-use durability drain은 구현하지 않는다.

## Update 2026-06-09 (Wax Capping Regeneration)

- `ABeehiveCombActor::TryRegenerateWaxCapping()`은 `EnsureCappingMaskState()` 후 full honey와 숙성 threshold(`WaxCappingRegenerationRipenessThreshold`, 기본 `1.0`)를 만족할 때 face별 mask 중 `255`가 아닌 값이 있는 face 전체를 `255`로 복원한다.
- 자동 재생성 호출은 `ABeehive::ApplyHoneyRipenessUpdate()`와 `ABeehive::DistributeHoneyIncreaseToCombs()`에만 있다.
- `SetCurrentHoney()`, `SetCurrentHoneyRipeness()`, `ApplyStateFromItemInstance()`는 capping mask를 자동 재생성하지 않으므로 작업대 배치/저장복원 직후에는 제거 상태가 유지된다.

## Update 2026-06-10 (Honey Container + Decanting Table)

- `FHoneyContainerItemState`를 `UItemInstance` optional state로 추가했다. `bHasState`, `CurrentVolumeMl`, `HoneyDensity`, `HoneyRipeness`를 저장하며, 비-꿀 용기 item은 해당 state를 무시한다.
- `UHoneyContainerItemDefinition`은 꿀 용기 정적 용량/default source of truth다. 꿀 용기 item은 `MaxStack=1` invariant를 가진다.
- hotbar/storage partial move로 새 `UItemInstance`를 만들 때 `CopyRuntimeStateFrom`으로 runtime state를 보존한다.
- `AHoneyContainerActor`, `AHoneyContainerSlotActor`, `UHoneyTransferComponent`, `UHoneyNozzlePartFocusActionComponent`, `UHoneyContainerRetrievePartFocusActionComponent`, `AHoneyDecantingTable`을 추가했다.
- material parameter 정본은 `HoneyDensity`, `HoneyRipeness`이고 Niagara parameter 정본은 `User.HoneyDensity`, `User.HoneyRipeness`, `User.DropLength`다.
- 꿀 줄기 Niagara component는 source `AHoneyContainerActor::HoneyStreamNiagara`가 소유한다. `UHoneyTransferComponent`는 active source stream을 제어하고, target length는 `Max(0, SourceStream.Z - TargetPourTarget.Z)`로 계산한다.
- `UCursorPartFocusActionComponent` primary lifecycle 함수는 C++ subclass가 instant primary action을 구현할 수 있도록 virtual로 열렸다.

## Update 2026-06-13 (Swarming Test Capture)

- 외부 Blueprint가 `ABeehive::BeginSwarmingAtTransform` 또는 `BeginSwarmingAtActor`를 호출해 분봉 테스트를 수동 시작한다.
- 시작 시 `ABeehive`는 `SwarmExitPoint` world location/rotation으로 `ABeehiveSwarmRouteActor`를 spawn하고, 분봉 본진 center까지 거리 기반 자동 중간점 route spline을 구성한 뒤 `SwarmRouteParameters`를 `ApplyExternalSwarmParameters`로 주입한다.
- `ABeehive`는 `SwarmClusterSpawnAmount`와 `SwarmClusterBeeDensityPerCubicMeter`(기본 `8000.0 bees/m^3`)로 분봉 본진을 시작한다. `ABeeSwarmClusterActor`의 `InitialAliveRadius`, `AliveRadius`, `SphereRadius`는 `RadiusCm = cbrt((3 * (SpawnAmount / BeeDensityPerCubicMeter)) / (4 * PI)) * 100`으로 파생되는 runtime 값이다.
- `ABeeSwarmClusterActor`는 `SpawnAmount`, `CapturedBeeAmount`, `InitialAliveRadius`를 기준으로 `AliveRadius = InitialAliveRadius * cbrt(RemainingBeeAmount / TotalBeeAmount)`를 Niagara user parameter `User.AliveRadius`에 적용한다.
- 분봉 본진 여왕벌은 기존 벌통 `QueenBeeChildActor`를 이동하지 않고 `SwarmQueenBeeActorClass` child actor로 별도 생성해 `ClusterCenter + QueenCenterOffset`에 두며, `InitializeSwarmClusterFromDensity()` 시점에 pitch/yaw/roll 세 축을 1회 랜덤 회전한다.
- `UBeeCarrierUseAction`은 `Item.UseArea.SwarmCluster.BeeCarrier` hit context의 impact point 이동 속도로 bees/sec bonus rate를 계산하고, 본진 잔여 벌 수와 BeeCarrier 남은 수용량으로 clamp한 실제 포획량을 `FBeeCarrierItemState`에 저장한다.
- `CapturedBeeAmount >= SpawnAmount` 또는 잔여 벌 수가 0이면 `bBeesCaptured`로 전환하고 BeeCarrier capture use-area와 `AliveRadius`만 완료 처리한다.
- `ABeehive`와 `ABeeSwarmClusterActor`는 `IQueenBeeCaptureSource`를 구현해 왕롱 포획 가능 여부와 host 상태 변경을 담당한다.
- 왕롱 포획 결과는 `FQueenCageItemState`(`bHasQueen`, `CapturedQueenBeeClass`, `BaseEggLayingPower`, `DiseaseValue`)로 저장하며 world actor reference는 저장하지 않는다.
- 분봉 본진의 최종 `bCaptured`/`ReceiveSwarmCaptured`는 `bBeesCaptured && bQueenCaptured`일 때 1회만 발생한다.
- 이번 분봉 테스트 시작 경로는 `ColonyBeeCount`, 기존 벌통 여왕벌 위치, active comb bee count/target count, bucket subscription을 변경하지 않는다.
- 왕롱으로 벌통 여왕벌을 포획해도 `ColonyBeeCount`, active comb bee count/target count, honey state는 즉시 변경하지 않는다.
- 새 GameplayTag: `Item.UseArea.SwarmCluster.BeeCarrier`, `Item.BeeCarrier`, `Item.UseArea.QueenBee.QueenCage`, `Item.QueenCage`.
