### [ItemUseAreaMeshComponent 통합 설계 QnA]

확정 방향:

- 실질 ItemUseArea의 hit/visual/material 설정은 `UItemUseAreaMeshComponent`가 담당한다.
- host actor는 직접 descriptor를 수동 생성하지 않는다.
- host actor에 부착된 담당 component가 host 및 child actor 내부의 `UItemUseAreaMeshComponent`를 수집해 `FItemUseAreaDescriptor`로 등록한다.
- child actor가 `UItemUseAreaMeshComponent`를 가진 경우, 상위 host의 담당 component가 해당 child actor 내부 component를 수집한다.
- 소비장 BeeBrush use area는 이 generic child actor 수집 구조의 한 사례다.
- 기존 actor-level `GetItemUseAreaDescriptors` override/BP override 방식은 제거 대상이다.

1. 기존 `IItemUseAreaProvider` 계열 완전 제거 여부
- 질문 내용
  - 새 구조에서는 `UItemUseAreaMeshComponent`와 host 담당 component가 descriptor 생성을 전담한다.
  - 기존 `UItemUseAreaProvider`/`IItemUseAreaProvider` interface를 완전히 제거할지, transition compatibility로 유지할지 결정한다.
- 필요한 이유
  - 완전 제거하면 등록 경로가 단일화되어 BP override/parent call 문제가 사라진다.
  - 다만 `ABeehive`, `AItemPlacementSlotActor`, `UChildItemUseAreaProviderComponent`, `UCursorItemUseAreaScopeComponent`의 Public API 변경 폭이 커진다.
- 선택지
  - 옵션 A: `IItemUseAreaProvider` 경로를 완전히 제거하고 `UItemUseAreaMeshProviderComponent` 단일 경로로 대체한다.
  - 옵션 B: `IItemUseAreaProvider`는 deprecated로 유지하되 scope에서는 새 provider를 우선 사용한다.
  - 옵션 C: 기존 provider interface를 유지하고 새 mesh provider도 `IItemUseAreaProvider`로만 노출한다.
- 권장 옵션:
  - 옵션 A]
- 답변 : 옵션A

2. host 담당 component 이름과 책임
- 질문 내용
  - host actor에 부착되어 `UItemUseAreaMeshComponent`를 수집하고 descriptor를 등록할 담당 component의 이름/책임을 결정한다.
- 필요한 이유
  - 기존 `UChildItemUseAreaProviderComponent`는 child actor의 interface provider를 위임하는 역할이라 새 방향과 책임이 다르다.
  - 새 component는 host 자체 component와 child actor 내부 component를 모두 수집해야 한다.
- 선택지
  - 옵션 A: 새 `UItemUseAreaMeshProviderComponent`를 추가하고, host/child mesh 수집과 descriptor 생성을 전담한다.
  - 옵션 B: 기존 `UChildItemUseAreaProviderComponent`를 확장해 mesh component 수집까지 맡긴다.
  - 옵션 C: `UCursorItemUseAreaScopeComponent`가 직접 host/child component를 순회한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

3. child actor 수집 깊이
- 질문 내용
  - host 담당 component가 child actor 내부 `UItemUseAreaMeshComponent`를 어디까지 순회할지 결정한다.
- 필요한 이유
  - direct child actor만 수집하면 비용과 책임이 명확하다.
  - recursive 수집은 편하지만 예상하지 못한 nested actor의 use area까지 등록될 수 있다.
- 선택지
  - 옵션 A: host의 직접 `UChildActorComponent` child actor까지만 수집한다.
  - 옵션 B: child actor 내부의 child actor까지 recursive로 수집한다.
  - 옵션 C: 기본은 direct only, component 옵션으로 recursive를 허용한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

4. child actor 필터 정책
- 질문 내용
  - host 담당 component가 어떤 child actor를 수집 대상으로 볼지 결정한다.
