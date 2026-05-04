## Beehive Comb Actor 설계 QnA

### [질문 1]

1. 소비장 액터 클래스 이름과 위치
- 질문 내용: 벌통 내부 소비장을 담당할 신규 C++ actor 이름과 시스템 위치를 확정해야 한다.
- 필요한 이유: 소비장은 월드 배치 가능한 독립 외형/연출 단위이지만 벌통 내부 구성물로 생성/관리된다. 이름이 일반적인 `CombActor`일 경우 향후 다른 comb 개념과 충돌할 수 있다.
- 선택지
  - 옵션 A: `ABeehiveCombActor`를 `WorldActors` 시스템에 추가한다.
  - 옵션 B: `ACombActor`를 `WorldActors` 시스템에 추가한다.
  - 옵션 C: 별도 신규 시스템을 만들고 그 아래에 소비장 actor를 둔다.
- 권장 옵션: 옵션 A. 벌통 내부 구성물이라는 소유 맥락이 이름에 드러나고, 현재 Beehive 관련 actor들이 `WorldActors`에 있으므로 시스템 경계가 가장 자연스럽다.
- 답변: 옵션 A

---

### [질문 2]

1. 소비장 생성/소유 방식
- 질문 내용: `ABeehive`가 소비장 actor를 어떤 방식으로 생성하고 소유할지 확정해야 한다.
- 필요한 이유: 소비장은 벌통 내부에 종속되는 구성물이며, 에디터 프리뷰와 런타임 갱신 모두에서 안정적으로 재배치되어야 한다.
- 선택지
  - 옵션 A: `ABeehive`가 `UChildActorComponent` 슬롯을 만들고 각 슬롯에 `ABeehiveCombActor` child actor를 소유한다.
  - 옵션 B: `ABeehive`가 런타임에 `SpawnActor<ABeehiveCombActor>()`로 생성하고 attach한다.
  - 옵션 C: 소비장을 레벨 독립 actor로 배치하고 `ABeehive`가 참조 배열로 관리한다.
- 권장 옵션: 옵션 A. 벌통 내부 구성물이라는 소유 관계가 명확하고, construction/editor preview 경로에서 상대 transform을 관리하기 쉽다.
- 답변: 옵션 A

---

### [질문 3]

1. 비활성 소비장 슬롯 처리 방식
- 질문 내용: `CurrentCombCount`가 `MaxCombCount`보다 작을 때 초과 슬롯의 소비장 actor를 제거할지, 숨김/비활성화할지 확정해야 한다.
- 필요한 이유: 요구사항은 `CurrentCombCount`만큼 소비장 actor가 배치되는 것이다. 다만 editor 안정성과 runtime 비용 측면에서 제거와 비활성화의 트레이드오프가 있다.
- 선택지
  - 옵션 A: `MaxCombCount`만큼 child actor component 슬롯은 유지하고, 초과 슬롯의 child actor는 제거하거나 class를 비운다.
  - 옵션 B: `MaxCombCount`만큼 child actor를 항상 유지하되 초과 슬롯은 hidden/inactive 처리한다.
  - 옵션 C: child actor component 자체를 매번 생성/삭제한다.
- 권장 옵션: 옵션 A. 슬롯 transform 계산과 에디터 구조는 안정적으로 유지하면서 실제 소비장 actor 수는 `CurrentCombCount`와 일치시킬 수 있다.
- 답변: 옵션 A

---

### [질문 4]

1. 소비장 균일 배치 기준값
- 질문 내용: `MaxCombCount`만으로는 소비장 배치 간격을 계산할 수 없으므로 벌통에 어떤 배치 기준값을 둘지 확정해야 한다.
- 필요한 이유: "균일 간격"과 "로컬 -Y 방향부터 채움"을 구현하려면 전체 배치 폭 또는 슬롯 간격 중 하나가 필요하다.
- 선택지
  - 옵션 A: `CombRackLengthY`를 벌통에 노출하고, 해당 전체 길이를 `MaxCombCount` 기준으로 균등 분할한다.
  - 옵션 B: `CombSlotSpacing`을 벌통에 노출하고, 슬롯 간 고정 간격으로 배치한다.
  - 옵션 C: 소비장 mesh bounds에서 자동 간격을 계산한다.
