### [소비장 면별 BeeBrush TargetBeeCount 설계 QnA]

1. 면별 `TargetBeeCount`의 의미와 `SpawnAmount` 관계
- 질문 내용
  - 현재 `ABeehiveCombActor`는 단일 `TargetBeeCount`를 `FrontFaceBeeNiagara`와 `BackFaceBeeNiagara` 양쪽에 동일하게 주입한다.
  - BeeBrush가 보이는 면만 감소하려면 front/back target 상태를 분리해야 한다.
  - 이때 기존 `SpawnAmount`를 각 면의 최대 벌 수로 볼지, 소비장 전체 벌 수로 보고 양면에 분배할지 결정한다.
- 필요한 이유
  - `SpawnAmount`를 면별 최대값으로 보면 기존 시각 밀도를 유지하면서 양면 target을 독립적으로 줄일 수 있다.
  - `SpawnAmount`를 소비장 전체 수로 보면 양면 합계가 colony 계산과 더 직관적으로 연결되지만, 기존 Niagara 표시 밀도와 계산식이 달라질 수 있다.
- 선택지
  - 옵션 A: `SpawnAmount`는 각 face Niagara의 최대/기준 값으로 유지하고, `FrontFaceTargetBeeCount`와 `BackFaceTargetBeeCount`를 각각 `0..SpawnAmount`로 clamp한다.
  - 옵션 B: `SpawnAmount`는 소비장 전체 기준 값으로 재정의하고, front/back target 합계가 `0..SpawnAmount`를 넘지 않도록 분배한다.
  - 옵션 C: `FrontFaceSpawnAmount`/`BackFaceSpawnAmount`까지 분리해 면별 spawn과 target을 모두 독립 상태로 관리한다.
- 권장 옵션:
  - 옵션 B
- 답변:
  - 옵션 B. 소비장에 할당되는 `TargetBeeCount`는 소비장 전체 벌 수로 본다.
  - `FrontFaceBeeNiagara`와 `BackFaceBeeNiagara`는 이 전체 `TargetBeeCount`를 절반씩 나눠 받는다.
  - `SpawnAmount`도 소비장 전체 기준 값으로 보고, face Niagara에는 `TargetBeeCount`와 같은 front/back 분배 방식으로 나눠 주입한다.
  - 기존처럼 양면 Niagara에 전체 `TargetBeeCount`를 동일하게 주입하는 해석은 잘못된 것으로 본다.

2. 기존 `GetTargetBeeCount()`와 회수 조건의 호환 의미
- 질문 내용
  - 면별 target을 도입한 뒤 기존 `GetTargetBeeCount()`와 소비장 회수 조건을 어떤 의미로 유지할지 결정한다.
  - 현재 회수 조건은 `GetTargetBeeCount() == 0` 및 queen 미부착이다.
- 필요한 이유
  - 기존 Blueprint/C++ 호출부를 즉시 모두 바꾸면 API 변경 폭과 Blueprint 영향이 커진다.
  - 한쪽 면만 0이 된 소비장을 회수 가능하게 하면 반대면 벌 상태가 남은 소비장이 회수되는 모순이 생길 수 있다.
- 선택지
  - 옵션 A: `GetTargetBeeCount()`는 `Max(FrontFaceTargetBeeCount, BackFaceTargetBeeCount)`를 반환하고, 회수는 양면 target이 모두 0일 때만 허용한다.
  - 옵션 B: `GetTargetBeeCount()`는 `Front + Back` 합계를 반환하고, 회수는 합계가 0일 때만 허용한다.
  - 옵션 C: 기존 `GetTargetBeeCount()` 의미를 visible face 값으로 바꾸고, 회수 조건은 별도 `GetTotalTargetBeeCount()` 기반으로 수정한다.
- 권장 옵션:
  - 옵션 B
- 답변:
  - 옵션 B. 기존 `GetTargetBeeCount()`는 소비장 전체 target bee count를 반환한다.
  - 면별 내부 상태를 도입하더라도 `GetTargetBeeCount()`의 의미는 `Front + Back` 합계로 유지한다.
  - 소비장 회수 조건은 양면 target 합계가 0이고 queen이 미부착일 때만 만족한다.

3. 시간 bucket/colony 갱신 시 BeeBrush 감소 상태 보존 범위
- 질문 내용
  - `ABeehive::RefreshCombSpawnAmounts()`는 현재 계산된 `SpawnAmount`를 각 active comb에 적용하면서 `SetSpawnAmountAndResetTargetBeeCount()`로 target도 reset한다.
  - 면별 BeeBrush 감소 후 colony population bucket이나 bee count 변경으로 spawn amount가 갱신될 때 감소된 face target을 유지할지, 양면을 새 spawn amount로 다시 채울지 결정한다.
- 필요한 이유
  - 양면을 매 갱신마다 reset하면 BeeBrush로 털어낸 벌이 다음 population 갱신에서 즉시 복구될 수 있다.
  - 반대로 감소 상태를 계속 보존하면 colony bee count 변화가 소비장 face Niagara에 충분히 반영되지 않을 수 있다.
- 선택지
  - 옵션 A: 기존 정책을 유지해 spawn amount 갱신 시 양면 target을 모두 새 `SpawnAmount`로 reset한다.
  - 옵션 B: spawn amount 갱신 시 각 면의 현재 target 비율을 보존한다. 예: `NewFaceTarget = RoundToInt(NewSpawnAmount * OldFaceTarget / OldSpawnAmount)`.
  - 옵션 C: BeeBrush로 감소한 lifted/visible face만 보존하고, 다른 face는 새 `SpawnAmount`로 reset한다.
- 권장 옵션:
  - 옵션 B
- 답변 : 옵션B

4. 소비장 흔들기 API와 BeeBrush API의 면별 동작 분리
- 질문 내용
  - 현재 `ApplyCombShakeByRatio()`도 `ReduceTargetBeeCountByRatio()`를 호출해 단일 target을 감소시킨다.
  - 면별 target 도입 후 흔들기와 BeeBrush가 같은 면별 감소 정책을 사용할지, 서로 다른 정책으로 분리할지 결정한다.
