# Architecture QnA

## 확정 전제

- 기존 QnA 내용은 사용자 요청에 따라 제거했다.
- 밀도 작업대는 C++ native WorldActor로 구현한다.
- 밀도 작업대는 전역 FocusConfirm 경로로 FocusEngaged 작업 상태에 진입한다.
- 밀도 작업대는 소비장 슬롯 1개를 가진다.
- 밀도질은 흰색 밀랍/capping layer를 원형 지우개 브러시처럼 부분 제거해, 제거된 부분 아래의 꿀 plane이 보이게 하는 hold-use 기능이다.

## 미해결 질문

### [밀도 작업대 슬롯]

1. 소비장 슬롯 class 구현 경로
- 질문 내용: 밀도 작업대의 단일 소비장 슬롯을 어떤 C++ class로 구현할 것인가?
- 필요한 이유: 기존 `ABeehiveCombSlotActor`는 소비장 class 검증과 item state 적용을 제공하지만, owning `ABeehive` refresh를 요청하는 벌통 전용 동작을 포함한다. 밀도 작업대 슬롯은 벌통 refresh 없이 작업대 item-use-area/part focus 갱신만 수행해야 한다.
- 선택지
  - 옵션 A: `ABeehiveCombSlotActor`를 그대로 재사용한다.
  - 옵션 B: `AUncappingWorkbenchCombSlotActor` 같은 작업대 전용 slot subclass를 추가한다.
  - 옵션 C: 공통 `ACombPlacementSlotActor` base를 새로 만들고, 벌통 슬롯과 작업대 슬롯이 각각 상속한다.
- 권장 옵션: 옵션 B. 현재 범위에서는 기존 벌통 슬롯을 건드리지 않고 작업대 전용 동작만 캡슐화하는 편이 가장 낮은 위험이다. 공통화는 중복이 커진 뒤 별도 리팩토링으로 판단한다.
- 답변 : 옵션B

2. 작업대 슬롯 배치/회수 입력 정책
- 질문 내용: FocusEngaged 작업대 안에서 소비장 배치와 회수를 어떤 입력 경로로 처리할 것인가?
- 필요한 이유: 현재 item-use-area LMB는 선택 아이템의 hold/placement action에 사용되고, occupied actor 회수는 PartFocus secondary 경로를 사용한다. 작업대에서도 같은 규칙을 쓸지 확정해야 prompt와 입력 충돌을 피할 수 있다.
- 선택지
  - 옵션 A: empty slot 배치는 기존 `UItemPlacementUseAction` + item-use-area LMB 경로를 사용하고, occupied comb 회수는 PartFocus secondary retrieve 경로를 사용한다.
  - 옵션 B: LMB로 empty/occupied 상태에 따라 배치와 회수를 모두 토글한다.
  - 옵션 C: 작업대 전용 FocusAction이 배치/회수를 직접 처리한다.
- 권장 옵션: 옵션 A. 기존 Focus/Inventory/WorldActors 책임 경계를 유지하고, hotbar acquire dry-run 기반 prompt availability도 재사용할 수 있다.
- 답변 : 옵션A

3. FocusEngaged 진입 시 hotbar 선택 정책
- 질문 내용: 밀도 작업대 FocusEngaged 진입 시 기존 anchored cursor 정책처럼 hotbar 선택을 비울 것인가?
- 필요한 이유: 현재 FocusEngaged anchored cursor 진입은 hotbar 선택을 빈손으로 전환하고, 진입 후 필요한 아이템을 다시 선택하는 정책이다. 작업대에서 소비장 배치나 밀도 선택 UX가 이 정책과 충돌할 수 있다.
- 선택지
  - 옵션 A: 기존 anchored cursor 정책을 그대로 따른다. 진입 시 선택을 비우고, 진입 후 소비장/밀도를 다시 선택한다.
  - 옵션 B: 작업대만 예외로 진입 전 선택 아이템을 유지한다.
  - 옵션 C: 선택 아이템이 소비장 또는 밀도일 때만 유지하고 그 외에는 비운다.
- 권장 옵션: 옵션 A. FocusEngaged item-use-area의 기존 invariant를 유지한다. 예외가 필요하면 작업대 UX 검증 후 별도 정책으로 추가한다.
- 답변 : 옵션A

### [밀도질 대상과 face 정책]

1. 밀도질 대상 조건
- 질문 내용: 어떤 소비장 상태에서 밀도질 use-area를 active로 볼 것인가?
- 필요한 이유: 현재 밀랍/capping plane 표시는 `IsHoneyFull()`에서 파생된다. 부분 밀도질이 추가되면 full honey 여부, 남은 capping 여부, 숙성도 조건을 어떻게 조합할지 정해야 한다.
- 선택지
  - 옵션 A: `IsHoneyFull()`이고 해당 face에 남은 capping mask가 있을 때만 active다.
  - 옵션 B: honey fill ratio가 일정 임계값 이상이면 active다.
  - 옵션 C: honey 양과 무관하게 항상 active다.