- 권장 옵션: 옵션 B. `CombSlotSpacing`을 source of truth로 두면 소비장 사이 간격을 디테일창에서 직접 조정할 수 있고, 질문 5의 `CombRackRoot` local space 기준 배치와 자연스럽게 결합된다.
- 답변: 옵션 B. `CombSlotSpacing`을 벌통 디테일창에 노출하고, 질문 5의 `CombRackRoot` 아래에서 local `-Y` 방향으로 소비장 간격에 적용한다. 최초 요구사항보다 이 QnA 답변을 우선한다.

---

### [질문 5]

1. 소비장 local transform 기준
- 질문 내용: 소비장 배치 포인트의 기준 root와 방향 축을 확정해야 한다.
- 필요한 이유: 요구사항은 로컬 `-Y` 방향부터 채우는 것이지만, 높이/전후 위치/회전 기준은 아직 명시되지 않았다.
- 선택지
  - 옵션 A: `ABeehive` root 기준 local transform을 사용하고, `CombRackOrigin`/`CombRackOffset` 같은 시작 기준 offset을 벌통에 둔다.
  - 옵션 B: `USceneComponent* CombRackRoot`를 벌통에 추가하고, 그 component local space에서 `-Y`부터 배치한다.
  - 옵션 C: 벌통 mesh bounds를 기준으로 자동 위치를 계산한다.
- 권장 옵션: 옵션 B. 디자이너가 벌통 내부 소비장 기준점을 에디터에서 직접 이동/회전할 수 있고, 계산은 `CombRackRoot` local `Y` 축만 사용하면 된다.
- 답변: 옵션 B

---

### [질문 6]

1. SpawnAmount 계산 반올림 정책
- 질문 내용: `SpawnAmount = 벌 수 * n2 / 소비장 수` 결과가 float일 때 int32로 변환하는 방식을 확정해야 한다.
- 필요한 이유: Niagara `SpawnAmount`는 int32이고, 반올림 방식에 따라 소비장별 벌 수 합계와 시각 밀도가 달라진다.
- 선택지
  - 옵션 A: `RoundToInt(ColonyBeeCount * n2 / CurrentCombCount)`
  - 옵션 B: `FloorToInt(ColonyBeeCount * n2 / CurrentCombCount)`
  - 옵션 C: `CeilToInt(ColonyBeeCount * n2 / CurrentCombCount)`
- 권장 옵션: 옵션 A. 기존 `AttractionSwarm`의 spawn 계산 정책과 일관되고, 비율 계산 결과가 한쪽으로 지속 편향되지 않는다.
- 답변: 옵션 A

---

### [질문 7]

1. CurrentCombCount가 0일 때 SpawnAmount 처리
- 질문 내용: 현재 소비장 수가 0일 때 SpawnAmount 계산과 Niagara 적용을 어떻게 처리할지 확정해야 한다.
- 필요한 이유: 계산식에 소비장 수가 분모로 들어가므로 divide-by-zero 방지가 필요하다.
- 선택지
  - 옵션 A: `CurrentCombCount <= 0`이면 SpawnAmount를 0으로 보고 모든 소비장 actor를 제거/비활성화한다.
  - 옵션 B: `CurrentCombCount <= 0`이면 계산을 건너뛰고 이전 SpawnAmount를 유지한다.
  - 옵션 C: 최소 소비장 수를 1로 강제해 0 상태를 허용하지 않는다.
- 권장 옵션: 옵션 A. 테스트 감소 기능에서 0 상태를 자연스럽게 허용하면서, 시각 상태도 비어 있는 벌통으로 명확히 표현된다.
- 답변: 옵션 A

---

### [질문 8]

1. SpawnAmount 변경 시 TargetBeeCount 초기화 범위
- 질문 내용: SpawnAmount가 재계산될 때 기존 소비장의 `TargetBeeCount`를 항상 SpawnAmount로 초기화할지 확정해야 한다.
- 필요한 이유: 요구사항에는 SpawnAmount 변경 시 TargetBeeCount도 해당 SpawnAmount로 초기화한다고 되어 있다. 다만 추후 소비장을 터는 기능과 결합되면 기존 감소 상태 보존 여부가 gameplay 의미를 바꿀 수 있다.
- 선택지
  - 옵션 A: SpawnAmount가 변경되는 모든 경로에서 active 소비장의 TargetBeeCount를 SpawnAmount로 초기화한다.
  - 옵션 B: MaxCombCount/CurrentCombCount 변경 시에는 초기화하고, 벌 수 변경 시에는 비율 보존한다.
  - 옵션 C: TargetBeeCount 감소 상태를 항상 보존하고 SpawnAmount만 갱신한다.