- 필요한 이유
  - BeeBrush는 사용자가 보고 있는 면을 직접 쓸어내는 행동이라 visible face 감소가 명확하다.
  - 흔들기는 소비장 전체 충격으로 볼 수도 있고, 현재 보이는 면 조작으로 볼 수도 있어 해석이 갈린다.
- 선택지
  - 옵션 A: BeeBrush만 visible face target 감소로 바꾸고, 기존 흔들기/legacy `ReduceTargetBeeCountByRatio`는 양면 감소로 유지한다.
  - 옵션 B: 흔들기도 visible face target만 감소하도록 바꾼다.
  - 옵션 C: 흔들기는 양면 감소, BeeBrush는 visible face 감소로 두되 API 이름을 명확히 분리하고 문서화한다.
- 권장 옵션:
  - 옵션 C
- 답변 : 옵션C

5. 홀수 `SpawnAmount`/`TargetBeeCount`의 front/back 분할 규칙
- 질문 내용
  - 소비장 전체 `SpawnAmount` 또는 `TargetBeeCount`가 홀수일 때 front/back에 어떻게 나눌지 결정한다.
  - 예: 전체 값이 `501`이면 `250/251` 중 어느 면이 1마리를 더 받을지 정해야 한다.
- 필요한 이유
  - 분할 합계는 항상 전체 `SpawnAmount`/`TargetBeeCount`와 일치해야 한다.
  - flip, 저장/복구, BeeBrush 감소 후 재분배에서 같은 규칙을 써야 시각값이 흔들리지 않는다.
- 선택지
  - 옵션 A: Front가 나머지 1을 가진다. 예: `Front = Ceil(Target/2)`, `Back = Floor(Target/2)`.
  - 옵션 B: Back이 나머지 1을 가진다. 예: `Front = Floor(Target/2)`, `Back = Ceil(Target/2)`.
  - 옵션 C: 현재 visible face가 나머지 1을 가진다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A. 홀수 분할 시 Front face가 나머지 1을 가진다.
  - 동일 규칙을 `SpawnAmount`와 `TargetBeeCount` 모두에 적용한다.

6. Niagara `User.SpawnAmount`의 면별 주입 규칙
- 질문 내용
  - 소비장 전체 `TargetBeeCount`를 front/back에 나누는 것과 별개로, `User.SpawnAmount`도 면별로 나눠 주입할지 결정한다.
  - 현재 `ABeehiveCombActor`는 `User.SpawnAmount`도 양면 Niagara에 동일한 전체 값을 주입한다.
- 필요한 이유
  - Niagara가 `SpawnAmount`를 면별 최대 particle 수나 생성 기준으로 사용한다면, `TargetBeeCount`만 나누고 `SpawnAmount`는 전체값을 넣는 경우 면별 표시 밀도/여유 particle 수가 의도와 달라질 수 있다.
  - 반대로 `SpawnAmount`를 나누면 기존 Niagara parameter 의미가 함께 바뀌므로 BP/Niagara authoring 확인이 필요하다.
- 선택지
  - 옵션 A: `User.TargetBeeCount`와 같이 `User.SpawnAmount`도 front/back에 절반씩 나눠 주입한다.
  - 옵션 B: `User.TargetBeeCount`만 나누고 `User.SpawnAmount`는 기존처럼 전체 소비장 값을 양면에 동일하게 주입한다.
  - 옵션 C: C++은 전체값과 면별값을 모두 별도 parameter로 제공하고 Niagara 쪽에서 선택한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A. `User.SpawnAmount`도 `User.TargetBeeCount`와 동일하게 front/back에 절반씩 나눠 주입한다.
  - `ABeehiveCombActor::GetSpawnAmount()`가 반환하는 값은 소비장 전체 `SpawnAmount` 의미로 유지한다.
  - face별 Niagara parameter에는 전체값이 아니라 분배된 face별 값을 넣는다.

### [배치 아이템/핫바 잔량 연동 설계 QnA]

7. 잔량 0 도달 시 배치 actor 처리 정책
- 질문 내용
  - 배치된 아이템의 `RemainingAmount`가 0이 되었을 때 placed actor와 slot 점유 상태를 어떻게 처리할지 결정한다.
  - 화분떡은 소모성 배치 아이템이므로 잔량 0 이후 월드에 남길지, 즉시 slot에서 제거할지 정책이 필요하다.
- 필요한 이유
  - 즉시 제거하면 소비 완료 상태가 명확하고 slot을 재사용할 수 있다.
  - 빈 actor를 남기면 visual/interaction 상태가 추가로 필요하고, 회수 시 0 잔량 아이템이 hotbar로 돌아갈 수 있다.
  - 아이템마다 "빈 껍데기"를 남겨야 하는 경우가 있을 수 있으므로 generic 잔량 component가 단일 정책을 강제하면 확장성이 떨어진다.
- 선택지
  - 옵션 A: 잔량 0 도달 시 slot을 자동 clear한다. 화분떡 기본 정책으로 사용한다.
  - 옵션 B: 잔량 0이어도 actor를 유지하고, 별도 empty visual/interaction 상태로 전환한다.
  - 옵션 C: `UPlacedItemRemainingComponent`에 `bClearOwningSlotWhenDepleted` 같은 per-item 정책 값을 두고 아이템별로 선택한다.
- 권장 옵션:
  - 옵션 C. 화분떡은 `bClearOwningSlotWhenDepleted=true`로 설정한다.
- 답변:
  - 옵션C

8. 잔량 보존 아이템의 stack 정책
- 질문 내용
  - `UItemInstance`에 잔량 상태가 있는 아이템을 hotbar/storage에서 stack 가능하게 둘지 결정한다.
  - 서로 다른 `RemainingAmount`를 가진 같은 item definition을 병합할 때 어떤 잔량을 보존할지 정책이 필요하다.