- 권장 옵션: 옵션 A. 기존 capping 표시 조건과 일치하며, 꿀이 full이 아닌 소비장을 밀도질하는 모순을 피한다.
- 답변 : 옵션A

2. 밀도질 face 선택과 뒤집기 경로
- 질문 내용: 밀도질은 현재 보이는 face만 대상으로 할 것인가, 그리고 작업대에서 앞/뒤 face를 어떻게 전환할 것인가?
- 필요한 이유: 밀도질은 cursor hit 위치와 face별 capping mask가 맞아야 한다. 양면 소비장을 처리하려면 작업대에서 소비장을 뒤집는 C++ 상호작용이 필요하다.
- 선택지
  - 옵션 A: 현재 visible face만 밀도질 가능하며, 작업대 안에서 C++ PartFocus/preview key 경로로 소비장 flip을 제공한다.
  - 옵션 B: front/back capping use-area를 모두 active로 두고 hit된 plane 기준으로 face를 결정한다.
  - 옵션 C: 1차 구현은 front face만 지원한다.
- 권장 옵션: 옵션 A. 기존 `VisibleCombFace` 상태와 회수 state 계약을 활용할 수 있고, 사용자가 보는 면과 수정되는 mask가 일치한다.
- 답변 : 옵션D. 현재 visible face만 밀도질 가능하며, 벌통 소비장 뒤집기와 같은 horizontal drag flip UX로 밀도 작업대의 소비장 뒤집기를 수행한다. 단, 기존 `UBeehiveCombPartFocusActionComponent`를 직접 재사용하지 않고 작업대 전용 PartFocus action에서 flip UX만 복제한다.

3. 작업대의 소비장 PartFocus 기능 범위
- 질문 내용: 작업대에 놓인 소비장도 벌통 안 소비장처럼 PartFocus primary/preview key 동작을 제공할 것인가?
- 필요한 이유: 기존 `UBeehiveCombPartFocusActionComponent`는 벌통의 lift/drag 정책과 결합되어 있다. 작업대에서는 lift 대신 flip/회수/검사 같은 별도 동작이 필요할 수 있다.
- 선택지
  - 옵션 A: 작업대 전용 comb PartFocus action component를 추가해 flip과 retrieve prompt를 제공한다.
  - 옵션 B: 기존 `UBeehiveCombPartFocusActionComponent`를 재사용한다.
  - 옵션 C: 작업대에서는 PartFocus를 쓰지 않고 item-use-area만 제공한다.
  - 옵션 D: 짧은 이름의 작업대 전용 action(`UCombUncappingPartFocusActionComponent`)을 추가하고, 기존 벌통 소비장 action의 horizontal drag flip UX만 복제한다. 벌통 전용 lift/shake/tag/prompt 정책은 가져오지 않는다.
- 권장 옵션: 옵션 A. 벌통의 lift 정책과 작업대의 face 조작 정책을 분리한다.
- 답변 : 옵션D. `UCombUncappingPartFocusActionComponent`를 추가한다. 기존 `UBeehiveCombPartFocusActionComponent`는 직접 재사용하지 않고, 같은 horizontal drag flip UX만 작업대 전용 action 안에 복제한다.

### [밀랍 mask 상태]

1. mask 상태 보존 범위
- 질문 내용: 밀도질로 제거된 capping mask를 소비장 회수/재배치 후에도 보존할 것인가?
- 필요한 이유: 작업대에서 일부만 밀도질한 소비장을 hotbar로 회수한 뒤 다시 배치하면, 꿀 양/숙성도처럼 capping 제거 상태도 유지되어야 자연스럽다. 이를 보존하려면 `FBeehiveCombItemState` 확장이 필요하다.
- 선택지
  - 옵션 A: front/back capping mask를 `FBeehiveCombItemState`에 저장해 회수/재배치까지 보존한다.
  - 옵션 B: 런타임 actor에만 저장하고 회수 시 제거 상태를 버린다.
  - 옵션 C: 전체 remaining ratio만 저장하고 원형 제거 모양은 버린다.
- 권장 옵션: 옵션 A. 사용자가 만든 원형 제거 결과를 gameplay state로 인정하는 모델이다.
- 답변 : 옵션A

