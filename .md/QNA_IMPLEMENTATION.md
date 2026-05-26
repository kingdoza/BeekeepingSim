### [Beehive Item Use Effects]

1. Hold item-use effect의 stack delta 적용 권한 경로
- 질문 내용
  - `UCursorItemUseAreaScopeComponent::TickComponent()`에서 `CachedHoldAction->ApplyUseEffect(...)`를 호출하지만, 반환되는 `FItemActionExecutionResult`(`bConsumedItem`, `StackDelta`)를 실제 `UItemInstance`/Hotbar stack에 반영하는 경로를 찾지 못했습니다.
  - 현재 구현 기준으로는 화분떡 설치 성공 시 stack 1 감소 요구사항을 충족할 안전한 authority 경로가 없습니다.
- 필요한 이유
  - 프롬프트 요구사항상 stack mutation은 Inventory/Hotbar authority 경로를 따라야 하며, 임의로 `UItemInstance` 내부 수량을 직접 변경하면 시스템 경계를 위반합니다.
  - 잘못된 경로로 mutation하면 UI 표시/동기화/후속 액션 조건과 불일치가 발생할 수 있습니다.
- 선택지
  - 옵션 A: `UCursorItemUseAreaScopeComponent`가 `ApplyUseEffect` 결과를 받아 `OwnerHotbarComponent`의 공식 API(신규 포함)로 stack delta 반영을 위임한다.
  - 옵션 B: `UHoldItemUseAction` 또는 `UItemInstance` 레이어에 "결과 적용(consume/apply stack delta)" 공통 함수를 추가하고 scope는 해당 함수만 호출한다.
  - 옵션 C: 화분떡 action이 `Execute`/`ApplyUseEffect`에서 직접 inventory mutation 권한을 갖도록 한다.
- 권장 옵션
  - 옵션 A (scope는 결과 라우팅만, 실제 mutation은 Hotbar/Inventory authority API가 수행)