- 필요한 이유
  - 현재 stack merge는 definition 중심으로 동작하므로 instance별 잔량 상태가 섞이면 데이터 손실이 생길 수 있다.
  - 회수된 배치 아이템의 잔량을 정확히 보존하려면 instance 단위 상태가 유지되어야 한다.
- 선택지
  - 옵션 A: 잔량 상태를 보존해야 하는 item definition은 `MaxStack == 1`로 강제한다.
  - 옵션 B: 같은 `RemainingAmount/MaxAmount`를 가진 아이템끼리만 stack merge를 허용한다.
  - 옵션 C: stack 병합 시 잔량을 가중 평균으로 합산한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 B.
  - 잔량 상태는 신규 `RemainingAmount/MaxAmount` 필드가 아니라 기존 durability 값을 기준으로 비교한다.
  - 같은 item definition이고 `CurrentDurability/MaxDurability`가 같은 item instance끼리만 stack merge를 허용한다.
  - 서로 다른 durability를 가진 item instance는 같은 definition이어도 병합하지 않는다.

9. hotbar/storage UI의 잔량 표시 범위
- 질문 내용
  - 핫바나 storage 슬롯에서 잔량 ratio를 표시할지, 배치된 월드 actor 외관으로만 표시할지 결정한다.
  - 잔량 상태는 `UItemInstance`에 저장되므로 UI 표시 자체는 가능하다.
- 필요한 이유
  - UI에 표시하면 회수한 화분떡의 남은 양을 inventory에서도 확인할 수 있다.
  - 다만 UI 표현까지 포함하면 slot widget, icon overlay, tooltip 등의 범위가 추가되어 1차 구현 폭이 커진다.
- 선택지
  - 옵션 A: 1차 구현에서는 월드 actor 외관만 표시하고 hotbar/storage UI 표시는 제외한다.
  - 옵션 B: hotbar/storage slot에 잔량 bar 또는 overlay를 함께 표시한다.
  - 옵션 C: tooltip/detail text에만 잔량을 표시하고 slot icon에는 표시하지 않는다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 1차 구현에서는 배치된 월드 actor 외관으로만 잔량을 표시한다.
  - hotbar/storage slot의 잔량 bar, overlay, tooltip 표시는 이번 범위에서 제외한다.
  - UI 표시는 `UItemInstance::GetDurabilityRatio()` 기반으로 나중에 별도 확장할 수 있다.

10. 잔량 상태 저장 단위
- 질문 내용
  - `UItemInstance`와 placed actor component가 잔량을 절대량(`RemainingAmount`, `MaxAmount`)으로 저장할지, ratio만 저장할지 결정한다.
  - 화분떡 visual은 ratio만 있으면 충분하지만, 실제 소모 시스템은 시간/벌통 상태에 따라 절대 사용량이 필요할 수 있다.
- 필요한 이유
  - ratio-only는 단순하지만 max amount 변경, 소비량 계산, UI 표시에서 의미가 부족할 수 있다.
  - 절대량 저장은 소비 시스템과 저장/회수 동기화에 유리하고, visual ratio는 항상 파생값으로 계산할 수 있다.
- 선택지
  - 옵션 A: `RemainingRatio`만 저장한다.
  - 옵션 B: `RemainingAmount`와 `MaxAmount`를 저장하고 ratio는 파생값으로 계산한다.
  - 옵션 C: item definition에는 `MaxAmount`, item instance에는 `RemainingAmount`만 저장한다.
- 권장 옵션:
  - 옵션 B
- 답변:
  - 옵션 C.
  - 새 `RemainingAmount/MaxAmount` 상태를 만들지 않고 기존 durability 상태를 재사용한다.
  - `UItemDefinition::bUsesDurability`와 `UItemDefinition::MaxDurability`가 배치 잔량의 max/source 설정이 된다.
  - `UItemInstance::Durability`가 hotbar/storage에 있는 아이템의 현재 잔량이 된다.
  - 배치 중에는 `UPlacedItemRemainingComponent`가 durability 값을 복사해 런타임 잔량으로 소유하고, 회수 성공 시 반환된 `UItemInstance`에 `SetDurability(...)`로 write-back한다.

11. `UPlacedItemRemainingComponent`의 부착 방식
- 질문 내용
  - `UPlacedItemRemainingComponent`를 모든 `APlacedItemActor`의 기본 subobject로 둘지, 잔량을 사용하는 아이템을 배치할 때만 runtime으로 생성할지 결정한다.
  - 사용량이 없는 배치 아이템도 `APlacedItemActor`를 공용으로 사용하되, 잔량 component는 비활성 상태로 호환되어야 한다.
- 필요한 이유
  - 기본 subobject 방식은 component 조회/초기화/회수 write-back 경로가 항상 동일해져 구현이 단순하다.
  - runtime 생성 방식은 사용하지 않는 아이템의 component 수를 줄일 수 있지만, 생성/파괴/Blueprint 확장 경로가 복잡해진다.
- 선택지
  - 옵션 A: `APlacedItemActor`에 `UPlacedItemRemainingComponent`를 기본 subobject로 항상 부착하고, `bUsesRemaining=false`이면 비활성/무상태로 둔다.
  - 옵션 B: `bUsesRemaining=true`인 item definition을 배치할 때만 `UPlacedItemRemainingComponent`를 runtime으로 생성한다.
  - 옵션 C: 잔량이 필요한 아이템만 `APlacedItemActor` subclass 또는 BP child에서 component를 직접 붙인다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - `UPlacedItemRemainingComponent`는 `APlacedItemActor`의 기본 subobject로 항상 부착한다.
  - 사용량이 없는 아이템은 component를 비활성/무상태로 두며, 배치/회수 흐름은 기존 generic placed item 경로와 동일하게 동작한다.

12. 배치 잔량 설정의 데이터 소유 위치
- 질문 내용
  - `MaxDurability`, durability를 배치 잔량으로 사용할지 여부, `bClearOwningSlotWhenDepleted`, visual 방식 같은 배치 잔량 설정을 어디에 둘지 결정한다.
  - 화분떡 전용 actor를 만들지 않는다면 item definition 또는 placed actor 공용 설정 중 하나가 source of truth가 되어야 한다.