2. mask 해상도와 plane aspect 처리
- 질문 내용: face별 capping mask 해상도와 소비장 plane 비율을 어떤 방식으로 처리할 것인가?
- 필요한 이유: wax plane이 1:1 비율이 아니면 square mask에서 UV 거리로 원을 찍을 때 월드상 타원으로 보일 수 있다. 브러시는 실제 작업면 기준 원형이어야 한다.
- 선택지
  - 옵션 A: `CombPlaneSize` 비율에 맞춰 mask width/height를 산출하고, 브러시 판정은 plane local 거리 기준으로 계산한다.
  - 옵션 B: 고정 square mask를 쓰고 UV 거리 기준 원형 stamp를 찍는다.
  - 옵션 C: 고정 square mask를 쓰되 aspect 보정을 수동 authoring 값으로 둔다.
- 권장 옵션: 옵션 A. plane 비율과 무관하게 월드/로컬 공간에서 원형 브러시가 유지된다.
- 답변 : 옵션A

3. mask 데이터 표현
- 질문 내용: capping mask gameplay state를 어떤 데이터 구조로 보관할 것인가?
- 필요한 이유: mask는 visual texture이면서 동시에 회수 state로 저장되어야 한다. UObject texture만 저장하면 item instance state로 직렬화/복사가 어렵다.
- 선택지
  - 옵션 A: face별 `TArray<uint8>` mask byte buffer를 source of truth로 두고, runtime texture는 이 buffer에서 재생성한다.
  - 옵션 B: `UTexture2D` 또는 `UTextureRenderTarget2D`를 source of truth로 둔다.
  - 옵션 C: 제거된 원형 stamp 목록을 source of truth로 둔다.
- 권장 옵션: 옵션 A. item state 저장과 runtime material 반영 경계를 가장 명확히 분리할 수 있다.
- 답변 : 옵션A

4. capping visual 반영 방식
- 질문 내용: `FrontWaxCappingPlane`/`BackWaxCappingPlane` material에 mask를 어떻게 주입할 것인가?
- 필요한 이유: 현재 capping plane material에는 `HoneyRipeness` scalar만 주입된다. 부분 제거 표현에는 alpha/opacity mask texture parameter가 필요하다.
- 선택지
  - 옵션 A: C++이 transient `UTexture2D` mask를 생성/갱신하고 material parameter `WaxCappingMask`에 주입한다.
  - 옵션 B: `UTextureRenderTarget2D`에 brush draw material을 그려 material에 주입한다.
  - 옵션 C: material scalar parameter만으로 remaining ratio를 표현한다.
- 권장 옵션: 옵션 A. mask byte buffer를 source of truth로 유지하면서 구현과 저장 경계가 단순하다.
- 답변 : 옵션A

### [밀도질 action과 입력 context]

1. cursor hit 정보 전달 경로
- 질문 내용: 밀도질 action이 커서 중심을 알기 위해 hit 정보를 어떤 경로로 받을 것인가?
- 필요한 이유: 현재 `FItemActionContext`에는 hit component와 effect target만 있고, impact point/normal/UV가 없다. 원형 브러시 stamp에는 작업면 좌표가 필요하다.
- 선택지
  - 옵션 A: `FItemActionContext`에 item-use-area hit 결과 필드(`bHasItemUseAreaHit`, `ItemUseAreaImpactPoint`, `ItemUseAreaImpactNormal`)를 추가하고 scope가 채운다.
  - 옵션 B: `UUncappingKnifeUseAction`이 매 Tick 직접 mouse deproject와 line trace를 다시 수행한다.
  - 옵션 C: `FItemActionContext`에는 UV만 추가한다.
- 권장 옵션: 옵션 A. trace owner는 Focus scope로 유지하고, action은 전달받은 context로 domain mutation만 수행한다.
- 답변 : 옵션A

2. 밀도질 도구 action class와 tag
- 질문 내용: 밀도 아이템의 hold-use action과 item-use-area tag 이름을 어떻게 정할 것인가?
- 필요한 이유: tag 이름은 DataAsset, use-area mesh, action query가 공유하는 Blueprint/API 계약이 된다. 나중에 rename하면 asset migration이 필요하다.
- 선택지
  - 옵션 A: `UUncappingKnifeUseAction`, tag `Item.UseArea.Beehive.CombCapping`
  - 옵션 B: `UCombUncappingUseAction`, tag `Item.UseArea.Comb.Capping`
  - 옵션 C: `UWaxCappingRemovalUseAction`, tag `Item.UseArea.WaxCapping`
- 권장 옵션: 옵션 B. 벌통 전용이 아니라 작업대/소비장 기능이므로 `Beehive`를 tag에 넣지 않는 편이 낫다.
- 답변 : 옵션D. `UCombUncappingUseAction`, tag `Item.UseArea.UncappingTable.Comb`

