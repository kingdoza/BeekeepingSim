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