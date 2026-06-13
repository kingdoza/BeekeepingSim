# Architecture QnA

## 확정 전제

- 이전 밀도 관련 QnA 내용은 사용자 요청에 따라 제거했다.
- 분봉 1차 구현은 외부 Blueprint에서 수동으로 시작해 테스트하는 범위다. 자동 발생 조건, 시간 bucket, AI/시뮬레이션 연동은 이번 범위에서 제외한다.
- 분봉 시작 시 기존 벌통의 `ColonyBeeCount`, 기존 `QueenBeeChildActor`, 소비장 벌 수/target count는 변경하지 않는다.
- 분봉 본진의 여왕벌은 기존 벌통 여왕벌을 이동하지 않고, 분봉 본진 actor가 별도 spawn/child actor로 소유한다.
- `벌 운반통` 포획 결과는 이번 범위에서 item instance runtime state로 저장하지 않는다. 포획 진행 source of truth는 분봉 본진 actor의 `AliveRadius`다.

## 미해결 질문

### [꿀 용기와 소분 작업대]

1. 꿀 용기 item state 저장 모델
- 질문 내용: 꿀 용기의 현재 용량(ml), 전체 용량(ml), 꿀 밀도(0..1), 꿀 숙성도(0..1)를 inventory item과 placed actor 사이에서 어떤 모델로 보존할 것인가?
- 필요한 이유: 기존 durability/placed remaining 모델은 단일 float 잔량만 안정적으로 다룬다. 꿀 용기는 여러 float state를 가지며 회수/재배치/보관 이동 후에도 동일 state가 유지되어야 한다.
- 선택지
  - 옵션 A: `UItemInstance`에 `FHoneyContainerItemState`를 추가하고, 정적 용량/기본값은 `UHoneyContainerItemDefinition` 또는 item definition 확장에 둔다. 꿀 용기 item은 `MaxStack=1`을 invariant로 둔다.
  - 옵션 B: 현재 용량은 durability로 재사용하고, 밀도/숙성도는 placed actor 런타임 state로만 둔다.
  - 옵션 C: 꿀 용기 state를 placed actor에만 저장하고 inventory 회수 시 기본값으로 초기화한다.
- 권장 옵션: 옵션 A. 상태 보존 범위가 명확하고, 기존 소비장 `FBeehiveCombItemState` 패턴과 일관된다. 단, item clone/acquire 경로에서 honey container state 복사 또는 회수 직후 write-back을 명시해야 한다.
- 답변: 옵션 A. 현재 inventory 생성 경로는 `UItemInstance` 단일 class를 직접 생성하므로, `UItemInstance` base에 optional `FHoneyContainerItemState`를 추가한다. 정적 용량/기본값은 `UHoneyContainerItemDefinition` 또는 item definition 확장에 두고, 꿀 용기 item은 `MaxStack=1`을 invariant로 둔다. 비-용기 item은 `bHasState=false`인 honey container state를 무시한다.

2. 꿀 용기 placed actor 구성
- 질문 내용: 말통/꿀통으로 배치되는 월드 actor를 어떤 C++ class 경로로 구현할 것인가?
- 필요한 이유: 꿀 용기는 `UPlacementOccupantComponent` 기반 회수, 꿀표현 메시 visual update, 소분 작업대 source일 때 노즐 PartFocus descriptor까지 제공해야 한다.
- 선택지
  - 옵션 A: `AHoneyContainerActor` 전용 WorldActor를 추가한다. 기본 구성은 container mesh, honey visual mesh, optional nozzle hit component, `UPlacementOccupantComponent`, retrieve action, honey state/visual API다.
  - 옵션 B: `APlacedItemActor`에 꿀 용기 state/visual/nozzle 기능을 직접 추가한다.
  - 옵션 C: C++ 전용 class 없이 Blueprint actor와 기존 generic placement만 사용한다.
