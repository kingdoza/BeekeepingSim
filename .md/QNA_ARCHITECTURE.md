### [질문 1]

1. 아이템 내구도 데이터 모델
- 질문 내용: 현재 `UItemInstance` 에 `Durability` 값은 존재하지만, 이 값이 0~1 비율인지 실제 현재 내구도 수치인지 확정이 필요하다. 또한 내구도가 없는 아이템을 어떻게 구분할지도 정해야 한다.
- 필요한 이유: `ItemVisualWidget` 이 내구도 바를 표시할지 여부와 표시 비율을 계산하려면 내구도 사용 여부, 현재값, 최대값의 의미가 명확해야 한다.
- 선택지
  - 옵션 A: `Durability` 를 0~1 비율로 사용한다. 내구도 표시 여부만 별도 bool 또는 definition 설정으로 구분한다.
  - 옵션 B: `Durability` 를 현재 내구도 수치로 사용하고, `UItemDefinition` 에 `bUsesDurability`, `MaxDurability` 를 추가한다.
  - 옵션 C: 내구도 시스템은 아직 확정하지 않고, UI 에서는 `HasDurability()` 가 false 인 것으로 처리한다.

[답변]
옵션 B

---

### [질문 2]

1. 드래그 중 ItemVisualWidget 처리 방식
- 질문 내용: 드래그 시작 시 슬롯 내부의 `ItemVisualWidget` 을 실제로 슬롯에서 detach 해서 커서 visual 로 재사용할지, 아니면 슬롯 visual 은 숨기고 drag visual widget 을 별도로 생성할지 확정이 필요하다.
- 필요한 이유: 실제 UMG reparent 방식은 레이아웃/상태 꼬임 가능성이 있고, 별도 생성 방식은 안정적이지만 “탈착” 구조와는 다르다.
- 선택지
  - 옵션 A: 슬롯 내부 `ItemVisualWidget` 을 실제로 `RemoveFromParent` 후 viewport 에 붙여 drag visual 로 재사용한다.
  - 옵션 B: 슬롯 내부 `ItemVisualWidget` 은 숨기고, drag visual 용 `ItemVisualWidget` 을 새로 생성한다.
  - 옵션 C: 슬롯 내부 visual 은 그대로 두고 drag visual 만 별도로 생성한다.

[답변]
옵션 B

---

### [질문 3]

1. RMB 부분 이동 시 다른 아이템이 있는 target 처리
- 질문 내용: RMB 부분 이동 중 target 슬롯에 다른 아이템이 이미 있을 때 어떻게 처리할지 확정이 필요하다.
- 필요한 이유: 전체 이동과 달리 부분 이동은 stack split/merge 의미가 있으므로, 다른 아이템과 swap 을 허용하면 UX 와 구현 규칙이 복잡해진다.
- 선택지
  - 옵션 A: target 슬롯에 다른 아이템이 있으면 실패 처리한다.
  - 옵션 B: 전체 이동처럼 다른 아이템과 교환한다.
  - 옵션 C: 같은 아이템이면 merge 하고, 다른 아이템이면 실패 처리한다.

[답변]
옵션 C

---

### [질문 4]

1. RMB 부분 이동 중 같은 아이템 stack merge 초과 처리
- 질문 내용: 부분 이동할 수량이 target 의 같은 아이템 stack 여유 공간보다 클 때 어떻게 처리할지 확정이 필요하다.
- 필요한 이유: `MaxStack` 한도를 넘는 경우 일부만 이동할지, 전체 실패할지에 따라 컨테이너 API 결과와 UI 피드백이 달라진다.
- 선택지
  - 옵션 A: 가능한 만큼만 merge 하고 남은 수량은 source 에 남긴다.
  - 옵션 B: 요청 수량 전체가 들어갈 수 없으면 실패 처리한다.
  - 옵션 C: 가능한 만큼 merge 하고 남은 수량은 target container 의 빈 슬롯에 추가 배치한다.

[답변]
옵션 C

---

### [질문 5]

1. 드래그 중 마우스휠 수량 조절 입력 처리 위치
- 질문 내용: RMB 부분 이동 중 마우스휠로 이동 수량을 조절할 때, wheel input 을 어느 객체가 받을지 확정이 필요하다.
- 필요한 이유: UMG drag/drop 중 wheel 이벤트가 drag visual, source slot, root widget, player controller 중 어디에 안정적으로 도달하는지에 따라 구현 구조가 달라진다.
- 선택지
  - 옵션 A: drag visual widget 이 `OnMouseWheel` 을 처리한다.
  - 옵션 B: inventory/storage/root UI 가 active drag operation 을 보관하고 wheel 을 처리한다.
  - 옵션 C: PlayerController 또는 HUD 계층에서 active drag operation 을 보관하고 wheel 을 처리한다.

[답변]
옵션 C

---

### [질문 6]

1. LMB 더블클릭 quick move 의 target storage 결정 방식
- 질문 내용: Hotbar UI 는 StorageBox 와 무관하게 항상 표시되는 독립 UI 이므로, hotbar 슬롯 더블클릭 시 어느 storage 로 아이템을 보낼지 결정 방식이 필요하다.
- 필요한 이유: hotbar 가 `UStorageBoxWidget` 에 의존하면 안 되지만, quick move 는 현재 열린 storage context 를 알아야 한다.
- 선택지
  - 옵션 A: `UStorageBoxFocusActionComponent` 가 열릴 때 캐릭터 또는 컨트롤러에 active storage component 를 등록하고, 닫힐 때 해제한다.
  - 옵션 B: `WBP_Hotbar` 가 optional active storage component 를 외부에서 주입받는다.
  - 옵션 C: StorageBox UI 가 열렸을 때만 별도의 storage 전용 hotbar UI 를 사용한다.