- 필요한 이유
  - item definition에 두면 같은 `APlacedItemActor` class를 여러 아이템이 공유해도 아이템별 잔량 정책을 분리할 수 있다.
  - 기존 `bUsesDurability`/`MaxDurability`를 재사용하면 `UItemInstance`에 별도 remaining state를 추가하지 않아도 hotbar/storage/배치 actor 간 잔량을 왕복시킬 수 있다.
  - placed actor/BP 기본값에 두면 actor class마다 정책이 묶여 공용 actor의 장점이 줄어든다.
- 선택지
  - 옵션 A: 수치 source는 기존 `UItemDefinition::bUsesDurability`/`MaxDurability`와 `UItemInstance::Durability`를 재사용하고, `UItemDefinition`의 `FPlacedItemRemainingSpec`에는 배치 중 durability를 잔량으로 사용할지 여부와 제거/visual 정책만 둔다.
  - 옵션 B: `UItemDefinition`에 `FPlacedItemRemainingSpec`을 두되 `MaxAmount`/`RemainingAmount` 계열 신규 잔량 수치를 별도로 정의한다.
  - 옵션 C: `APlacedItemActor` 또는 `UPlacedItemRemainingComponent`의 component default 값으로 잔량 수치와 정책을 정의한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - `MaxAmount`/`RemainingAmount` 신규 수치는 만들지 않고 기존 durability 시스템을 배치 잔량의 수치 source로 재사용한다.
  - `FPlacedItemRemainingSpec`은 수치가 아니라 `bUseDurabilityAsPlacedRemaining`, `bClearOwningSlotWhenDepleted`, `VisualComponentClass` 같은 배치 상태 정책만 소유한다.
  - `bUseDurabilityAsPlacedRemaining`은 `bUsesDurability=true`인 아이템 중 배치 actor가 durability를 잔량으로 복사/소모/write-back해도 되는 아이템을 구분하는 연결 스위치다.

13. 잔량 외관 표현 확장 방식
- 질문 내용
  - 배치 아이템의 잔량 ratio를 아이템별 외관으로 반영하는 확장 지점을 component class로 둘지, interface/BP event로 둘지 결정한다.
  - 화분떡은 mesh XY scale 방식이지만, 다른 아이템은 material, mesh 교체, Niagara parameter 등 다른 표현을 쓸 수 있다.
- 필요한 이유
  - 잔량 component가 외관 방식을 직접 알면 화분떡 전용 로직이 공용 component에 섞인다.
  - visual 확장 지점을 분리해야 `APlacedItemActor` 하나로 여러 배치 아이템을 처리할 수 있다.
- 선택지
  - 옵션 A: item definition의 remaining spec에 `VisualComponentClass`를 두고, `APlacedItemActor`가 해당 visual component를 생성/초기화한다.
  - 옵션 B: `APlacedItemActor`가 `ReceiveRemainingRatioChanged(float)` Blueprint event만 제공하고 아이템별 BP에서 구현한다.
  - 옵션 C: `IPlacedItemRemainingVisual` interface 구현체를 owner actor나 component에서 찾아 호출한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - item definition의 remaining spec에 `VisualComponentClass`를 두고, `APlacedItemActor`가 초기화 시 해당 visual component를 준비한다.
  - `UPlacedItemRemainingComponent`는 잔량 수치와 변경 이벤트만 담당하고, 아이템별 외관 반영은 visual component가 담당한다.
  - 화분떡은 mesh 면적 scale 전용 visual component로 처리한다.

14. 기존 `APollenPattyActor` 정리 정책
- 질문 내용
  - 화분떡 배치 전용 actor를 생성하지 않고 `APlacedItemActor` 공용 경로를 사용하기로 할 경우, 이미 존재하는 `APollenPattyActor` C++ class를 어떻게 처리할지 결정한다.
- 필요한 이유
  - 즉시 삭제하면 구조가 단순해지지만 기존 Blueprint/asset 참조가 깨질 수 있다.
  - deprecated wrapper로 남기면 migration은 쉽지만 화분떡 전용 actor 경로가 계속 남아 혼선을 만든다.
- 선택지
  - 옵션 A: `APollenPattyActor`를 제거하고 화분떡 item definition의 `PlacedActorClass`를 `APlacedItemActor`로 전환한다.
  - 옵션 B: `APollenPattyActor`를 deprecated compatibility wrapper로 남기되 신규 배치 경로에서는 사용하지 않는다.
  - 옵션 C: `APollenPattyActor`를 유지하되 내부 구현만 `APlacedItemActor`와 같은 component 조합으로 맞춘다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 화분떡 배치 전용 C++ actor 경로는 제거하고, 화분떡 item definition의 `PlacedActorClass`는 공용 `APlacedItemActor`를 사용한다.
  - 화분떡 mesh는 item definition의 `WorldMesh`/배치 표시 mesh 경로로 주입한다.
  - 기존 `APollenPattyActor` 참조가 남아 있다면 구현 단계에서 제거/rename 및 필요한 redirect/migration을 함께 처리한다.

15. 화분떡 scale visual의 기준 scale 처리
- 질문 내용
  - 화분떡 잔량 ratio를 scale로 표시할 때 `PattyMesh`의 relative scale을 절대값 `(sqrt(Ratio), sqrt(Ratio), 1)`로 덮을지, 배치/mesh authoring의 초기 scale에 곱해서 적용할지 결정한다.
- 필요한 이유
  - 절대값 적용은 요구사항 예시와 일치하고 계산이 명확하다.
  - 초기 scale 곱셈 방식은 mesh별 크기 보정이나 slot별 visual scale을 보존할 수 있지만, 최종 scale 값이 예시와 달라질 수 있다.
