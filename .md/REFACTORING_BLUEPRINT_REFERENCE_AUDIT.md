# Blueprint Reference Audit

## 상태

- 기준일: 2026-04-29
- 범위: 보류 리팩토링 후속 구현 완료 상태(post-migration)
- 전제:
  - C++/Config 리팩토링 반영 완료
  - 사용자가 Blueprint 수동 마이그레이션/컴파일/저장 완료

## 현재 결론 (Post-migration)

- 완료된 항목:
  - `UItemDragVisualWidget` 삭제
  - `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation` rename
  - `EStorageSlotContainerType` -> `EItemSlotContainerType` rename
  - `Config/DefaultEngine.ini` Core Redirect 반영
  - `bClearFocusOnConfirm` / `ShouldClearFocusOnConfirm` 제거
  - `UStorageBoxWidget` wrapper API 제거
  - `UItemSlotWidget::GetDragPreviewDisplayStackCount` 제거

- 유지된 항목:
  - `EItemSlotDragMode`
  - `FItemSlotMoveResult`
  - `InitializeSlotContext`
  - `ShouldHideItemVisualForCurrentDrag`
  - `IsPartialDragPreviewActive`
  - `GetPartialDragPreviewDisplayStackCount`
  - `OnStorageWidgetInitialized`

## 원본 산출물 처리

- 검사 과정에서 생성했던 Python/JSON 중간 산출물은 최종 문서 정리 단계에서 삭제했다.
- 이 문서는 해당 검사 결과의 요약본이자 최종 기록이다.
- 향후 유사 리팩토링에서 Blueprint 참조 확인이 필요하면, 그 시점의 최신 `Content` 기준으로 다시 일괄 검사한다.

## 검증 요약

- UBT/UHT 빌드: 성공
- Editor-Cmd 로드: 성공
- rename/delete 관련 missing class/enum/property 오류: 미발견

## Historical (Pre-migration Findings)

- 아래 항목들은 과거 의사결정 근거이며 **현재 상태를 의미하지 않는다**:
  - `UItemDragVisualWidget` 잔존 문자열 위험
  - `UStorageSlotDragDropOperation`, `EStorageSlotContainerType` rename 위험
  - Core Redirect 미적용 상태 위험

- 위 historical 위험은 post-migration 구현과 사용자 수동 Blueprint 마이그레이션 완료로 해소되었다.