- 권장 옵션: 옵션 A. generic `APlacedItemActor`를 비대하게 만들지 않고, 꿀 용기 전용 state와 노즐 상호작용을 한 actor 경계에 캡슐화할 수 있다.
- 답변 : 옵션A

3. 말통 슬롯과 꿀통 슬롯 구분
- 질문 내용: 소분 작업대의 말통 슬롯과 꿀통 슬롯은 어떤 기준으로 수용 item을 구분할 것인가?
- 필요한 이유: 두 슬롯 모두 꿀 용기를 받지만 source/target 역할과 mesh/노즐/pour target 정책이 다르다. 잘못된 item 배치를 막아야 transfer state와 prompt가 단순해진다.
- 선택지
  - 옵션 A: `AHoneyContainerSlotActor`를 추가하고 `EHoneyContainerSlotRole(SourceCan, TargetJar)`와 accepted gameplay tag query를 둔다. 예: `Item.HoneyContainer.SourceCan`, `Item.HoneyContainer.TargetJar`.
  - 옵션 B: 기존 `AItemPlacementSlotActor` 두 개를 BP에서만 설정하고, C++ role 검증은 하지 않는다.
  - 옵션 C: 하나의 슬롯 class가 모든 꿀 용기를 허용하고, 작업대가 런타임에 source/target 가능 여부를 추론한다.
- 권장 옵션: 옵션 A. slot role을 C++ source of truth로 둬서 배치 실패, prompt availability, 자동 stop 조건을 같은 기준으로 판정할 수 있다.
- 답변: 옵션 A. 단, slot role enum은 말통/꿀통 고정명(`SourceCan`, `TargetJar`)이 아니라 재사용 가능한 `Source`, `Target`으로 둔다. `AHoneyContainerSlotActor`는 role과 accepted gameplay tag query만 소유하고, 어떤 용기 조합을 받을지는 작업대/BP authoring tag query로 설정한다. 실제 꿀 이송, Niagara, 자동 stop 조건은 작업대 actor가 소유한다.

4. 노즐 클릭과 transfer ownership
- 질문 내용: 배치된 말통 배출구 노즐 클릭은 어느 actor/component가 처리하고, 실제 꿀 이동 state는 누가 소유할 것인가?
- 필요한 이유: 노즐은 배치된 말통의 일부처럼 보이지만, 이송에는 source slot과 target slot 둘 다 필요하다. state owner가 분산되면 stop 조건과 Niagara 파라미터 갱신이 흔들린다.
- 선택지
  - 옵션 A: source 말통 `AHoneyContainerActor`가 nozzle PartFocus descriptor를 제공하고, `UHoneyNozzlePartFocusActionComponent`가 owning `AHoneyDecantingTable`에 toggle 요청만 보낸다. 실제 transfer tick, validation, Niagara는 작업대가 소유한다.
  - 옵션 B: 작업대가 고정 nozzle component를 소유하고, 말통 actor는 visual container로만 둔다.
  - 옵션 C: 노즐 클릭을 item-use action으로 처리해 선택 아이템 사용 경로에 넣는다.
- 권장 옵션: 옵션 A. 사용자가 클릭하는 대상은 말통 노즐로 유지하면서, source/target을 함께 알아야 하는 domain mutation은 작업대에 집중된다.
- 답변 : 옵션A

5. 꿀 이동과 혼합 규칙
- 질문 내용: 말통에서 꿀통으로 이동할 때 target 꿀통에 이미 꿀이 있으면 밀도와 숙성도를 어떻게 갱신할 것인가?
- 필요한 이유: 밀도/숙성도는 단순 덮어쓰기를 하면 기존 target 내용물이 사라지고, 혼합을 허용하지 않으면 소분 UX가 제한된다. gameplay state 규칙을 먼저 확정해야 한다.
- 선택지
  - 옵션 A: 매 tick `min(TransferRateMlPerSecond * DeltaTime, SourceVolumeMl, TargetFreeVolumeMl)`만큼 이동한다. source는 volume만 감소하고 밀도/숙성도는 유지한다. target은 volume-weighted average로 밀도/숙성도를 갱신한다. target이 비어 있으면 source 값을 그대로 복사한다.
  - 옵션 B: target이 비어 있거나 source와 동일한 밀도/숙성도일 때만 이동을 허용한다.
  - 옵션 C: target의 밀도/숙성도는 항상 source 값으로 덮어쓴다.