- 선택지
  - 옵션 A: 화분떡 visual component는 `PattyMesh` relative scale을 항상 `(sqrt(Ratio), sqrt(Ratio), 1)`로 설정한다.
  - 옵션 B: BeginPlay/초기화 시점의 base scale을 저장하고 `(BaseX*sqrt(Ratio), BaseY*sqrt(Ratio), BaseZ)`로 적용한다.
  - 옵션 C: item definition에 absolute/preserve-base 모드를 둔다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 화분떡 visual component는 잔량 ratio에 따라 placed mesh relative scale을 항상 `(sqrt(Ratio), sqrt(Ratio), 1)`로 설정한다.
  - Z scale은 잔량 변화로 수정하지 않는다.
  - 예: ratio `0.5`이면 `(sqrt(0.5), sqrt(0.5), 1)`을 적용한다.

16. durability 잔량 아이템 stack 병합 판정 tolerance
- 질문 내용
  - 같은 item definition이고 같은 durability 잔량을 가진 아이템끼리만 stack merge를 허용할 때, float 비교 tolerance를 얼마로 둘지 결정한다.
- 필요한 이유
  - exact float 비교는 소모/회수 과정의 미세 연산 오차로 병합이 실패할 수 있다.
  - tolerance가 너무 크면 실제로 다른 잔량의 아이템이 같은 잔량으로 취급되어 데이터 의미가 흐려진다.
- 선택지
  - 옵션 A: `KINDA_SMALL_NUMBER` 수준의 절대 tolerance(`0.0001f`)를 사용한다.
  - 옵션 B: `0.001f` 절대 tolerance를 사용한다.
  - 옵션 C: `MaxDurability * 0.0001f` 같은 ratio 기반 tolerance를 사용한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - durability stack merge 판정은 `FMath::IsNearlyEqual(A, B, 0.0001f)` 기준으로 처리한다.
  - 같은 item definition은 `MaxDurability`가 동일하다는 전제를 두므로 current durability 절대값만 비교한다.
  - 이 tolerance는 float 오차만 흡수하는 값이며, 시각적으로 구분될 수 있는 잔량 차이를 병합하지 않기 위한 보수적 기준이다.

17. invalid placed remaining 설정 처리
- 질문 내용
  - `bUseDurabilityAsPlacedRemaining=true`인데 `bUsesDurability=false`이거나 `MaxDurability <= 0`인 잘못된 item definition 설정을 어떻게 처리할지 결정한다.
- 필요한 이유
  - 잘못된 설정으로 배치 자체를 실패시키면 designer data 오류가 gameplay 흐름을 막을 수 있다.
  - 반대로 조용히 무시하면 설정 오류를 발견하기 어렵다.
- 선택지
  - 옵션 A: warning log를 남기고 `UPlacedItemRemainingComponent`를 비활성/무상태로 둔다.
  - 옵션 B: 배치 자체를 실패 처리한다.
  - 옵션 C: assert/check로 개발 중 즉시 중단한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - invalid config는 warning log를 남기고 잔량 component를 비활성/무상태로 둔다.
  - 배치 actor spawn과 generic 회수 흐름은 유지한다.

18. remaining visual component의 mesh 대상
- 질문 내용
  - `UPlacedItemRemainingVisualComponent`가 잔량 외관을 적용할 때 어떤 mesh를 기본 대상으로 삼을지 결정한다.
- 필요한 이유
  - 화분떡은 공용 `APlacedItemActor`의 표시 mesh 하나만 조정하면 충분하다.
  - 다중 mesh 아이템까지 1차 구현에서 일반화하면 visual component 설정과 탐색 규칙이 불필요하게 커진다.
- 선택지
  - 옵션 A: 1차 구현은 `APlacedItemActor`의 primary placed mesh를 기본 대상으로 한다.
  - 옵션 B: visual component마다 대상 mesh component reference를 item definition에서 지정한다.
  - 옵션 C: actor 하위 mesh를 tag/name으로 검색한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 화분떡 scale visual은 공용 `APlacedItemActor`의 primary placed mesh relative scale을 조정한다.
  - 다중 mesh 아이템은 이후 별도 visual component 또는 target selection spec으로 확장한다.

19. 회수 write-back 시점
- 질문 내용
  - 배치 아이템 회수 시 `UPlacedItemRemainingComponent`의 런타임 잔량을 언제 반환 `UItemInstance`에 기록할지 결정한다.
- 필요한 이유
  - hotbar acquire 실패 상태에서 placed actor를 수정하면 아이템이 월드에도 남고 hotbar 상태도 애매해질 수 있다.
  - 회수 성공 후 반환된 item instance가 확정되어야 정확한 write-back 대상이 생긴다.
- 선택지
  - 옵션 A: `TryAcquireItem` 성공 후 `LastModifiedItemInstance`가 확정된 뒤 `SetDurability(...)`로 write-back하고, 그 다음 slot clear를 수행한다.
  - 옵션 B: hotbar acquire 전에 임시 item instance를 만들고 먼저 durability를 기록한다.
  - 옵션 C: slot clear 직전 placed actor 상태를 읽고, clear 이후 새 item instance를 찾아 기록한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - hotbar acquire가 실패하면 placed actor와 잔량 상태는 그대로 유지한다.
  - acquire 성공 후 `LastModifiedItemInstance`에 durability를 write-back하고, 그 다음 slot clear/destroy를 진행한다.

20. 회수 시 state-aware acquire API 필요 여부
- 질문 내용
  - 배치 아이템 회수 시 기존 `TryAcquireItem(ItemDefinition, 1)`만 사용할지, durability 상태를 함께 전달하는 state-aware acquire 경로를 추가할지 결정한다.
- 필요한 이유
  - 기존 acquire가 durability 상태를 모르면 같은 item definition이지만 다른 durability를 가진 stack에 병합될 수 있다.
  - 그 뒤 `LastModifiedItemInstance->SetDurability(...)`를 호출하면 기존 stack 전체 durability를 잘못 덮어쓸 수 있다.
  - durability 잔량 아이템은 stack 병합 단계에서부터 같은 durability인지 확인해야 한다.
