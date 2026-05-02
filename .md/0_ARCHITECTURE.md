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