- 권장 옵션: 옵션 A. 소분/혼합 gameplay로 가장 자연스럽고, volume 단위 state와도 일관된다. 단, 결과 밀도 또는 source 밀도가 `1.0` 미만이면 숙성도는 저장해도 gameplay 판단에는 사용하지 않는다.
- 답변: 옵션 A. tick별 이동량은 `min(TransferRateMlPerSecond * DeltaTime, SourceVolumeMl, TargetFreeVolumeMl)`로 계산한다. target의 `HoneyDensity`와 `HoneyRipeness`는 기존 target 내용물과 유입량을 volume-weighted average로 계산한다. 단, 유입 꿀 또는 혼합 결과의 `HoneyDensity`가 `1.0` 미만이면 target `HoneyRipeness`는 `0.0`으로 정규화한다. 모든 꿀 용기 state는 `HoneyDensity < 1.0 && HoneyRipeness == 0.0` 또는 `HoneyDensity == 1.0 && HoneyRipeness >= 0.0` invariant를 만족해야 한다.

6. DropLength 연출과 실제 이송 타이밍
- 질문 내용: 꿀 줄기 Niagara의 `DropLength`가 0에서 목적값까지 증가하는 동안 실제 volume 이동은 언제 시작할 것인가?
- 필요한 이유: 시각적으로 줄기가 아직 꿀통에 닿지 않은 시간에도 target volume이 증가할지 여부는 gameplay/표시 싱크에 영향을 준다.
- 선택지
  - 옵션 A: 노즐 클릭으로 배출이 시작되면 volume 이동도 즉시 시작한다. `DropLength` 증가는 visual-only grow 연출이며, source empty/target full/slot invalid/focus cancel/slot clear 시 자동 stop한다.
  - 옵션 B: `DropLength`가 목적값에 도달한 뒤부터 target volume을 증가시킨다.
  - 옵션 C: 클릭 1회마다 고정량을 즉시 이동하고, Niagara는 짧은 피드백만 재생한다.
- 권장 옵션: 옵션 A. 사용자가 "배출 시작"을 조작한 순간부터 상태 변화가 일어나며, Niagara grow는 출발 연출로 분리된다.
- 답변 : 옵션B

7. DropLength 목적값 산출
- 질문 내용: Niagara `DropLength`의 목적값(cm)은 어떤 기준으로 계산할 것인가?
- 필요한 이유: 말통/꿀통 mesh와 슬롯 위치는 BP/레벨 authoring에 따라 달라질 수 있다. 고정값이면 배치 variant에서 줄기 길이가 어긋날 수 있다.
- 선택지
  - 옵션 A: source container의 stream 기준 component와 target slot/container의 `PourTarget` scene component 기준으로 계산한다. component가 없으면 작업대 authored `DefaultDropLengthCm`로 fallback한다.
  - 옵션 B: 작업대에 `TargetDropLengthCm` float를 두고 항상 authored 값만 사용한다.
  - 옵션 C: C++은 DropLength를 계산하지 않고 BP가 전부 세팅한다.
- 권장 옵션: 옵션 A. BP authoring 유연성을 유지하면서 C++ transfer state와 Niagara parameter를 동기화할 수 있다.
- 답변 : 옵션A. 후속 확정으로 source 기준 component는 `AHoneyContainerActor::HoneyStreamNiagara`이며, 목표 길이는 source stream world Z와 target container/slot `PourTarget` world Z 차이(`Max(0, SourceStream.Z - TargetPourTarget.Z)`)로 계산한다. 기존 `NozzleOrigin` world distance 기준은 사용하지 않는다.

