# 코드 리뷰 요청 프롬프트 (8단계 리팩토링 검증)

아래 변경은 **완료된 8단계 리팩토링 구현**이다.  
Unreal Engine 5.7 C++ 기준으로 **동작 회귀/구조 준수/리팩토링 규칙 위반 여부**를 리뷰하라.

## 리뷰 목표

1. 파일 이동/폴더 구조 재구성이 계획대로 반영됐는지
2. include 경로 정리가 완전한지 (`Public/...` 잔여 금지)
3. 클래스명/UCLASS/USTRUCT/UENUM/public Blueprint API 변경 여부
4. Inventory 스택 이동 중복 통합이 **동작 변경 없이** 수행됐는지
5. 지정된 주석 정리가 정확히 반영됐는지
6. Architecture 문서 분할이 요구 형식을 충족하는지
7. 빌드 가능 상태 유지 여부(UBT)

---

## 고정 제약(리뷰 기준)

- 클래스명/파일명 rename 금지
- public Blueprint API 삭제/시그니처 변경 금지
- `UItemDragVisualWidget` 유지
- `UFocusTargetComponent::bClearFocusOnConfirm` 유지
- `ShouldClearFocusOnConfirm()` 유지
- `UStorageSlotDragDropOperation`, `EStorageSlotContainerType`, `FItemSlotMoveResult` 이름 유지
- 분석 범위 밖 C++ 수정은 include 보정만 허용
- 동작 변경 전제 리팩토링 금지

---

## 리뷰 대상 핵심 경로

- `Source/BeekeepingSim/Public/{Character,Camera,Focus,Interaction,Inventory,UI,WorldActors}/**/*.h`
- `Source/BeekeepingSim/Private/{Character,Camera,Focus,Interaction,Inventory,UI,WorldActors}/**/*.{h,cpp}`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/*.md`

---

## 단계별 검증 포인트

### 1~3단계: 구조 이동
- 60개 대상 파일이 시스템 폴더로 이동되었는지
- `git mv` 결과로 추적 가능한 rename 상태인지
- `Core` 소스 폴더가 생성되지 않았는지

### 4단계: include 정리
- `#include "Public/...` 잔여가 없는지
- 이동된 public 헤더가 `System/File.h` 형식으로 참조되는지
- `.generated.h` 규칙(마지막 include) 위반이 없는지

### 5단계: Inventory helper 통합
- `TryAcquireItem`, partial move 계열 API 시그니처 유지
- `ItemStackMoveUtils`가 계산/생성 공통화만 담당하는지
- delegate broadcast/reevaluate 순서가 기존과 동일한지
- outer 소유권 규칙 유지:
  - Hotbar 생성 아이템 outer = `UBeekeeperHotbarComponent`
  - Storage 생성 아이템 outer = `UStorageBoxComponent`
- QuickMove 대상 선택 우선순위(merge 가능 슬롯 -> 빈 슬롯) 회귀 없는지

### 6단계: 주석 정리
- 아래만 제거됐는지, 그 외 의미 있는 주석 손상 없는지
  - `BeekeeperCharacter.cpp` 템플릿/예시 주석
  - `BeekeeperHotbarComponent.cpp` 구 `HandleWheelInput` 주석 코드
  - `BeekeeperMovementComponent.cpp` 주석 코드/예시 주석
  - `BeekeeperCameraShakeComponent.h` `IdleSpeedThreshold` 추측 주석
- 파일 상단 copyright 주석 유지 여부
- `ItemSlotWidget.h` compatibility wrapper 주석 유지 여부

### 7단계: 문서 분할
- `.md/0_ARCHITECTURE.md`가 지도 문서로 축소되었는지
- `.md/Architecture/` 8개 문서 존재 여부
- 각 문서 필수 섹션 존재 여부:
  - `Scope`
  - `Responsibilities`
  - `Key Classes`
  - `Dependencies`
  - `Refactoring Notes`
  - `Manual Review Points`
- `CoreSystem.md`에 “Core 전용 source 파일 없음” 명시 여부
- `FocusSystem.md`, `UISystem.md`의 유지/미래검토 메모 반영 여부

### 8단계: 검증 결과
- UBT 빌드 성공 여부(`BeekeepingSimEditor Win64 Development`)
- 빌드 실패 시 include/전방선언/헤더 추가 범위 밖 수정이 없는지

---

## 리뷰 시 권장 확인 명령

- `rg '#include "Public/' Source/BeekeepingSim`
- `rg 'class BEEKEEPINGSIM_API UStorageSlotDragDropOperation|enum class EStorageSlotContainerType|struct FItemSlotMoveResult' Source/BeekeepingSim`
- `git diff --name-status`
- `git diff -- Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `git diff -- Source/BeekeepingSim/Private/Inventory/StorageBoxComponent.cpp`

---

## 출력 형식

1. **Findings (Severity 순: High -> Medium -> Low)**  
   - 각 항목에 파일/함수/라인 근거 포함
2. **Open Questions / Assumptions**
3. **Regression Risk Checklist**
4. **Pass/Fail 요약**

이슈가 없으면 “치명/중간 이슈 없음”을 명시하고, 남은 테스트 공백만 적어라.