- 필요한 이유
  - 벌통에는 소비장 외에도 swarm/queen/slot 등 여러 child actor가 있다.
  - 모든 child actor를 스캔해도 비용은 크지 않지만, 의도하지 않은 use-area component가 섞일 수 있다.
- 선택지
  - 옵션 A: `RequiredChildActorComponentTag`와 `RequiredChildActorClass` 필터를 제공하고, 둘 다 비어 있으면 모든 child actor를 허용한다.
  - 옵션 B: `UItemUseAreaMeshComponent`가 있는 child actor는 무조건 수집한다.
  - 옵션 C: class filter만 제공하고 component tag filter는 제거한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A 변형, `RequiredChildActorComponentTag`만 필터로 제공하고, 필터 태그가 비어 있으면 모든 child actor를 허용한다.

5. inactive use area 등록 정책
- 질문 내용
  - `UItemUseAreaMeshComponent`가 현재 active 조건을 만족하지 않을 때 descriptor를 등록할지, 완전히 제외할지 결정한다.
- 필요한 이유
  - descriptor를 제외하면 scope가 해당 mesh의 material parameter/collision을 관리할 수 없어 기본 material 상태가 그대로 보일 수 있다.
  - descriptor를 등록하되 `AreaTags`를 비우면 scope가 opacity 0/hover 0 상태를 적용하면서도 item query 매칭은 막을 수 있다.
- 선택지
  - 옵션 A: inactive component도 descriptor로 등록하되 effective `AreaTags`를 비운다.
  - 옵션 B: inactive component는 descriptor를 반환하지 않는다.
  - 옵션 C: descriptor는 등록하고 `FItemUseAreaDescriptor`에 `bActiveConditionPassed` 같은 별도 필드를 추가한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

6. active 조건 확장 방식
- 질문 내용
  - 소비장 BeeBrush처럼 "자신이 lifted 됐을 때만 active" 같은 조건을 generic 구조에서 어떻게 확장할지 결정한다.
- 필요한 이유
  - `UItemUseAreaMeshComponent`가 벌통/소비장 전용 상태를 직접 알면 generic component 경계가 깨진다.
  - 하지만 child actor별로 active 조건을 확장할 수 있어야 한다.
- 선택지
  - 옵션 A: child actor가 `IItemUseAreaActivationProvider`를 구현해 `IsItemUseAreaMeshActive(Component, HostActor)`를 반환한다.
  - 옵션 B: `UItemUseAreaMeshComponent`에 `BlueprintNativeEvent IsItemUseAreaActiveForHost(HostActor)`를 둔다.
  - 옵션 C: host 담당 component가 class별로 active 조건을 직접 분기한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

7. EffectTargetObject 정책
- 질문 내용
  - `UItemUseAreaMeshComponent`에서 생성된 descriptor의 `EffectTargetObject`를 무엇으로 둘지 결정한다.
- 필요한 이유
  - BeeBrush는 소비장 actor를 target으로 받아야 하고, placement slot은 slot actor를 target으로 받아야 한다.
  - host actor로 고정하면 child actor use area 효과 target 역추적이 필요해진다.
- 선택지
  - 옵션 A: `UItemUseAreaMeshComponent`에 `EffectTargetPolicy`를 둔다. 기본값은 `ComponentOwner`, 선택지로 `HostActor`, `ExplicitObject`를 제공한다.
  - 옵션 B: 모든 descriptor의 `EffectTargetObject`는 component owner actor로 고정한다.
  - 옵션 C: 모든 descriptor의 `EffectTargetObject`는 host actor로 고정한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

8. `UItemUseAreaMeshComponent` 기본 enabled 정책
- 질문 내용
  - `UItemUseAreaMeshComponent::bItemUseAreaEnabled` 기본값을 true로 둘지 false로 둘지 결정한다.