3. brush 성공 판정과 내구도 감소 조건
- 질문 내용: LMB hold 중 언제 `FItemActionExecutionResult::bSucceeded=true`로 볼 것인가?
- 필요한 이유: active-use durability drain의 기본 정책은 effect success와 연결될 수 있다. 이미 제거된 영역을 문지를 때도 내구도가 닳는지 결정해야 한다.
- 선택지
  - 옵션 A: 이번 Tick에서 실제 mask pixel이 하나 이상 감소했을 때만 success다.
  - 옵션 B: valid capping use-area 위에 있으면 mask 변화가 없어도 success다.
  - 옵션 C: LMB use session이 active면 항상 success다.
  - 옵션 D: 밀도 도구 내구도 기능은 이번 범위에서 구현하지 않는다. `bSucceeded=true`는 실제 mask pixel이 하나 이상 제거된 Tick에만 반환하고, 이미 제거된 부분을 문질러 mask 변화가 없으면 `bSucceeded=false`를 반환한다.
- 답변 : 옵션 D. 내구도 기능은 일단 구현하지 않는다. `bSucceeded=true`는 실제 mask pixel이 하나 이상 제거된 Tick에만 반환한다. 이미 다 지워진 부분을 문질러 mask 변화가 없으면 `bSucceeded=false`다.

4. brush 속도와 stamp 간격
- 질문 내용: 밀도질 brush는 매 Tick stamp를 찍을 것인가, 이동 거리/시간 기준으로 rate limit을 둘 것인가?
- 필요한 이유: 프레임레이트와 마우스 이동 속도에 따라 제거량이 달라지면 조작감과 내구도 소모가 불안정해질 수 있다.
- 선택지
  - 옵션 A: `MinStampInterval`과 `MinStampDistanceCm`를 두고, 조건을 만족할 때만 stamp를 찍는다.
  - 옵션 B: 매 Tick 현재 위치에 stamp를 찍는다.
  - 옵션 C: cursor 이동 경로를 선분으로 보간해 여러 stamp를 찍는다.
- 권장 옵션: 옵션 A. 1차 구현에서 안정적인 빈도 제어가 가능하다. 빠른 이동 시 끊김이 눈에 띄면 옵션 C를 후속 개선으로 검토한다.
- 답변 : 옵션A

### [완료 상태와 후속 시스템 경계]

1. 밀도 완료 판정
- 질문 내용: face 또는 소비장 전체를 "밀도 완료"로 보는 기준은 무엇인가?
- 필요한 이유: capping plane 숨김, use-area 비활성화, 향후 채밀/검사 시스템의 선행 조건으로 같은 기준을 써야 한다.
- 선택지
  - 옵션 A: face별 remaining mask ratio가 `UncappedThreshold` 이하이면 해당 face 완료다. 양면 모두 완료되어야 소비장 전체 완료다.
  - 옵션 B: 한 면만 완료되면 소비장 전체 완료다.
  - 옵션 C: 완료 상태를 따로 두지 않고 visual mask만 관리한다.
- 권장 옵션: 옵션 A. 양면 소비장 모델과 future extraction 조건에 가장 명확하다.
- 답변 : 옵션A

2. 채밀/수확 시스템 연동 범위
- 질문 내용: 이번 구현에서 밀도 완료 후 채밀 또는 수확 가능 상태까지 연결할 것인가?
- 필요한 이유: 밀도질은 꿀을 보이게 하지만 꿀 양을 줄이는 기능은 아니다. 채밀까지 함께 구현하면 새로운 item action, 작업대 또는 extractor actor, inventory output 계약이 추가된다.
- 선택지
  - 옵션 A: 이번 범위는 밀도 작업대와 밀도질 mask 제거까지만 구현하고, 채밀/수확은 후속 QnA로 분리한다.
  - 옵션 B: 밀도 완료 즉시 꿀 아이템 생산 또는 honey amount 감소까지 구현한다.
  - 옵션 C: 밀도 완료 상태만 저장하고 prompt에 후속 작업 disabled entry를 표시한다.
- 권장 옵션: 옵션 A. 현재 요구의 기능 경계를 넘지 않고, WorldActors/Inventory 계약 확장을 최소화한다.
- 답변 : 옵션A

3. Content/Editor 수동 작업 범위
- 질문 내용: 밀도질 시각 효과 구현을 위해 어떤 Content/Editor 수동 작업을 허용할 것인가?
- 필요한 이유: C++에서 component와 mask texture는 만들 수 있지만, capping material이 `WaxCappingMask` texture parameter와 opacity/opacity mask 처리를 지원해야 실제 부분 제거가 보인다.
- 선택지
  - 옵션 A: C++은 component/API/state를 구현하고, `BP_BeehiveComb` capping plane material 설정과 material graph의 `WaxCappingMask` parameter 연결은 수동 Content 작업으로 둔다.
  - 옵션 B: C++ default material까지 생성/지정하려고 한다.
  - 옵션 C: material 변경 없이 C++ visibility만으로 표현한다.
- 권장 옵션: 옵션 A. Unreal asset authoring 경계와 현재 Content 취급 규칙에 맞는다.
- 답변 : 옵션A
