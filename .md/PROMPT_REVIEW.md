# 리뷰 프롬프트: PartFocus 등록 + RMB Placed Item 회수

## 리뷰 목적

이번 리뷰는 아래 기능의 **정합성/회귀/경계 위반 여부**를 검증한다.

- `APlacedItemActor`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `UFocusSecondaryActionComponent` 관련 경로(구경로 잔존 포함)
- FocusEngaged 상태 RMB(secondary)로 배치 아이템 회수하는 경로

중요: 이번 워크트리에는 여러 에이전트의 변경이 섞여 있을 수 있다.  
리뷰는 "최종 코드 상태 기준"으로 수행하고, 변경 주체를 가정하지 않는다.

---

## 핵심 검증 질문

1. PartFocus descriptor 공급/등록이 provider 기반으로 일관되게 동작하는가?
2. placed item이 global focus target이 아니라 host 내부 PartFocus part로만 취급되는가?
3. RMB secondary 입력이 FocusEngaged -> PartFocus hovered action handler로 정확히 라우팅되는가?
4. 회수 성공 판정(`TryAcquireItem(ItemDefinition, 1)` + `AddedQuantity == 1`)이 정확한가?
5. 회수 실패 시 actor/slot 상태 유지, 성공 시 slot clear/descriptor 전환이 보장되는가?
6. 기존/구경로(`UFocusSecondaryActionComponent`, `PlacedItemRetrieveFocusActionComponent`)가 충돌 없이 정리되었는가?

---

## 리뷰 범위 (우선 파일)

### Focus
- `Public/Focus/CursorPartFocusProvider.h`
- `Public/Focus/CursorPartFocusRegistrationComponent.h`
- `Private/Focus/CursorPartFocusRegistrationComponent.cpp`
- `Public/Focus/ChildCursorPartFocusProviderComponent.h`
- `Private/Focus/ChildCursorPartFocusProviderComponent.cpp`
- `Public/Focus/CursorPartFocusScopeComponent.h`
- `Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Public/Focus/CursorPartFocusActionComponent.h`
- `Private/Focus/CursorPartFocusActionComponent.cpp`
- `Public/Focus/FocusActionComponent.h`
- `Private/Focus/FocusActionComponent.cpp`
- `Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Private/Focus/BeekeeperFocusComponent.cpp`

### WorldActors
- `Public/WorldActors/PlacedItemActor.h`
- `Private/WorldActors/PlacedItemActor.cpp`
- `Public/WorldActors/PlacedItemRetrievePartFocusActionComponent.h`
- `Private/WorldActors/PlacedItemRetrievePartFocusActionComponent.cpp`
- `Public/WorldActors/ItemPlacementSlotActor.h`
- `Private/WorldActors/ItemPlacementSlotActor.cpp`
- `Public/WorldActors/Beehive.h`
- `Private/WorldActors/Beehive.cpp`

### Character / Inventory
- `Public/Character/BeekeeperCharacter.h`
- `Private/Character/BeekeeperCharacter.cpp`
- `Public/Inventory/BeekeeperHotbarComponent.h`
- `Private/Inventory/BeekeeperHotbarComponent.cpp`

---

## 상세 체크리스트

### 1) Provider/Registration 구조
- `ICursorPartFocusProvider::GetCursorPartFocusDescriptors` 계약이 명확한지
- `UCursorPartFocusRegistrationComponent`의 gather 순서(actor -> components)가 의도대로인지
- beehive의 기존 직접 등록(lid/comb) + registration append가 clear 타이밍 충돌 없이 동작하는지
- child provider tag/class 필터가 누락/과수집을 유발하지 않는지

### 2) Slot 상태별 descriptor 정책
- empty slot: `ItemUseArea`만 제공
- occupied slot: `PartFocus`만 제공
- `SanitizeAndCheckOccupied()`를 통한 invalid placed actor 정리 후 descriptor 일관성 유지 여부

### 3) PlacedItemActor 책임
- global focus 컴포넌트(`UFocusTargetComponent`) 의존 제거 여부
- PartFocus hit/action getter가 descriptor 작성에 충분한지
- `InitializePlacedItem`에서 `ItemDefinition->WorldMesh` 있을 때만 mesh override하는지

### 4) Secondary 입력 라우팅
- `ABeekeeperCharacter::FocusSecondaryInput` -> `UBeekeeperFocusComponent::HandleSecondaryInput`
- engaged일 때만 `EngagedFocusAction->HandleSecondaryInputWhileEngaged` 호출
- anchored action이 scope `HandleSecondaryInput()`으로 전달하는지
- scope가 hovered descriptor/action 유효성, required state 조건을 올바르게 체크하는지

### 5) 회수 로직 정확성
- `UPlacedItemRetrievePartFocusActionComponent`에서
  - owner/type/character/hotbar/item definition 검증
  - `TryAcquireItem(ItemDefinition, 1)`
  - `!bSuccess || AddedQuantity != 1` 실패 처리
  - 성공 시 slot interface clear 호출
- 실패 시 destroy 금지, 성공 시 slot 점유 해제/descriptor 재전환 여부

### 6) 구경로/중복 경로 위험
- `UFocusSecondaryActionComponent`, `UPlacedItemRetrieveFocusActionComponent`가 남아있다면
  - 현재 호출 경로와 충돌 가능성
  - dead code 여부
  - 향후 제거 필요성
- multi-agent 변경으로 생긴 이중 처리(같은 입력을 두 경로가 소비) 가능성

### 7) Blueprint/API/Redirect 안전성
- `APollenPattyActor` rename/UCLASS rename/file rename 없음
- `Config/DefaultEngine.ini` CoreRedirect 추가/변경 없음
- BP native parent/serialized component 이름 변경으로 인한 파손 위험 지점

---

## 코드 검색 기준

### 있어야 함
- `ICursorPartFocusProvider`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `GetCursorPartFocusDescriptors`
- `HandleSecondaryPartFocusAction`
- `HandleSecondaryInputWhileEngaged`
- `UPlacedItemRetrievePartFocusActionComponent`
- `APlacedItemActor::GetPartFocusHitComponent`
- `TryAcquireItem(ItemDefinition, 1)`
- `AddedQuantity != 1`

### 없어야 함 (또는 영향 분석 필수)
- `APlacedItemActor`의 `UFocusTargetComponent` 기본 subobject
- placed item 회수의 global preview secondary 의존
- `APollenPattyActor` rename
- `CoreRedirect` 신규 추가
- UI 위젯 직접 회수 mutation

---

## 검증 방법

1. 코드 리뷰 + 검색 결과 제시
2. 빌드 확인
   - `BeekeepingSimEditor Win64 Development`
3. 가능하면 PIE 시나리오 기반 논리 검증
   - empty/occupied 전환 시 descriptor 전환
   - hover RMB 회수 성공/실패 분기
   - 다른 파트(lid/comb) RMB 무동작

---

## 리뷰 결과 출력 형식

- Findings를 **High -> Medium -> Low** 순으로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 문제 원인
  - 실제 영향
  - 수정 제안
- Findings 이후:
  - 불확실성/가정(특히 multi-agent 변경으로 인한 추정 지점)
  - 추가 검증 필요 항목
  - 문서 동기화 누락 여부