- 필요한 이유
  - component를 붙인다는 것 자체가 등록 후보라는 의미라면 기본 true가 자연스럽다.
  - false가 기본이면 BP에서 설정 누락으로 use area가 조용히 등록되지 않을 가능성이 크다.
- 선택지
  - 옵션 A: `bItemUseAreaEnabled = true`를 기본값으로 둔다.
  - 옵션 B: `bItemUseAreaEnabled = false`를 기본값으로 둔다.
  - 옵션 C: enabled property를 두지 않고 AreaTags 유무만 등록 후보로 본다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

9. 기존 `AItemPlacementSlotActor` migration 범위
- 질문 내용
  - generic placement slot actor의 item-use-area 등록도 새 `UItemUseAreaMeshComponent` 방식으로 즉시 전환할지 결정한다.
- 필요한 이유
  - "기존의 ItemUseArea 모든 등록방식을 완전 대체"하려면 `AItemPlacementSlotActor::GetItemUseAreaDescriptors_Implementation`도 제거해야 한다.
  - 다만 slot occupied 상태에 따라 active/inactive를 제어하는 조건 provider가 필요하다.
- 선택지
  - 옵션 A: `AItemPlacementSlotActor`도 `UItemUseAreaMeshComponent`를 사용하고, actor가 `IItemUseAreaActivationProvider`로 occupied 조건을 제공한다.
  - 옵션 B: BeeBrush/소비장만 먼저 전환하고 placement slot은 후속 작업으로 둔다.
  - 옵션 C: placement slot은 별도 특수 actor provider 경로를 유지한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

10. Blueprint/API migration 및 Core Redirect 필요 여부
- 질문 내용
  - 기존 `BeeBrushUseAreaMesh` 타입 변경, 기존 provider class/API 삭제가 Blueprint serialized 참조에 미치는 영향을 어떻게 처리할지 결정한다.
- 필요한 이유
  - `UCLASS` rename은 아니더라도 UPROPERTY type 변경과 component class 변경은 BP compile/save가 필요하다.
  - 기존 class/API 삭제 시 BP override/function node가 깨질 수 있다.
- 선택지
  - 옵션 A: 새 class 추가 후 기존 API는 deprecated로 한 단계 유지하고 BP migration 후 삭제한다.
  - 옵션 B: 한 번에 삭제/교체하고 Editor compile/save 및 참조 scan으로 정리한다.
  - 옵션 C: 기존 API는 영구 compatibility layer로 둔다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

### [벌통 소비장 슬롯 배치/회수 설계 QnA]

1. 소비장 회수 시 내부 상태 보존 정책
- 질문 내용
  - 소비장 슬롯에서 배치된 소비장을 화분떡처럼 회수할 때, `ABeehiveCombActor`의 꿀 양, visible face, target bee count 같은 runtime 상태를 아이템으로 보존할지 결정한다.
- 필요한 이유
  - 현재 `UItemInstance`는 definition/stack/durability 중심이며 소비장 actor의 도메인 상태를 일반적으로 직렬화하는 계약이 없다.
  - 상태를 버리고 회수하면 꿀/벌 상태 손실이 생기고, 상태를 보존하려면 item instance 상태 확장 또는 별도 comb item state 계약이 필요하다.
- 선택지
  - 옵션 A: 1차 구현에서는 빈/idle 소비장만 회수 허용하고, 꿀/특수 상태가 있는 소비장은 회수 차단한다.
  - 옵션 B: 회수 시 소비장 상태를 버리고 기본 소비장 item 1개만 hotbar에 반환한다.
  - 옵션 C: `UItemInstance` 또는 comb 전용 item state를 확장해 소비장 상태를 보존한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 꿀양과 visible face만 보존하고 target bee count(여왕벌 포함) 가 0이 아니면 회수 불가능

2. 초기 배치 소비장의 회수 item definition
- 질문 내용
  - 벌통이 `CurrentCombCount`/기본 배치로 생성한 초기 소비장을 회수할 때 어떤 item definition을 hotbar에 반환할지 결정한다.