- 선택지
  - 옵션 A: `FItemAcquireSpec` 같은 입력 구조체를 추가하고, item definition/quantity/durability override를 함께 전달하는 state-aware acquire API를 사용한다.
  - 옵션 B: 기존 `TryAcquireItem(ItemDefinition, 1)` 호출 후 `LastModifiedItemInstance`에 durability를 write-back한다.
  - 옵션 C: durability 잔량 아이템은 항상 `MaxStack == 1`로 강제해 기존 acquire를 유지한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 회수 경로는 durability 상태를 포함한 acquire spec을 hotbar에 전달한다.
  - hotbar acquire는 stack 선택/병합 단계에서 item definition과 durability를 함께 비교한다.
  - durability 비교는 16번 정책의 `0.0001f` tolerance를 사용한다.
  - acquire 성공 후에는 반환된 `LastModifiedItemInstance`가 이미 올바른 durability stack이므로, placed actor 상태를 해당 instance에 최종 동기화한 뒤 slot clear/destroy를 진행한다.
  - acquire 실패 시 placed actor와 잔량 상태는 그대로 유지한다.

21. source item instance가 없는 preplaced actor 초기 잔량
- 질문 내용
  - 레벨에 미리 배치된 actor 또는 `InitialOccupantActor`처럼 hotbar `SourceItemInstance` 없이 배치 점유 상태가 시작되는 경우 잔량을 어떻게 초기화할지 결정한다.
- 필요한 이유
  - runtime 배치는 `SourceItemInstance::Durability`를 복사하면 되지만, preplaced actor에는 복사할 source item instance가 없다.
  - 에디터에 미리 놓은 화분떡이 잔량 없는 상태로 시작하면 사용자가 의도한 기본 배치와 어긋날 수 있다.
- 선택지
  - 옵션 A: source item instance가 없으면 occupant의 return item definition 기준으로 full durability(`MaxDurability`)로 초기화한다.
  - 옵션 B: source item instance가 없으면 remaining component를 비활성/무상태로 둔다.
  - 옵션 C: preplaced actor마다 별도 authored current durability override 값을 요구한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - `SourceItemInstance`가 있으면 해당 instance의 durability를 런타임 잔량으로 복사한다.
  - `SourceItemInstance`가 없고 return item definition이 `bUsesDurability=true` 및 `bUseDurabilityAsPlacedRemaining=true`이면 full durability로 초기화한다.
  - invalid config는 17번 정책대로 warning log를 남기고 remaining component를 비활성/무상태로 둔다.
  - 부분 잔량 preplaced authoring은 1차 범위에서 제외하고, 필요해지면 별도 authored override 값으로 확장한다.

### [화분떡 고정 소모 로직 설계 QnA]

22. 화분떡 소모량 산출식
- 질문 내용
  - 화분떡 소모량을 벌 수, 온도, 여왕벌, bucket 길이 등과 연동할지, 벌통별 고정값으로만 처리할지 결정한다.
- 필요한 이유
  - 소모량 공식이 colony population과 연결되면 balance와 시스템 의존성이 커진다.
  - 사용자는 화분떡 소모량에 관여하는 다른 요인이 없고 벌통마다 고정치를 소모한다고 확정했다.
- 선택지
  - 옵션 A: `PollenPattyConsumptionAmountPerBucket` 고정값만 소모한다.
  - 옵션 B: `ColonyBeeCount` 또는 시간 길이 기반 공식으로 산출한다.
  - 옵션 C: 화분떡 item definition별 소모량 계수를 추가한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 화분떡 소모량은 벌통 설정값 `PollenPattyConsumptionAmountPerBucket`만 사용한다.
  - `ColonyBeeCount`, 여왕벌, 온도, bucket 길이, population/honey 계산 결과는 소모량에 관여하지 않는다.

23. 여러 화분떡 장착 시 소모 대상 수
- 질문 내용
  - 벌통에 여러 화분떡이 장착되어 있을 때 한 bucket에서 여러 개를 나눠 소모할지, 하나만 소모할지 결정한다.
- 필요한 이유
  - 균등 분배는 여러 화분떡의 잔량을 동시에 줄이지만, 사용자는 소모 당시 가장 왼쪽 또는 가장 오른쪽 화분떡만 소모한다고 확정했다.
- 선택지
  - 옵션 A: bucket마다 선택된 화분떡 1개만 소모한다.
  - 옵션 B: 모든 화분떡에 균등 분배한다.
  - 옵션 C: 선택된 화분떡이 부족하면 남은 소모량을 다음 화분떡으로 넘긴다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 한 소모 bucket에서는 선택된 화분떡 1개만 `ConsumeAmount(PollenPattyConsumptionAmountPerBucket)`를 받는다.
  - 선택된 화분떡이 0이 되어 제거되더라도 같은 bucket에서 남은 소모량을 다른 화분떡으로 넘기지 않는다.
  - 다음 bucket부터는 남아 있는 화분떡 중 다시 leftmost/rightmost를 선택한다.

24. 화분떡 left/right 판정 기준
- 질문 내용
  - 가장 왼쪽/오른쪽 화분떡을 어떤 좌표계와 축으로 판정할지 결정한다.
- 필요한 이유
  - world 좌표를 쓰면 벌통이 회전된 레벨 배치에서 좌/우 의미가 흔들릴 수 있다.
  - 벌통 내부 슬롯은 local Y 기준으로 중앙 정렬되는 구조가 이미 문서화되어 있다.
- 선택지
  - 옵션 A: 벌통 local Y 기준으로 판정한다. local Y 최소가 `Leftmost`, local Y 최대가 `Rightmost`다.
  - 옵션 B: world Y 기준으로 판정한다.
  - 옵션 C: slot 배열 index 기준으로 판정한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 소모 대상 위치 비교는 벌통 actor transform 기준 local Y 값을 사용한다.
  - `PollenPattyConsumptionSide=Leftmost`이면 local Y가 가장 작은 화분떡을 선택한다.
  - `PollenPattyConsumptionSide=Rightmost`이면 local Y가 가장 큰 화분떡을 선택한다.

