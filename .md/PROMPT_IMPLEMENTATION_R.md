# 구현 프롬프트: 보류 리팩토링 후속 구현 리뷰 반영

## 배경

보류 리팩토링 후속 구현 최종 검토 결과, 현재 로컬 워킹트리 기준 C++/UHT/UBT는 통과했지만 제출 단위에서 치명적인 누락 위험이 확인됐다.

핵심 문제는 drag/drop rename의 신규 파일들이 아직 `??` untracked 상태라는 점이다. 현재 로컬에는 파일이 존재하므로 빌드는 성공하지만, 커밋/패치에 포함되지 않으면 기존 `StorageSlotDragDrop*` 파일만 삭제되고 신규 `ItemSlotDragDrop*` 파일은 누락되어 다른 환경에서 빌드가 실패한다.

또한 Blueprint 수동 마이그레이션 완료 상태와 일부 `.md`/audit JSON 문서 내용이 불일치한다.

## 리뷰 판정 요약

- 최종 판정: Fail
- 이유: 제출 단위 기준 신규 rename 파일 누락 가능성 및 문서/audit 상태 불일치
- 현재 로컬 빌드: UBT/UHT 통과
- 최신 로그: 이번 rename/delete 관련 missing class/enum/property 경고 검색 결과 없음

## 구현 목표

1. drag/drop rename 신규 파일을 최종 제출 대상에 포함한다.
2. 삭제된 구 파일과 신규 파일이 rename 결과로 추적 가능하게 정리한다.
3. Blueprint 수동 마이그레이션 완료 사실과 문서/audit JSON 상태를 일치시킨다.
4. 리뷰 대상 밖 IDE 파일 변경이 제출에 섞이지 않도록 한다.
5. C++ 동작/API는 추가 변경하지 않고 현재 빌드 성공 상태를 유지한다.

## 수정 허용 범위

허용:

- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropOperation.cpp` 추적 포함
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropOperation.h` 추적 포함
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropTypes.h` 추적 포함
- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md` 현재 완료 상태에 맞게 갱신
- Blueprint 참조 검사 결과는 최종 요약을 `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`에 반영한다.
- 검사 과정에서 임시 Python/JSON 산출물이 필요하면 생성해도 되지만, 최종 정리 단계에서 삭제한다.
- 필요 시 `.md/0_ARCHITECTURE.md`, `.md/Architecture/UISystem.md`, `.md/Architecture/FocusSystem.md`, `.md/Architecture/InventorySystem.md`의 표현 보정

금지:

- C++ 동작 변경
- UCLASS/USTRUCT/UENUM 이름 추가 변경
- public Blueprint API 추가 삭제
- `UItemSlotDragDropOperation`, `EItemSlotContainerType`, `EItemSlotDragMode`, `FItemSlotMoveResult` 이름 변경
- `UFocusTargetComponent` 클래스 자체 변경
- `Config/DefaultEngine.ini` Core Redirect 의미 변경
- Blueprint asset 수정 자동화
- `.idea/.idea.BeekeepingSim/.idea/workspace.xml` 제출 포함

## 필수 작업

### 1. Rename 신규 파일 추적 포함

현재 상태:

```text
D  Source/BeekeepingSim/Private/UI/StorageSlotDragDropOperation.cpp
D  Source/BeekeepingSim/Public/UI/StorageSlotDragDropOperation.h
D  Source/BeekeepingSim/Public/UI/StorageSlotDragDropTypes.h
?? Source/BeekeepingSim/Private/UI/ItemSlotDragDropOperation.cpp
?? Source/BeekeepingSim/Public/UI/ItemSlotDragDropOperation.h
?? Source/BeekeepingSim/Public/UI/ItemSlotDragDropTypes.h
```

요구사항:

- 신규 `ItemSlotDragDrop*` 3개 파일이 반드시 최종 제출 대상에 포함되어야 한다.
- 가능하면 `git status`/`git diff -M --name-status`에서 rename으로 추적 가능해야 한다.
- 최소한 삭제 3개 + 추가 3개가 모두 제출 대상에 포함되어야 한다.

확인 명령:

```powershell
git status --short -- Source/BeekeepingSim/Private/UI/ItemSlotDragDropOperation.cpp Source/BeekeepingSim/Public/UI/ItemSlotDragDropOperation.h Source/BeekeepingSim/Public/UI/ItemSlotDragDropTypes.h Source/BeekeepingSim/Private/UI/StorageSlotDragDropOperation.cpp Source/BeekeepingSim/Public/UI/StorageSlotDragDropOperation.h Source/BeekeepingSim/Public/UI/StorageSlotDragDropTypes.h

git diff -M --name-status -- Source/BeekeepingSim/Private/UI Source/BeekeepingSim/Public/UI
```

기대:

- 신규 파일이 `A` 또는 rename 결과로 표시된다.
- 기존 `StorageSlotDragDrop*` 삭제만 있고 신규 파일이 누락되는 상태는 없어야 한다.

### 2. Stale Blueprint audit/scan 갱신

현재 불일치:

- Blueprint 참조 감사 기록에 `UItemDragVisualWidget`, `StorageSlotDragDropOperation`, `EStorageSlotContainerType` 잔존 위험이 완료 상태로 정리되지 않았다.
- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`에 `UItemDragVisualWidget` 삭제와 enum rename이 여전히 위험/보류/잔존 심볼로 기술되어 있다.
- 현재 구현 전제는 사용자가 Blueprint 수동 마이그레이션/컴파일/저장을 완료했고 동작 확인도 끝낸 상태다.

요구사항:

- audit 문서는 post-migration 상태를 기준으로 갱신한다.
- 완료 항목과 보류 항목을 혼동하지 않게 분리한다.
- `UItemDragVisualWidget` 삭제, `UStorageSlotDragDropOperation -> UItemSlotDragDropOperation`, `EStorageSlotContainerType -> EItemSlotContainerType`는 완료 상태로 기록한다.
- 과거 위험 분석은 필요하면 `Historical / Pre-migration Findings` 같은 섹션으로 분리하고 현재 상태처럼 보이지 않게 한다.
- Blueprint 참조 검사는 현재 에셋 상태 기준으로 수행하는 것이 원칙이며, 최종 결과만 audit 문서에 요약한다.
- 재생성이 불가능하면 문서에 `stale pre-migration snapshot`임을 명확히 표기하거나, 리뷰 대상에서 현재 상태 증거로 오인되지 않게 갱신한다.

확인 명령:

```powershell
rg 'UItemDragVisualWidget|UStorageSlotDragDropOperation|EStorageSlotContainerType|StorageSlotDragDropOperation|StorageSlotDragDropTypes' .md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md
```

기대:

- 현재 상태 섹션에서 구심볼을 완료된 rename/delete 설명 외의 잔존/보류 항목으로 표현하지 않는다.
- JSON이 현재 상태를 표현한다면 구심볼 hit는 없어야 한다.
- JSON이 과거 snapshot이면 그 사실이 파일 내 metadata 또는 인접 문서에 명확해야 한다.

### 3. 문서 상태 정합성 재확인

대상:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`

요구사항:

- 이미 삭제/rename 완료된 항목을 보류로 쓰지 않는다.
- 아직 유지되는 항목과 삭제된 항목을 명확히 구분한다.
- `UItemDragVisualWidget`은 삭제 완료로 기록한다.
- `UFocusTargetComponent::bClearFocusOnConfirm` / `ShouldClearFocusOnConfirm()`은 제거 완료로 기록한다.
- `UStorageBoxWidget` 이동/스왑 wrapper API 제거 완료를 반영한다.
- `UItemSlotWidget::GetDragPreviewDisplayStackCount()` 제거 완료를 반영한다.
- drag/drop 타입 rename 및 Core Redirect 적용 완료를 반영한다.

확인 명령:

```powershell
rg '보류|완료|삭제|rename|UItemDragVisualWidget|UStorageSlotDragDropOperation|EStorageSlotContainerType|UItemSlotDragDropOperation|EItemSlotContainerType|bClearFocusOnConfirm|ShouldClearFocusOnConfirm' .md/0_ARCHITECTURE.md .md/Architecture/UISystem.md .md/Architecture/FocusSystem.md .md/Architecture/InventorySystem.md
```

### 4. IDE 파일 변경 제외

`.idea/.idea.BeekeepingSim/.idea/workspace.xml`는 리뷰 대상 밖 변경이다.

요구사항:

- 최종 제출에 포함하지 않는다.
- 되돌리기가 필요하면 사용자 승인 후 수행한다.
- 승인 없이 destructive revert를 하지 않는다.

확인 명령:

```powershell
git status --short -- .idea/.idea.BeekeepingSim/.idea/workspace.xml
```

## 반드시 유지해야 할 통과 항목

- `rg '#include "Public/' Source/BeekeepingSim` 결과 없음
- C++ source에서 구 drag/drop 심볼 직접 참조 없음
- `UItemDragVisualWidget` C++ 참조 없음
- `bClearFocusOnConfirm` / `ShouldClearFocusOnConfirm` C++ 참조 없음
- `.generated.h` 이후 일반 include 없음
- Core Redirect 유지:
  - `StorageSlotDragDropOperation -> ItemSlotDragDropOperation`
  - `EStorageSlotContainerType -> EItemSlotContainerType`
- UBT/UHT 통과

## 최종 검증 명령

```powershell
rg '#include "Public/' Source/BeekeepingSim
rg 'UStorageSlotDragDropOperation|EStorageSlotContainerType|StorageSlotDragDropOperation|StorageSlotDragDropTypes|UItemDragVisualWidget|bClearFocusOnConfirm|ShouldClearFocusOnConfirm|GetDragPreviewDisplayStackCount' Source/BeekeepingSim
rg 'UItemSlotDragDropOperation|EItemSlotContainerType|ItemSlotDragDropOperation|ItemSlotDragDropTypes' Source/BeekeepingSim
git diff -- Config/DefaultEngine.ini
git status --short
```

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSimEditor Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
```

로그 확인:

```powershell
rg -i 'StorageSlotDragDropOperation|EStorageSlotContainerType|ItemDragVisualWidget|bClearFocusOnConfirm|ShouldClearFocusOnConfirm|missing class|missing enum|missing property|failed to load.*BeekeepingSim|could not find.*BeekeepingSim|LogRedirectors|CoreRedirect' Saved/Logs/BeekeepingSim.log
```

## 완료 기준

- 신규 `ItemSlotDragDrop*` 3개 파일이 제출 대상에 포함된다.
- 기존 `StorageSlotDragDrop*` 삭제와 신규 파일 추가/rename이 함께 추적된다.
- 문서와 audit JSON이 post-migration 상태를 반영한다.
- `.idea` workspace 변경이 최종 제출에 섞이지 않는다.
- UBT/UHT가 성공한다.
- 최신 로그에 이번 rename/delete 관련 missing class/enum/property 경고가 없다.