- 필요한 이유
  - 화분떡처럼 아이템으로 배치된 소비장은 source item definition을 저장하면 되지만, 기존 벌통이 자동 생성한 소비장은 source item instance가 없다.
  - 회수 성공 판정은 `TryAcquireItem(ItemDefinition, 1)`이므로 반환할 definition이 필요하다.
- 선택지
  - 옵션 A: `ABeehive`에 `DefaultCombItemDefinition`을 추가하고 초기 소비장 슬롯은 이 definition을 저장한다.
  - 옵션 B: source item definition이 없는 초기 소비장은 회수 불가로 둔다.
  - 옵션 C: 초기 소비장 자동 생성 기능을 제거하고 모든 소비장은 아이템 배치로만 생성한다.
- 권장 옵션:
  - 옵션 C
- 답변:
  - `ABeehive::DefaultCombItemDefinition`은 추가하지 않는다.
  - 아이템 사용으로 배치된 소비장은 배치 시점의 source item instance에서 반환 `ItemDefinition`을 주입받는다.
  - 이미 월드에 배치된 소비장은 actor/component 인스턴스에 명시된 authored fallback `ItemDefinition`을 사용할 수 있다.
  - source item 주입값과 authored fallback이 모두 없으면 회수 불가로 본다.

### [Generic Placement Occupant/Retrieve 설계 QnA]

1. 배치 점유자 계약 형태
- 질문 내용
  - 배치된 actor가 반환 item definition, owning slot, 회수 가능 여부를 제공하는 계약을 interface로 둘지 component로 둘지 결정한다.
- 필요한 이유
  - interface는 가볍지만 actor class마다 구현이 필요하다.
  - component는 소비장, 화분떡, 향후 preplaced world item에 공통 부착할 수 있어 generic authoring에 유리하다.
- 선택지
  - 옵션 A: `UPlacementOccupantComponent`를 추가하고, 회수 정보/초기화/authoring fallback을 component가 소유한다.
  - 옵션 B: `IPlacementSlotOccupant` interface를 추가하고, 각 actor가 직접 구현한다.
  - 옵션 C: component와 interface를 모두 두되, retrieve action은 component를 우선 조회하고 interface는 compatibility fallback으로 둔다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

2. 이미 월드에 배치된 점유 actor와 slot 연결 방식
- 질문 내용
  - item placement 경로로 spawn되지 않고 레벨/BP에 미리 배치된 actor를 어떤 방식으로 slot의 occupied actor로 등록할지 결정한다.
- 필요한 이유
  - 회수 성공 시 `IItemPlacementSlot::ClearPlacedItem`을 호출하려면 occupant가 owning slot을 알아야 한다.
  - preplaced actor는 `TryPlaceItem`을 거치지 않으므로 runtime source item/slot 주입이 자동으로 발생하지 않는다.
- 선택지
  - 옵션 A: slot actor에 `InitialOccupantActor`를 `EditInstanceOnly`로 노출하고 BeginPlay/OnConstruction에서 점유자로 claim한다.
  - 옵션 B: slot actor가 attach된 child/nearby actor를 자동 스캔해 `UPlacementOccupantComponent`가 있는 actor를 claim한다.
  - 옵션 C: occupant component에 `AuthoredOwningSlotActor`를 노출하고 actor 쪽에서 slot을 지정한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A. slot actor가 `InitialOccupantActor`를 `EditInstanceOnly`로 노출하고, BeginPlay/OnConstruction에서 자기 슬롯의 occupied actor로 claim한다.
  - claim은 새 actor spawn이 아니라 기존 preplaced actor 등록이다.
  - slot은 `InitialOccupantActor`의 `UPlacementOccupantComponent`에 owning slot을 주입한다.
  - attach/snap 여부는 slot 설정으로 분리한다. 기본은 slot별 정책에 따르되, 소비장 슬롯은 attach point snap을 허용한다.