8. HoneyRipeness/HoneyRipness 파라미터 철자
- 질문 내용: 꿀 용기 material과 꿀 줄기 Niagara에 적용할 숙성도 scalar parameter 이름은 `HoneyRipeness`와 `HoneyRipness` 중 무엇으로 확정할 것인가?
- 필요한 이유: 기존 소비장 honey material 계약은 `HoneyRipeness`를 사용한다. 새 기능에서 `HoneyRipness`를 쓰면 거의 같은 의미의 Blueprint/Niagara 파라미터가 두 개 생긴다.
- 선택지
  - 옵션 A: 기존 계약과 맞춰 `HoneyRipeness`로 통일한다. 새 material/Niagara도 `HoneyDensity`, `HoneyRipeness`, `DropLength`를 사용한다.
  - 옵션 B: 요청 문구 그대로 `HoneyRipness`를 새 계약으로 사용한다.
  - 옵션 C: 이행 기간에는 C++이 `HoneyRipeness`와 `HoneyRipness` 둘 다 세팅하고, 문서 정본은 하나만 고른다.
- 권장 옵션: 옵션 A. 오탈자성 API가 누적되는 것을 막고 기존 `ABeehiveCombActor` material parameter와 일관성을 유지한다.
- 답변 : 옵션A. 내가 `HoneyRipness`라 쓴거는 영어 미숙 이슈임. `HoneyRipeness`가 맞음.

### [꿀 이송 재사용성]

1. 공통 꿀 이송 로직의 위치
- 질문 내용: 말통->꿀통 소분뿐 아니라 다른 작업대의 꿀 용기 간 이송에서도 같은 규칙을 재사용하려면 실제 transfer 계산과 state mutation을 어디에 둘 것인가?
- 필요한 이유: `AHoneyDecantingTable`에 직접 구현하면 소분 작업대 전용 로직이 되어 다른 작업대에서 재사용하기 어렵다. 이동량 공식, 혼합 규칙, density/ripeness invariant, source/target validation은 공통 도메인 규칙이다.
- 선택지
  - 옵션 A: `UHoneyTransferComponent`를 추가하고 작업대 actor가 source slot, target slot, VFX anchor만 연결한다.
  - 옵션 B: `FHoneyTransferUtils` 같은 static helper에 계산/적용 함수를 두고, 작업대 actor가 transfer state와 VFX를 소유한다.
  - 옵션 C: `AHoneyDecantingTable`에 직접 구현하고 후속 작업대에서 필요할 때 복사/리팩토링한다.
- 권장 옵션: 옵션 A. transfer 진행 상태, grow phase, 실제 이송 phase, 자동 stop 조건까지 component가 소유하면 여러 작업대 actor가 조립식으로 재사용할 수 있다.
- 답변 : 옵션A

2. 노즐 PartFocus descriptor 제공 경로
- 질문 내용: source slot에 배치된 꿀 용기의 노즐 클릭 descriptor는 어느 경로에서 제공할 것인가?
- 필요한 이유: 기존 generic placement slot의 occupied descriptor는 주로 회수 action에 맞춰져 있다. 노즐 클릭은 source 용기 전용 toggle이며, 작업대 host의 transfer component와 연결되어야 한다.
- 선택지
  - 옵션 A: `AHoneyContainerActor`가 nozzle hit component와 `UHoneyNozzlePartFocusActionComponent`를 소유하고 PartFocus descriptor를 제공한다. slot/host provider는 occupied actor의 descriptor를 수집한다.
  - 옵션 B: `AHoneyContainerSlotActor`가 occupied container의 nozzle component를 조회해 descriptor를 직접 구성한다.
  - 옵션 C: 작업대 actor가 source slot 위에 고정 nozzle hit component를 별도로 두고 container 노즐은 visual-only로 둔다.
- 권장 옵션: 옵션 A. 사용자가 클릭하는 대상과 action owner가 꿀 용기 actor에 있어 자연스럽고, 다른 작업대에서도 같은 container actor를 재사용할 수 있다. 단, action은 concrete 작업대 class가 아니라 transfer host/interface/component에 toggle 요청해야 한다.
- 답변 : 옵션A

