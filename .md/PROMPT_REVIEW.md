# 코드 리뷰 요청 프롬프트 (보류 리팩토링 후속 구현 최종 검토)

아래 변경은 Unreal Engine 5.7 기준 **보류 리팩토링 후속 구현**이다.  
이번 변경은 고위험 항목을 포함하며, C++/Config 리팩토링 + 사용자의 Blueprint 수동 마이그레이션(컴파일/저장 완료)까지 반영된 상태다.

리뷰 목표는 **회귀 위험, 직렬화 호환성, API 파손 여부, 문서-코드 불일치**를 찾는 것이다.

---

## 현재 구현 사실(리뷰 전제)

- 완료된 주요 변경:
  - `UFocusTargetComponent::bClearFocusOnConfirm` 제거
  - `UFocusTargetComponent::ShouldClearFocusOnConfirm()` 제거
  - `UStorageBoxWidget` 이동/스왑 wrapper API 제거
  - `UItemSlotWidget::GetDragPreviewDisplayStackCount()` 제거
  - `UItemSlotWidget` 일부 함수 Blueprint 노출 제거(C++ 전용 유지)
  - `UItemDragVisualWidget` 삭제
  - Drag/Drop 타입 rename
    - `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
    - `EStorageSlotContainerType` -> `EItemSlotContainerType`
  - 파일명 rename
    - `StorageSlotDragDropOperation.h/.cpp` -> `ItemSlotDragDropOperation.h/.cpp`
    - `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`
  - `Config/DefaultEngine.ini` Core Redirect 추가

- 사용자 측 작업(완료):
  - 관련 Blueprint 수동 마이그레이션/컴파일/저장 완료
  - 동작 정상 확인 완료

---

## 리뷰 목표

1. C++ rename/삭제가 일관되게 반영됐는지
2. Core Redirect 설정이 안전하고 누락 없는지
3. Blueprint 연동 전제 하에 런타임 회귀 가능성이 남아있는지
4. include/forward declaration/UHT 생성 규칙 위반이 없는지
5. 문서(`.md`)가 실제 구현 결과를 정확히 반영하는지

---

## 리뷰 대상 경로

- `Source/BeekeepingSim/Public/**`
- `Source/BeekeepingSim/Private/**`
- `Config/DefaultEngine.ini`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`
- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`

---

## 필수 검증 포인트

### A. 타입/파일 rename 정합성
- 구심볼이 C++ 코드에 잔존하지 않는지:
  - `UStorageSlotDragDropOperation`
  - `EStorageSlotContainerType`
  - `StorageSlotDragDropOperation.*`
  - `StorageSlotDragDropTypes.h`
- 신심볼 사용이 일관적인지:
  - `UItemSlotDragDropOperation`
  - `EItemSlotContainerType`
  - `ItemSlotDragDropOperation.*`
  - `ItemSlotDragDropTypes.h`

### B. 삭제 항목 회귀
- `UItemDragVisualWidget` 삭제 후 C++ 직접 참조 잔존 여부
- 제거된 Focus/Storage/UI API를 호출하는 코드가 남아있지 않은지

### C. Redirect 안정성
- `DefaultEngine.ini` `[CoreRedirects]` 문법/섹션 위치/중복/충돌 점검
- ClassRedirect/EnumRedirect 대상명이 실제 UCLASS/UENUM 변경과 정확히 대응하는지

### D. 빌드/로딩 신뢰도
- UBT 빌드 성공 기준에서 경고성 리스크가 없는지
- Editor-Cmd 로그 기준 missing class/enum/property 경고가 남는지

### E. 문서 정확도
- 문서가 “이미 삭제/rename 완료된 상태”를 반영하는지
- 아직 보류로 남겨야 할 항목과 완료 항목이 혼동 없이 구분되는지

---

## 권장 확인 명령

- `rg '#include "Public/' Source/BeekeepingSim`
- `rg 'UStorageSlotDragDropOperation|EStorageSlotContainerType|StorageSlotDragDropOperation|StorageSlotDragDropTypes|UItemDragVisualWidget' Source/BeekeepingSim`
- `rg 'UItemSlotDragDropOperation|EItemSlotContainerType|ItemSlotDragDropOperation|ItemSlotDragDropTypes' Source/BeekeepingSim`
- `git diff --name-status`
- `git diff -- Config/DefaultEngine.ini`

---

## 출력 형식 (반드시 준수)

1. **Findings (High -> Medium -> Low)**  
   - 각 이슈에 파일/심볼/근거 포함
2. **Open Questions / Assumptions**
3. **Regression Risk Checklist**
4. **최종 판정: Pass / Conditional Pass / Fail**

이슈가 없으면 **“High/Medium 이슈 없음”**을 명시하고, 남은 테스트 공백만 간단히 적어라.
