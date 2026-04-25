# 구현 프롬프트: 아이템 손/커서 표현 Actor 전환

## 작업 목표

`UItemDefinition` 의 raw static mesh 기반 held item 표현을 전용 Actor class 기반 표현으로 전환한다.

현재 `WorldMesh` 는 pickup actor 표시와 held item visualizer 표시가 함께 사용하고 있다. 이번 작업에서는 pickup 표시용 mesh 의 기존 동작은 유지하고, 손에 들거나 `EngagedFocus` 중 커서를 따라다니는 표현은 `AItemPresentationActor` 기반으로 분리한다.

## 대상 파일 목록

수정 대상:

- `Source/BeekeepingSim/Public/ItemDefinition.h`
- `Source/BeekeepingSim/Private/ItemDefinition.cpp`
- `Source/BeekeepingSim/Public/ItemInstance.h`
- `Source/BeekeepingSim/Private/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`
- `Source/BeekeepingSim/Public/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldItemPickup.cpp`
- `.md/0_ARCHITECTURE.md`

신규 파일:

- `Source/BeekeepingSim/Public/ItemPresentationActor.h`
- `Source/BeekeepingSim/Private/ItemPresentationActor.cpp`

## 구현 요구사항

1. `AItemPresentationActor` 를 추가한다.
   - `AActor` 기반 시각 표현 전용 actor 로 만든다.
   - gameplay 상태 변경, 획득 처리, hotbar 상태 변경 책임을 갖지 않는다.
   - 기본 root scene component 를 가진다.
   - Blueprint 확장을 위해 `BlueprintType`, `Blueprintable` 로 선언한다.
   - collision 은 기본적으로 비활성화하는 헬퍼를 제공한다.
   - owner only see, shadow, reflection/ray tracing 표시 정책을 적용할 수 있는 초기화 함수를 제공한다.

2. `UItemDefinition` 에 held item 표현용 class 필드를 추가한다.
   - 필드명: `HeldPresentationActorClass`
   - 타입: `TSubclassOf<AItemPresentationActor>`
   - category: `Item|Presentation`
   - 기존 `WorldMesh` 는 이번 작업에서 제거하지 않는다.
   - 기존 `WorldMesh` 는 pickup 표시용 및 fallback 표시용으로 유지한다.

3. `UItemInstance` 에 held item 표현 class 조회 API 를 추가한다.
   - 예: `TSubclassOf<AItemPresentationActor> GetHeldPresentationActorClass() const`
   - `Definition` 이 없으면 null 을 반환한다.

4. `UBeekeeperHeldItemVisualizerComponent` 를 actor spawn 방식으로 변경한다.
   - 기존 `UStaticMeshComponent* HeldItemMeshComponent` 직접 표시 구조를 제거하거나 더 이상 핵심 경로로 사용하지 않는다.
   - 선택 아이템 변경 시 현재 spawned presentation actor 를 정리하고 새 actor 를 spawn 한다.
   - spawn class 는 선택된 `UItemInstance` 의 `HeldPresentationActorClass` 를 우선 사용한다.
   - `HeldPresentationActorClass` 가 없고 `WorldMesh` 가 있으면 fallback actor 를 만들 수 있도록 처리한다.
     - 단순 fallback 이 필요하면 `AItemPresentationActor` 안에 static mesh component 를 optional 로 두거나, visualizer 내부에서 fallback 전용 actor class 없이 최소 표현을 구성한다.
     - fallback 구현이 복잡해질 경우 `WorldMesh` fallback 은 생략하고 class 가 없는 아이템은 숨겨도 된다. 이 경우 로그를 남긴다.
   - spawned actor 는 로컬 플레이어에서만 생성한다.
   - spawned actor 는 `FirstPersonCamera` 에 attach 한다.
   - `EHotbarPresentationMode::InHand` 에서는 기존 `InHandLocalOffset`, `InHandLocalRotation`, `MeshRelativeScale` 에 대응하는 actor relative transform 을 적용한다.
   - `EHotbarPresentationMode::OnCursor` 에서는 기존 커서 정규화 좌표 로직을 유지하고 actor relative transform 을 갱신한다.
   - 선택 해제, 비유효 아이템, presentation class 없음, 비로컬 상태에서는 spawned actor 를 숨기거나 destroy 한다.
   - 비로컬에서는 기존처럼 Tick interval 을 0.25초로 낮춘다.