25. 화분떡 소모 대상 식별 기준
- 질문 내용
  - `ABeehive`가 어떤 배치 actor를 화분떡 소모 대상으로 볼지 결정한다.
- 필요한 이유
  - 화분떡 전용 actor 경로는 제거되었고, 공용 `APlacedItemActor`를 사용한다.
  - placed actor class만으로 화분떡을 구분할 수 없으므로 slot metadata 또는 item definition metadata를 사용해야 한다.
- 선택지
  - 옵션 A: pollen slot의 `AreaTags`가 `ABeehive` 디테일 설정 `PollenPattyConsumptionAreaTags`와 매칭되고, occupied actor에 active `UPlacedItemRemainingComponent`가 있으면 소모 대상으로 본다.
  - 옵션 B: item definition의 별도 `Item.PollenPatty` tag를 새로 추가해 판정한다.
  - 옵션 C: visual component class가 `UPlacedItemAreaScaleRemainingVisualComponent`인지로 판정한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 기존 placement/use-area tag 체계를 재사용해 pollen slot 기준으로 수집한다.
  - `ABeehive`에 `PollenPattyConsumptionAreaTags`를 디테일창 노출 프로퍼티로 추가하고, 소모 대상 탐색은 이 설정값만 사용한다.
  - 런타임 탐색 로직에서 `Item.UseArea.Beehive.PollenPatty` 태그 문자열을 하드코딩하지 않는다.
  - 권장 기본 설정은 `PollenPattyConsumptionAreaTags={Item.UseArea.Beehive.PollenPatty}`다.
  - `PollenPattyConsumptionAreaTags`가 비어 있으면 소모 대상이 없는 것으로 처리한다.
  - slot `AreaTags`가 `PollenPattyConsumptionAreaTags`를 모두 포함할 때 매칭된 것으로 본다.
  - occupied actor에 active `UPlacedItemRemainingComponent`가 없으면 소모 대상에서 제외한다.
  - 신규 item identity tag는 이번 설계에서 추가하지 않는다.

26. 화분떡 소모 bucket 기본값
- 질문 내용
  - 화분떡 고정 소모를 어떤 bucket 기본값과 BeginPlay 정책으로 실행할지 결정한다.
- 필요한 이유
  - 기존 queen/colony/honey 갱신과 같이 시간 bucket 경계에서 처리하는 것이 일관적이다.
  - BeginPlay 즉시 소모가 켜져 있으면 배치 직후 예기치 않은 잔량 감소가 발생할 수 있다.
- 선택지
  - 옵션 A: `PollenPattyConsumptionBucketMinutes=60`, `bApplyPollenPattyConsumptionOnBeginPlayBucket=false`를 기본값으로 둔다.
  - 옵션 B: 10분 bucket으로 더 자주 소모한다.
  - 옵션 C: BeginPlay 즉시 1회 소모한다.
- 권장 옵션:
  - 옵션 A
- 답변:
  - 옵션 A.
  - 기본 소모 bucket은 60분이다.
  - BeginPlay 즉시 소모는 기본 비활성화한다.
  - bucket 길이는 소모 주기만 바꾸며, `PollenPattyConsumptionAmountPerBucket` 값 자체를 시간 비율로 스케일하지 않는다.

### [화분떡 인구 가속효과 설계 QnA]

27. 화분떡 인구 가속효과 적용 대상
- 질문 내용
  - 화분떡이 colony population 공식의 어느 항에 영향을 줄지 결정한다.
- 필요한 이유
  - 현재 공식에는 증가 항 `ItemEggLayingBonus`와 감소 항 `ItemLifespanBonus`가 분리되어 있다.
  - 화분떡을 어느 항에 넣느냐에 따라 "산란/육아 가속"인지 "수명 증가/감소 완화"인지 의미가 달라진다.
- 선택지
  - 옵션 A: 증가 항에만 적용한다. `ItemEggLayingBonus`를 통해 `Increase`만 곱한다.
  - 옵션 B: 감소 항에만 적용한다. `ItemLifespanBonus`를 통해 `Decrease`를 줄인다.
  - 옵션 C: 증가 항과 감소 항에 모두 적용한다.
- 권장 옵션:
  - 옵션 A. 화분떡은 인구 증가를 가속하는 보조 먹이로 보고, 벌 수 감소 완화와는 분리한다.
- 답변:
  - 옵션 A.
  - 화분떡 인구 가속효과는 colony population 증가 항에만 적용한다.
  - `ItemEggLayingBonus`를 통해 `Increase`만 곱하고, `ItemLifespanBonus`/`Decrease`에는 관여하지 않는다.

28. 여러 화분떡의 인구 가속효과 중첩
- 질문 내용
  - 벌통에 active 화분떡이 여러 개 있을 때 population bonus를 중첩할지 결정한다.
- 필요한 이유
  - 현재 화분떡 소모는 여러 개 중 leftmost/rightmost 1개만 소모한다.
  - bonus를 개수만큼 중첩하면 소모량은 1개 기준인데 효과는 여러 개 기준이 되어 밸런스가 과해질 수 있다.
- 선택지
  - 옵션 A: active 화분떡이 1개 이상이면 단일 bonus만 적용한다. 중첩하지 않는다.
  - 옵션 B: active 화분떡 개수만큼 선형 중첩한다.
  - 옵션 C: active 화분떡 개수만큼 중첩하되 최대 cap을 둔다.
- 권장 옵션:
  - 옵션 A. 단순하고 기존 "한 bucket에 한 개만 소모" 정책과 균형이 맞다.
- 답변:
  - 옵션 A.
  - active 화분떡이 1개 이상이면 단일 bonus만 적용한다.
  - 여러 화분떡이 있어도 population bonus는 중첩하지 않는다.
  - 단일 bonus 대상은 최고 tier/최대 `EggLayingMultiplier` 화분떡이 아니라 기존 화분떡 소모 대상 선택 정책과 동일하게 고른다.
  - 즉 `PollenPattyConsumptionSide`에 따라 벌통 local Y 기준 leftmost/rightmost active 화분떡 1개를 선택하고, 그 화분떡의 `EggLayingMultiplier`만 적용한다.