3. authored fallback item definition 노출 범위
- 질문 내용
  - preplaced occupant의 회수용 fallback `ItemDefinition`을 어디까지 디테일창에 노출할지 결정한다.
- 필요한 이유
  - source item으로 배치된 actor는 runtime definition을 사용하지만, preplaced actor는 별도 fallback이 없으면 회수할 수 없다.
  - class default에 설정하면 모든 BP 인스턴스가 같은 반환 item을 공유하고, instance only로 설정하면 레벨 배치별 명시성이 높아진다.
- 선택지
  - 옵션 A: `UPlacementOccupantComponent::AuthoredReturnItemDefinition`을 `EditInstanceOnly`로 노출한다.
  - 옵션 B: `EditAnywhere`로 노출해 BP class default와 level instance에서 모두 설정 가능하게 한다.
  - 옵션 C: authored fallback은 두지 않고 source item으로 배치된 actor만 회수 가능하게 한다.
- 권장 옵션:
  - 옵션 B
- 답변 : 옵션B

4. 회수 가능 조건 확장 방식
- 질문 내용
  - generic retrieve action이 actor별 회수 차단 조건을 어떻게 확인할지 결정한다.
- 필요한 이유
  - 소비장은 target bee count/여왕벌 상태에 따라 회수 차단이 필요하고, 향후 다른 preplaced item도 고유 조건이 생길 수 있다.
  - retrieve action이 actor class별로 분기하면 generic 경계가 깨진다.
- 선택지
  - 옵션 A: `UPlacementOccupantComponent`에 `BlueprintNativeEvent CanRetrievePlacementOccupant(Character)`를 두고 actor/BP별 override를 허용한다.
  - 옵션 B: actor가 별도 interface를 구현하면 retrieve action이 그 interface를 추가 조회한다.
  - 옵션 C: slot actor가 모든 회수 가능 조건을 판단한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

5. 기존 `APlacedItemActor` migration 방식
- 질문 내용
  - 기존 화분떡/placed item actor의 `ItemDefinition`/`OwningPlacementSlotActor` 보관과 retrieve action을 새 generic occupant component 구조로 어떻게 옮길지 결정한다.
- 필요한 이유
  - 기존 `UPlacedItemRetrievePartFocusActionComponent`는 `APlacedItemActor` cast에 의존한다.
  - 새 구조로 전환하면 소비장과 화분떡이 같은 retrieve action을 사용할 수 있지만, 기존 BP/native 계약 변화가 생긴다.
- 선택지
  - 옵션 A: `APlacedItemActor`에 `UPlacementOccupantComponent`와 `UPlacementSlotRetrievePartFocusActionComponent`를 붙이고 기존 getter는 deprecated wrapper로 유지한다.
  - 옵션 B: 기존 `APlacedItemActor` API를 즉시 제거하고 component API만 사용한다.
  - 옵션 C: 기존 placed item 경로는 유지하고 소비장만 새 component retrieve 경로를 사용한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A

6. clear 시 occupied actor 처리
- 질문 내용
  - `IItemPlacementSlot::ClearPlacedItem`이 generic occupied actor를 제거할 때 destroy만 할지, actor별 cleanup hook을 먼저 호출할지 결정한다.
- 필요한 이유
  - 화분떡은 destroy만으로 충분할 수 있지만, 소비장이나 향후 world item은 detach, 상태 저장, 연출 종료 같은 정리가 필요할 수 있다.
- 선택지
  - 옵션 A: 기본은 destroy로 유지하되, `UPlacementOccupantComponent::PreClearPlacementOccupant` hook을 먼저 호출한다.
  - 옵션 B: slot이 항상 destroy만 수행한다.
  - 옵션 C: occupant component가 clear 방식(`Destroy`, `DetachOnly`, `HideOnly`)을 설정한다.
- 권장 옵션:
  - 옵션 A
- 답변 : 옵션A