- 권장 옵션: 옵션 A. 현재 명시 요구사항과 일치하며, 초기 기능의 상태 모델이 가장 단순하다.
- 답변: 옵션 A

---

### [질문 9]

1. TargetBeeCount 감소 API 형태
- 질문 내용: 소비장 actor가 외부에서 전달받는 감소 요청을 비율 기준으로 받을지, 수량 기준으로 받을지, 둘 다 제공할지 확정해야 한다.
- 필요한 이유: 현재 요구사항은 벌통의 `n1` 비율 기반 감소를 말하지만, 추후 소비장을 터는 기능에서는 고정 수량 감소가 필요할 수 있다.
- 선택지
  - 옵션 A: `ReduceTargetBeeCountByRatio(float Ratio)`와 `ReduceTargetBeeCountByAmount(int32 Amount)`를 둘 다 제공한다.
  - 옵션 B: 비율 기반 API만 제공한다.
  - 옵션 C: 수량 기반 API만 제공하고 벌통이 비율을 수량으로 변환한다.
- 권장 옵션: 옵션 A. 소비장은 `n1`을 보유하지 않으면서도 외부 정책 변화에 대응할 수 있고, clamp 책임을 소비장 내부에 둘 수 있다.
- 답변: 옵션 A

---

### [질문 10]

1. PlaneSize source of truth 위치
- 질문 내용: `PlaneSize`를 소비장 actor별 값으로 둘지, 벌통이 모든 소비장에 일괄 적용하는 값으로 둘지 확정해야 한다.
- 필요한 이유: 요구사항은 소비장 액터 단계 노출값으로 PlaneSize를 명시했다. 하지만 벌통이 여러 소비장을 생성하면 소비장마다 값이 달라질 수 있는지, 벌통 전체에서 통일할지 결정이 필요하다.
- 선택지
  - 옵션 A: `ABeehiveCombActor`가 `PlaneSize`를 소유하고, 벌통은 건드리지 않는다.
  - 옵션 B: `ABeehive`가 공통 `CombPlaneSize`를 소유하고 모든 소비장에 주입한다.
  - 옵션 C: 기본값은 소비장 actor가 갖고, 벌통에 override 옵션을 둔다.
- 권장 옵션: 옵션 A. 사용자가 "소비장 액터에서는 PlaneSize를 디테일창에 노출"한다고 명시했으므로 소비장 자체 source of truth로 두는 것이 가장 직접적이다.
- 답변: 옵션 B. 최초 프롬프트 내용보다 옵션B를 우선시.

---

### [질문 11]

1. 소비장 Niagara System asset 지정 방식
- 질문 내용: `FrontFaceBeeNiagara`와 `BackFaceBeeNiagara`가 사용할 Niagara System asset을 어디서 지정할지 확정해야 한다.
- 필요한 이유: C++에서 Content asset path를 하드코딩하면 Content 의존이 생긴다. BP에서 지정하면 C++ 경계가 안전하지만 수동 에디터 작업이 필요하다.
- 선택지
  - 옵션 A: `ABeehiveCombActor` BP child에서 두 NiagaraComponent에 Niagara System asset을 직접 지정한다.
  - 옵션 B: `ABeehiveCombActor`에 `UNiagaraSystem*` property를 노출하고 C++에서 두 component에 적용한다.
  - 옵션 C: `ABeehive`가 Niagara System asset을 소유하고 생성된 소비장에 주입한다.
- 권장 옵션: 옵션 A. 기존 Beehive Niagara asset 지정 정책과 일관되고 C++ Content path 의존을 피할 수 있다.
- 답변: 옵션 A

---

### [질문 12]