29. 화분떡 잔량과 인구 가속효과 연동 방식
- 질문 내용
  - 화분떡 remaining amount가 population bonus 크기에 영향을 줄지 결정한다.
- 필요한 이유
  - 잔량 비율 기반 bonus는 정교하지만, 부분적으로 남은 화분떡이 매 bucket마다 다른 효과를 내므로 밸런싱과 설명이 복잡해진다.
- 선택지
  - 옵션 A: remaining amount가 0보다 크면 full bonus를 준다. 잔량 비율은 bonus 크기에 관여하지 않는다.
  - 옵션 B: 가장 효과가 큰 화분떡 1개의 remaining ratio에 비례해 bonus를 준다.
  - 옵션 C: active 화분떡들의 remaining ratio 합계에 비례해 bonus를 준다.
- 권장 옵션:
  - 옵션 A. 잔량은 소모/외관/회수 상태만 담당하고, 인구 가속효과는 active 여부만 본다.
- 답변:
  - 옵션 A.
  - remaining amount가 0보다 크면 full bonus를 적용한다.
  - remaining ratio는 population bonus 크기에 관여하지 않는다.

30. 화분떡 인구 가속효과 설정 위치와 기본값
- 질문 내용
  - bonus 수치를 item definition에 둘지, 벌통 설정에 둘지 결정한다.
- 필요한 이유
  - item definition에 두면 아이템별 효과 확장이 쉽지만 inventory item stat 체계를 새로 열어야 한다.
  - 벌통 설정에 두면 현재 `ABeehive`의 colony population 계수 구조 안에서 작게 확장할 수 있다.
- 선택지
  - 옵션 A: `ABeehive`에 `PollenPattyEggLayingMultiplier`를 추가한다. 기본값은 `1.2f`다.
  - 옵션 B: `UItemDefinition`에 population bonus stat을 추가하고 화분떡 definition에서 설정한다.
  - 옵션 C: remaining ratio 기반 curve asset을 item definition 또는 벌통에 둔다.
  - 옵션 D: `UPollenPattyItemDefinition : UItemDefinition`를 추가하고, 화분떡 전용 subclass에 `EggLayingMultiplier`를 둔다.
- 권장 옵션:
  - 옵션 D. 여러 티어의 화분떡이 생길 수 있으므로 효과 수치는 아이템별로 두되, 일반 `UItemDefinition`에는 화분떡 전용 필드를 노출하지 않는다.
- 답변:
  - 옵션 D.
  - `UPollenPattyItemDefinition`를 `UItemDefinition` subclass로 추가한다.
  - `EggLayingMultiplier`는 `UPollenPattyItemDefinition`에만 둔다.
  - 일반 아이템은 기존 `UItemDefinition`을 계속 사용하므로 인구 가속 수치가 노출되지 않는다.
  - 여러 티어의 화분떡은 각각 `UPollenPattyItemDefinition` asset으로 만들고 `EggLayingMultiplier`만 다르게 설정한다.
  - 기본 권장값은 `EggLayingMultiplier=1.2f`다.

31. 화분떡 인구 가속효과 적용 시점과 소모 순서
- 질문 내용
  - `ColonyPopulation` bucket과 `PollenPattyConsumption` bucket이 같은 시간 경계에서 함께 발생할 때 어떤 순서로 처리할지 결정한다.
- 필요한 이유
  - population update 전에 화분떡을 먼저 소모하면, 잔량이 적어 해당 bucket에서 사라지는 화분떡은 그 경계의 population bonus를 주지 못한다.
  - population update 후에 소모하면, 직전 interval 동안 존재했던 화분떡이 해당 interval의 효과를 준 뒤 소모되는 의미가 된다.
- 선택지
  - 옵션 A: `ColonyPopulation`을 먼저 처리하고 그 뒤 `PollenPattyConsumption`을 처리한다.
  - 옵션 B: `PollenPattyConsumption`을 먼저 처리하고 그 뒤 `ColonyPopulation`을 처리한다.
  - 옵션 C: population bonus와 consumption을 별도 bucket/별도 시점으로 분리한다.
- 권장 옵션:
  - 옵션 A. 현재 bucket subscription 순서와 맞고, "지난 interval 동안 먹이가 있었으므로 성장 bonus를 받고 이후 소모된다"는 의미가 명확하다.
- 답변:
  - 옵션 A.
  - 같은 시간 경계에서는 `ColonyPopulation`을 먼저 처리하고 그 뒤 `PollenPattyConsumption`을 처리한다.
  - 직전 interval 동안 active 화분떡이 있었다면 해당 population update에서 bonus를 받은 뒤 소모된다.

32. 인구 가속효과 대상 화분떡 식별 기준
- 질문 내용
  - population bonus 대상 화분떡을 consumption 대상과 같은 기준으로 찾을지 결정한다.
- 필요한 이유
  - 별도 tag/slot 기준을 두면 bonus만 주고 소모되지 않는 화분떡 같은 예외 상태가 생길 수 있다.
- 선택지
  - 옵션 A: 기존 `PollenPattyConsumptionAreaTags`와 active `UPlacedItemRemainingComponent` 기준을 그대로 재사용한다.
  - 옵션 B: `PollenPattyPopulationBonusAreaTags`를 별도로 추가한다.
  - 옵션 C: item definition의 별도 population bonus tag/stat으로 판정한다.
- 권장 옵션:
  - 옵션 A. 소모 대상과 효과 대상을 일치시켜 authoring 실수를 줄인다.
- 답변:
  - 옵션 A.
  - 인구 가속효과 대상 화분떡은 기존 `PollenPattyConsumptionAreaTags`와 active `UPlacedItemRemainingComponent` 기준을 재사용한다.
  - 별도 population bonus tag/stat은 이번 설계에서 추가하지 않는다.