5. `AWorldItemPickup` 의 기존 동작은 유지한다.
   - pickup actor 는 계속 `UItemDefinition::WorldMesh` 를 사용해 월드에 놓인 아이템 mesh 를 표시한다.
   - pickup 표시를 `AItemPresentationActor` 로 전환하지 않는다.

## 구현 원칙

- `UItemDefinition` 에 runtime actor instance 를 저장하지 않는다.
- DataAsset 에는 actor class 만 저장한다.
- `UBeekeeperHotbarComponent` 의 상태 오너 책임은 변경하지 않는다.
- `UBeekeeperHeldItemVisualizerComponent` 는 시각화 전용 책임만 유지한다.
- `AItemPresentationActor` 는 gameplay 로직을 포함하지 않는다.
- 기존 hotbar 획득, stack 병합, pickup focus action 흐름은 변경하지 않는다.
- 기존 `WorldMesh` 에 의존하는 pickup 동작을 깨지 않는다.
- 기존 `Public` / `Private` 구조를 유지한다.
- Unreal UObject/Actor lifecycle 과 GC 를 고려해 spawned actor 참조는 `UPROPERTY(Transient)` 로 보관한다.
- 불필요한 Tick 은 늘리지 않는다.

## 세부 기능 설명

### `AItemPresentationActor`

- 기본적으로 scene root 를 가진다.
- Blueprint 에서 mesh, skeletal mesh, Niagara, 여러 component 조합을 자유롭게 추가할 수 있어야 한다.
- visualizer 에서 spawn 한 뒤 카메라에 attach 해 상대 transform 을 제어한다.
- visualizer 가 호출할 수 있는 함수 예시:
  - `InitializePresentation(ABeekeeperCharacter* OwningCharacter, UItemInstance* ItemInstance)`
  - `SetPresentationHidden(bool bHidden)`
  - `ApplyFirstPersonVisibilityPolicy()`

### `UItemDefinition`

- 기존 필드:
  - `WorldMesh`: pickup world 표시용으로 유지
- 신규 필드:
  - `HeldPresentationActorClass`: 손/커서 표현용 actor class

### `UBeekeeperHeldItemVisualizerComponent`

- 선택 아이템이 바뀌면 presentation actor class 변경 여부를 확인한다.
- class 가 바뀐 경우 기존 actor 를 destroy 하고 새 actor 를 spawn 한다.
- 같은 item/class 인 경우 transform/visibility 만 갱신한다.
- 로컬 플레이어가 아니면 presentation actor 를 숨기거나 destroy 한다.
- `OnHotbarChanged` 와 focus rule 변경 delegate 구독 구조는 유지한다.

## Unreal 관련 제약 조건

- spawned actor 는 `GetWorld()->SpawnActorDeferred` 또는 `SpawnActor` 를 사용한다.
- owner 는 `ABeekeeperCharacter` 로 설정한다.
- attach 는 `FAttachmentTransformRules::KeepRelativeTransform` 기준으로 처리한다.
- presentation actor collision 은 꺼야 한다.
- local player 전용 표현이므로 network replication 은 사용하지 않는다.
- asset migration 부담을 줄이기 위해 기존 `WorldMesh` 제거/rename 은 이번 작업에서 하지 않는다.

## 문서 반영 요구사항

구현 완료 후 `.md/0_ARCHITECTURE.md` 의 변경된 부분만 갱신한다.

반영할 내용:

- `AItemPresentationActor` 추가
- `UItemDefinition` 의 held 표현 필드가 `HeldPresentationActorClass` 임을 명시
- `WorldMesh` 는 pickup 표시용/fallback 용도로 유지됨을 명시
- `UBeekeeperHeldItemVisualizerComponent` 가 static mesh component 직접 표시가 아니라 presentation actor spawn/attach 방식으로 동작함을 명시

## 출력 요구사항

작업 완료 보고는 아래 형식을 따른다.

```
[상태] 완료
[요약] 변경된 구조와 핵심 동작 요약
[변경 파일] 수정/추가한 파일 목록
[검증] 빌드 또는 정적 확인 결과. 실행하지 못한 경우 이유
[주의] 기존 asset 에서 설정이 필요한 필드 또는 fallback 동작
```