1. 소비장 Niagara User Parameter 숨김 방식
- 질문 내용: 소비장 하위 `FrontFaceBeeNiagara`, `BackFaceBeeNiagara`의 User Parameter UI를 어떻게 숨길지 확정해야 한다.
- 필요한 이유: 요구사항은 NiagaraComponent User Parameter 직접 수정을 금지한다. 기존 프로젝트에는 NiagaraComponent details customization으로 특정 컴포넌트의 User Parameter UI를 숨기는 선례가 있다.
- 선택지
  - 옵션 A: 기존 NiagaraComponent details customization을 확장해 owner가 `ABeehiveCombActor`이고 component name이 `FrontFaceBeeNiagara`/`BackFaceBeeNiagara`인 경우 User Parameter UI를 숨긴다. 동시에 적용 경로에서 항상 값을 덮어쓴다.
  - 옵션 B: UI 숨김은 하지 않고, `OnConstruction`/`BeginPlay`/`PostEditChangeProperty`에서 항상 덮어쓴다.
  - 옵션 C: NiagaraComponent property 자체를 숨기고 Niagara System asset은 별도 property로만 지정한다.
- 권장 옵션: 옵션 A. 요구사항의 "노출 금지"에 가장 가깝고, 숨김이 완전하지 않은 경우에도 source-of-truth 값을 강제할 수 있다.
- 답변: 옵션 A

---

### [질문 13]

1. MaxCombCount 변경 시 CurrentCombCount 초기화 정책
- 질문 내용: `MaxCombCount`가 변경될 때 `CurrentCombCount`를 항상 `MaxCombCount`로 초기화할지 확정해야 한다.
- 필요한 이유: 요구사항은 MaxCombCount 변경 시 CurrentCombCount를 MaxCombCount로 초기화한다고 명시한다. 다만 테스트로 줄인 상태를 유지해야 하는지 여부와 충돌할 수 있다.
- 선택지
  - 옵션 A: `MaxCombCount` 변경 시 항상 `CurrentCombCount = MaxCombCount`로 초기화한다.
  - 옵션 B: `CurrentCombCount = Clamp(CurrentCombCount, 0, MaxCombCount)`만 수행한다.
  - 옵션 C: 에디터에서는 초기화하고 런타임에서는 clamp만 수행한다.
- 권장 옵션: 옵션 A. 명시 요구사항과 일치하며, MaxCombCount를 벌통의 현재 완전 구성 상태로 보는 모델이 단순하다.
- 답변: 옵션 B

---

### [질문 14]

1. 소비장 테스트 기능의 에디터/런타임 노출 범위
- 질문 내용: CurrentCombCount 증감 테스트 기능을 CallInEditor로 제공할지, BlueprintCallable runtime API로만 둘지 확정해야 한다.
- 필요한 이유: "현재 소비장 수 테스트"가 에디터 배치 검증용인지, PIE/runtime 검증용인지에 따라 노출 방식이 달라진다.
- 선택지
  - 옵션 A: `CallInEditor`와 `BlueprintCallable`을 모두 제공한다.
  - 옵션 B: `CallInEditor`만 제공한다.
  - 옵션 C: `BlueprintCallable`만 제공한다.
- 권장 옵션: 옵션 A. 에디터 배치 확인과 PIE 상호작용 테스트를 모두 지원할 수 있다.
- 답변: 옵션 A

---

### [질문 15]

1. 소비장 기능과 기존 Beehive swarm들의 관계
- 질문 내용: 신규 소비장 양면 Niagara가 기존 `AttractionSwarmNiagara` 및 outgoing/ingoing spline swarm과 동시에 동작할지 확정해야 한다.
- 필요한 이유: 동시에 동작하면 벌통 주변/내부 벌 밀도가 증가한다. 특정 상태에서 대체하려면 기존 swarm 활성/비활성 정책이 추가로 필요하다.
- 선택지
  - 옵션 A: 소비장 양면 Niagara는 기존 swarm들과 항상 동시에 동작한다.
  - 옵션 B: 소비장이 활성화되면 `AttractionSwarmNiagara`를 끈다.
  - 옵션 C: 벌통 lid/open 상태나 gameplay 상태에 따라 선택적으로 동작한다.
- 권장 옵션: 옵션 A. 현재 요구에는 대체/상태 전환 조건이 없으므로, 소비장 내부 벌 표현을 기존 외부/구심점 swarm에 더하는 구조가 가장 단순하다.
- 답변: 옵션 A