3. 꿀 용기 회수 시 state write-back 경로
- 질문 내용: 배치된 꿀 용기를 회수할 때 runtime `FHoneyContainerItemState`를 hotbar의 `UItemInstance`에 어떤 경로로 기록할 것인가?
- 필요한 이유: generic `UPlacementSlotRetrievePartFocusActionComponent`는 durability remaining만 write-back한다. 꿀 용기 state를 별도 처리하지 않으면 배치 중 바뀐 용량/밀도/숙성도가 회수 item에 보존되지 않을 수 있다.
- 선택지
  - 옵션 A: `UHoneyContainerRetrievePartFocusActionComponent`를 추가해 generic retrieve 성공 후 `WriteHoneyContainerStateToItemInstance(AcquiredItemInstance)`를 호출하고 slot clear를 수행한다.
  - 옵션 B: generic retrieve component에 optional interface hook을 추가해 occupant actor가 acquired item instance에 state를 write-back할 수 있게 한다.
  - 옵션 C: `UPlacementOccupantComponent::PreClearPlacementOccupant`에서 write-back한다.
- 권장 옵션: 옵션 A. 소비장 전용 retrieve bridge와 같은 패턴이며, generic placement retrieve 계약을 넓히지 않고 꿀 용기 state 보존을 명확히 보장한다.
- 답변 : 옵션A

4. DropLength grow 설정
- 질문 내용: 배출 시작 후 꿀 줄기 Niagara `DropLength`가 0에서 목적값까지 증가하는 속도는 무엇으로 설정할 것인가?
- 필요한 이유: 6번 답변이 옵션 B이므로 실제 volume 이동은 `DropLength`가 목적값에 도달한 뒤 시작한다. grow 시간/속도가 미정이면 이송 시작 타이밍과 UX가 작업대마다 임의 구현될 수 있다.
- 선택지
  - 옵션 A: `DropLengthGrowSpeedCmPerSecond`를 두고 `CurrentDropLength += Speed * DeltaTime`으로 목적값까지 증가시킨다.
  - 옵션 B: `DropGrowDurationSeconds`를 두고 목적값까지 normalized alpha로 보간한다.
  - 옵션 C: BP가 Niagara timeline을 전부 소유하고 C++은 grow 완료 이벤트만 받는다.
- 권장 옵션: 옵션 A. 목적 거리(cm)와 같은 단위의 속도라서 작업대/용기 배치가 달라져도 같은 물리적 grow 속도를 유지하기 쉽다.
- 답변 : 옵션A

5. `MaxVolumeMl` 소유 위치
- 질문 내용: 꿀 용기의 전체 용량(`MaxVolumeMl`)은 item definition과 item instance state 중 어디에 둘 것인가?
- 필요한 이유: `CurrentVolumeMl`은 런타임 상태지만 전체 용량은 정적 용기 스펙일 수도 있고, 업그레이드/특수 상태로 인스턴스별 변경될 수도 있다. 소유 위치를 정해야 state 구조와 visual fill ratio 계산이 일관된다.
- 선택지
  - 옵션 A: `UHoneyContainerItemDefinition::MaxVolumeMl`에 둔다. `FHoneyContainerItemState`에는 `CurrentVolumeMl`, `HoneyDensity`, `HoneyRipeness`, `bHasState`만 둔다.
  - 옵션 B: `FHoneyContainerItemState`에도 `MaxVolumeMl`을 저장해 인스턴스별 최대 용량 변경을 허용한다.
  - 옵션 C: placed actor BP authoring 값으로만 둔다.
- 권장 옵션: 옵션 A. 현재 요구에는 인스턴스별 최대 용량 변화가 없으므로 definition이 source of truth가 되는 편이 단순하고, item 회수/재배치 시 상태 복사량도 줄어든다.
- 답변 : 옵션A
