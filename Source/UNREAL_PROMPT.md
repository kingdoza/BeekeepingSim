다음은 현재 C++ 구현 기준으로, Unreal Editor 내장 AI 어시스턴트에게 그대로 붙여넣어 사용할 수 있는 프롬프트다.

---

현재 프로젝트 상황:
- `ABeekeeperCharacter`는 `UBeekeeperHotbarComponent`를 소유한다.
- `UBeekeeperHotbarComponent`는 8칸 고정 슬롯 핫바를 관리한다.
- 슬롯 선택 상태는 `SelectedIndex` 하나로 관리된다.
- `SelectedIndex == INDEX_NONE`이면 맨손 상태다.
- 일반 상태에서 선택된 슬롯은 `InHand` 표현 모드다.
- `EngagedFocus` 상태에서 선택된 슬롯은 `OnCursor` 표현 모드다.
- `EngagedFocus` 진입 시 선택은 무조건 해제된다.
- 핫바 UI는 C++ 컴포넌트 상태만 읽고, 실제 위젯 표시와 연출은 Blueprint에서 처리한다.
- `BP_BeekeeperCharacter`가 핫바 위젯을 생성하고 소유하는 구조를 유지해도 된다.

내가 원하는 작업:
1. `WBP_BeekeeperHotbar` 같은 8칸 핫바 위젯 Blueprint를 만드는 방법을 단계별로 설명해줘.
2. 이 위젯이 `BP_BeekeeperCharacter`의 `BeekeeperHotbar` 컴포넌트와 연결되도록 설명해줘.
3. 아래 내용을 반드시 포함해줘:
   - 핫바 위젯 생성 위치
   - `BP_BeekeeperCharacter`에서 `Create Widget` / `Add to Viewport` 하는 방법
   - 위젯에 `BeekeeperHotbarComponent` 참조를 넘기는 방법
   - 위젯에서 `GetSlots()`, `GetSelectedIndex()`, `IsSlotEnabled(Index)`, `GetPresentationMode()`를 읽는 방법
   - 슬롯 8개 UI를 만드는 방법
   - 선택 슬롯 하이라이트 처리
   - 비활성 슬롯 시각 처리
   - 빈 슬롯과 선택 상태를 분리해서 표현하는 방법
   - `InHand`, `OnCursor`, `None` 표현 모드를 UI에 반영하는 방법
   - `OnHotbarChanged` 델리게이트에 바인딩해서 위젯을 갱신하는 방법

핵심 구현 조건:
- 핫바는 항상 8칸이다.
- 하이라이트는 아이템 유무가 아니라 `SelectedIndex` 기준이다.
- 빈 슬롯도 선택될 수 있다.
- 비활성 슬롯은 `bIsEnabled == false` 또는 `IsSlotEnabled(Index) == false`로 판단한다.
- 핫바 필터는 `EngagedFocus`에서만 적용된다.
- 위젯은 포커스 대상 내부 구조를 몰라도 되고, `BeekeeperHotbar`만 보면 된다.

원하는 위젯 구조:
- 루트: `Canvas Panel` 또는 적절한 루트
- 하단 중앙에 8칸 핫바 배치
- 각 슬롯은 동일한 하위 슬롯 위젯으로 분리해도 되고, 하나의 위젯 안에서 8개를 직접 만들어도 된다
- 각 슬롯은 최소한 아래 시각 요소를 가진다:
  - 슬롯 배경
  - 아이콘 영역
  - 선택 하이라이트
  - 비활성 오버레이 또는 Tint

설명해줬으면 하는 Blueprint 설계:

### 1. `WBP_BeekeeperHotbar`
- 변수:
  - `HotbarComponentRef` : `BeekeeperHotbarComponent` 참조
- 함수:
  - `RefreshHotbar`
- 역할:
  - 현재 슬롯 8개를 읽고 UI 반영
  - `SelectedIndex` 기준 하이라이트
  - `IsSlotEnabled(Index)` 기준 비활성 처리
  - `GetPresentationMode()`를 읽어 현재 모드 텍스트나 디버그 표시 가능하면 표시

### 2. `WBP_BeekeeperHotbarSlot`를 쓰는 경우
- 변수:
  - `SlotIndex`
  - `bIsSelected`
  - `bIsEnabled`
  - `ItemInstance`
- 함수:
  - `RefreshFromData`
- 역할:
  - 단일 슬롯 시각 갱신

### 3. `BP_BeekeeperCharacter`
- `BeginPlay`에서:
  - `Create Widget`
  - `Promote to Variable`
  - `Add to Viewport`
  - `Get BeekeeperHotbar`
  - 위젯에 `HotbarComponentRef` 전달
  - `OnHotbarChanged`에 바인딩
  - 최초 `RefreshHotbar` 호출

꼭 설명해줬으면 하는 연결 방법:
- `BP_BeekeeperCharacter`에서 `BeekeeperHotbar`를 가져오는 방법
- `Bind Event to OnHotbarChanged` 하는 방법
- 델리게이트 이벤트에서 `RefreshHotbar`를 호출하는 방법
- UI에서 슬롯별로 `Get Slots` 배열을 순회하는 방법
- `SelectedIndex`와 배열 인덱스를 비교해서 선택 테두리를 켜는 방법
- `ItemInstance == None`이면 아이콘은 비우되, 선택 하이라이트는 별도로 유지하는 방법

표현 모드도 UI에서 확인 가능하게 해줘:
- `GetPresentationMode()` 결과가:
  - `None`
  - `InHand`
  - `OnCursor`
  인지에 따라 디버그 텍스트나 상태 표시를 넣는 방법도 함께 설명해줘

특히 노드 이름 기준으로 설명해줘:
- `Create Widget`
- `Add to Viewport`
- `Get Component By Class`
- `Bind Event to OnHotbarChanged`
- `Get Slots`
- `Get Selected Index`
- `Is Slot Enabled`
- `Get Presentation Mode`
- `Set Brush Tint`
- `Set Visibility`
- `SetText`

출력 형식 요구:
1. 먼저 `WBP_BeekeeperHotbar` 생성 절차
2. 그 다음 `WBP_BeekeeperHotbarSlot`를 분리하는 방식과 안 하는 방식 비교
3. 그 다음 `BP_BeekeeperCharacter`에서 위젯 생성/연결 절차
4. 그 다음 `RefreshHotbar` 함수 구현 절차
5. 마지막에 테스트 체크리스트

추가로 같이 설명해줘:
- 현재 구조에서는 핫바 위젯을 `BP_BeekeeperCharacter`가 소유하는 게 괜찮은지
- 아니면 `PlayerController` 또는 HUD로 옮기는 게 더 나은지
- 둘을 비교하되, 현재 프로젝트 구조 기준으로 추천안을 하나 제시해줘

최종 목표:
- `BP_BeekeeperCharacter`가 핫바 위젯을 생성하고
- `BeekeeperHotbar` 상태 변경 시 UI가 자동 갱신되며
- 슬롯 하이라이트, 비활성, 빈 슬롯, 표현 모드(`None/InHand/OnCursor`)가 정상 반영되는 Blueprint 구현 가이드를 받고 싶다
