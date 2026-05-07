# BeekeepingSim Architecture Map

## 문서 기준

- 기준일: 2026-04-29
- 상태: 1차 구조 리팩토링과 보류 리팩토링 완료 후 현재 구조 기준
- 정본 문서: `.md/0_ARCHITECTURE.md`와 `.md/Architecture/*.md`
- legacy 문서: `Source/ARCHITECTURE.md`는 정본이 아니며 이 문서로 연결하는 안내 파일로만 유지한다.

## 분석 범위

- 주 분석 범위:
  - `Source/BeekeepingSim/Public`
  - `Source/BeekeepingSim/Private`
- 현재 대상 C++ 파일 수: 60개
  - Public header: 33개
  - Private cpp/header: 27개
- `Content`는 Blueprint 참조 검증 범위로만 다룬다. C++ 시스템 책임의 정본은 Source 하위 문서에 둔다.
- `Config/DefaultEngine.ini`는 Core Redirect가 필요한 rename 호환 경로로만 문서화한다.

## 시스템 문서

- [CharacterSystem.md](Architecture/CharacterSystem.md): 캐릭터 입력, 컨트롤러, 이동, held item 시각화
- [CameraSystem.md](Architecture/CameraSystem.md): 이동/착지 기반 카메라 셰이크
- [FocusSystem.md](Architecture/FocusSystem.md): PreviewFocus/EngagedFocus, 타겟, 액션, 크로스헤어 정책
- [InteractionSystem.md](Architecture/InteractionSystem.md): pickup/storage 상호작용 액션
- [InventorySystem.md](Architecture/InventorySystem.md): hotbar, storage, item model, stack 이동 계산
- [UISystem.md](Architecture/UISystem.md): slot widget, storage widget, drag/drop payload와 routing
- [WorldActorsSystem.md](Architecture/WorldActorsSystem.md): beehive, pickup, storage box actor 구성
- [EnvironmentSystem.md](Architecture/EnvironmentSystem.md): 24시간 가속 시간, 하늘/조명, 태양/달, 에디터 프리뷰
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

## 시스템 간 책임 흐름

- Character는 로컬 플레이어 입력과 컴포넌트 조립 지점이다.
- Controller는 active storage와 active drag operation 같은 UI 세션 컨텍스트를 보관한다.
- Focus는 현재 타겟, engaged action, prompt, item rule, crosshair visibility의 단일 상태 오너다.
- Interaction은 FocusAction의 구체 구현이며 pickup/storage 같은 도메인별 상호작용을 실행한다.
- Inventory는 hotbar/storage/item instance의 실제 상태 변경과 stack 이동 결과를 소유한다.
- UI는 위젯 상태, drag payload, drop 라우팅, Blueprint 표시 API를 제공한다.
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
- 기존 `ABeeSplineSwarmActor`/`BP_BeeSplineSwarm` 워크플로우는 별도로 유지된다.
- Environment는 24시간 가속 시간과 하늘/조명/태양/달 연출을 단일 시간 값에서 평가한다.
- Environment의 `UGameTimeBucketSubsystem`은 월드 공용 시간 bucket 이벤트를 제공하며, listener interface를 구현한 actor들에 n분 경계 이벤트를 dispatch한다.

## 주요 의존 방향

- Character -> Camera, Focus, Inventory, UI
- Camera -> Character
- Focus -> Character, Camera, Inventory
- Interaction -> Focus, Inventory, UI, WorldActors
- Inventory -> Focus, UI
- UI -> Character, Inventory
- WorldActors -> Focus, Interaction, Inventory
- Environment -> Core

Environment와 WorldActors의 C++ 의존 경계는 유지한다. 월드 actor 시간 반응은 Environment concrete actor 직접 참조 대신 `IGameTimeBucketListener` interface + subsystem dispatch 경로로 연결한다.

의존 방향은 완전한 단방향 레이어가 아니라 Unreal 컴포넌트 조합을 반영한다. 새 기능을 추가할 때는 "상태 오너"와 "표시/입력 라우터"를 먼저 구분한다.

## 현재 Blueprint 계약

Blueprint native parent로 확인된 핵심 C++ 클래스:

- `ABeekeeperCharacter`
- `ABeekeeperController`
- `ABeehive`
- `AStorageBox`
- `AItemPresentationActor`
- `UItemSlotWidget`
- `UItemVisualWidget`
- `UStorageBoxWidget`

현재 유지해야 하는 Blueprint API:

- `UItemSlotWidget::InitializeSlotContext`
- `UItemSlotWidget::ShouldHideItemVisualForCurrentDrag`
- `UItemSlotWidget::IsPartialDragPreviewActive`
- `UItemSlotWidget::GetPartialDragPreviewDisplayStackCount`
- `UStorageBoxWidget::OnStorageWidgetInitialized`

`ShouldHideItemVisualForCurrentDrag`는 legacy wrapper지만 `WBP_ItemSlot` 참조가 남아 있으므로 삭제하지 않는다.

## 완료된 고위험 리팩토링

- `UItemDragVisualWidget` 삭제
- `UFocusTargetComponent::bClearFocusOnConfirm` 제거
- `UFocusTargetComponent::ShouldClearFocusOnConfirm()` 제거
- `UStorageBoxWidget`의 이동/스왑 wrapper API 제거
- `UItemSlotWidget::GetDragPreviewDisplayStackCount()` 제거
- Drag/Drop 타입명 정리:
  - `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
  - `EStorageSlotContainerType` -> `EItemSlotContainerType`
  - `StorageSlotDragDropOperation.*` -> `ItemSlotDragDropOperation.*`
  - `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`
- `Config/DefaultEngine.ini` `[CoreRedirects]`에 class/enum redirect 추가

## 현재 설계 원칙

- C++ 상태 변경은 Inventory/Focus/Interaction 쪽 컴포넌트가 맡고, Widget은 입력과 표시 상태를 라우팅한다.
- Blueprint API는 실제 사용 여부와 에셋 직렬화 참조를 분리해 판단한다.
- UCLASS/USTRUCT/UENUM rename은 Core Redirect, Editor 재시작, Blueprint compile/save, post-migration scan까지 한 세트로 처리한다.
- UI drag/drop은 `UItemSlotDragDropOperation` payload와 `UItemSlotDragDropLibrary` routing으로 통일한다.
- Quick move 대상 선택은 현재 `UItemSlotWidget`에 남아 있다. 규칙이 복잡해지면 Inventory 쪽 서비스/helper로 이동하는 것이 다음 후보다.
## Time Clock Widget Summary

- `UTimeOfDayClockWidget` displays runtime `Hour24` in fixed `HH:MM` format.
- Display minute uses floor conversion and updates only when the displayed minute changes.
- The widget does not search world actors; `ABeekeeperController` resolves `AEnvironmentTimeOfDayActor` and pushes `Hour24`.
## Beehive Attraction Niagara Summary

- `ABeehive` includes `AttractionSwarmNiagara` directly for attraction-style local swarm motion.
- The component position is the attraction center, editable per placed beehive instance.
- Spawn count is computed from `ColonyBeeCount` and attraction settings, not from time buckets.