[답변]
옵션 A

---

### [질문 7]

1. 더블클릭 판정 구현 위치
- 질문 내용: LMB 더블클릭 quick move 를 C++ slot widget native event 로 처리할지, Blueprint event 로 처리할지, 기존 mouse down 에서 직접 click interval 을 계산할지 확정이 필요하다.
- 필요한 이유: 슬롯 입력이 LMB drag, RMB drag, wheel quantity, double click 까지 확장되므로 입력 책임 위치를 정해야 유지보수가 가능하다.
- 선택지
  - 옵션 A: `UItemSlotWidget` C++ base 에서 `NativeOnMouseButtonDoubleClick` 으로 처리한다.
  - 옵션 B: `WBP_ItemSlot` Blueprint 의 `OnMouseButtonDoubleClick` 에서 처리한다.
  - 옵션 C: 기존 `OnMouseButtonDown` 에서 click time 을 직접 계산한다.

[답변]
옵션 A

---

### [질문 8]

1. Hotbar 내부 swap 후 선택 인덱스 정책
- 질문 내용: hotbar 내부 slot swap 후 `SelectedIndex` 를 슬롯 위치 기준으로 유지할지, 아이템을 따라 이동시킬지, 선택을 해제할지 확정이 필요하다.
- 필요한 이유: Hotbar 는 현재 `SelectedIndex` 하나로 선택 상태를 관리한다. swap 후 선택 의미가 슬롯 기준인지 아이템 기준인지에 따라 UI 와 held visual 결과가 달라진다.
- 선택지
  - 옵션 A: 선택은 슬롯 위치 기준으로 유지한다.
  - 옵션 B: 선택은 아이템을 따라 이동한다.
  - 옵션 C: swap 후 선택을 해제한다.

[답변]
옵션 A

---

### [질문 9]

1. 부분 이동으로 생성되는 새 `UItemInstance` outer
- 질문 내용: stack split 으로 새 `UItemInstance` 를 생성해야 할 때 outer 를 어디로 둘지 확정이 필요하다.
- 필요한 이유: 현재 획득 아이템은 hotbar component 를 outer 로 생성된다. storage 로 부분 이동하거나 storage 에서 hotbar 로 부분 이동할 때 새 item instance 의 lifecycle/GC 소유자가 명확해야 한다.
- 선택지
  - 옵션 A: 새 item instance 를 target container component outer 로 생성한다.
  - 옵션 B: source item 과 같은 outer 로 생성한다.
  - 옵션 C: character 또는 별도 inventory owner 를 outer 로 사용한다.

[답변]
옵션 A

---

### [질문 10]

1. StorageFocus Engaged 중 hotbar 선택 입력 정책
- 질문 내용: `StorageBoxFocusActionComponent` 로 StorageFocus 에 진입했을 때 기존 선택 hotbar 슬롯을 유지할지, 그리고 engaged 상태에서 숫자키/마우스휠 hotbar 선택 입력을 허용할지 확정이 필요하다.
- 필요한 이유: Storage UI 에서는 마우스휠이 RMB 부분 이동 수량 조절 입력으로 전환된다. 같은 입력이 hotbar 선택 순환까지 동시에 수행되면 UI 조작 결과가 충돌한다.
- 선택지
  - 옵션 A: StorageFocus 진입 시 hotbar 선택을 해제하고, StorageFocus Engaged 동안 숫자키/마우스휠 hotbar 선택 입력을 비활성화한다.
  - 옵션 B: StorageFocus 진입 시 선택은 유지하되, StorageFocus Engaged 동안 숫자키/마우스휠 hotbar 선택 입력만 비활성화한다.
  - 옵션 C: StorageFocus 에서도 기존 hotbar 선택/휠 순환 입력을 그대로 유지한다.

[답변]
옵션 A

---

### [질문 11]

1. RMB 드래그 중 source slot stack count 감소 처리 방식
- 질문 내용: RMB 부분 드래그 중 이동 예정 수량이 증가할 때 source slot 의 StackCount 가 줄어든 것처럼 보여야 한다. 이 감소를 실제 `UItemInstance::StackCount` 에 즉시 반영할지, 아니면 drag 중 UI preview 로만 표시할지 확정이 필요하다.
- 필요한 이유: 실제 데이터를 드래그 중 변경하면 drop 실패/cancel/ESC/위젯 제거 시 rollback 처리가 필요하다. 반면 UI preview 방식이면 실제 데이터는 drop 성공 시점에만 변경되고, drag visual/source slot visual 만 즉시 갱신하면 된다.
- 선택지
  - 옵션 A: UI preview 로만 처리한다. 드래그 중 source slot 은 `OriginalStackCount - MoveQuantity` 를 표시하고, 실제 `UItemInstance::StackCount` 는 drop 성공 시에만 변경한다.
  - 옵션 B: 드래그 중 실제 source item stack count 를 즉시 감소시킨다. drop 실패/cancel 시 원복한다.
  - 옵션 C: source slot 은 LMB/RMB 모두 드래그 중 완전히 숨기고, 남은 수량 preview 는 표시하지 않는다.

[답변]
옵션 A
