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
