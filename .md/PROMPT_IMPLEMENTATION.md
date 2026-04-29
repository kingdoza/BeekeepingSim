# 구현 프롬프트: 8단계 리팩토링 리뷰 후속 개선

## 배경

8단계 리팩토링 검증 리뷰 결과, 로컬 워킹트리 기준 C++ 빌드는 통과했지만 제출/커밋 단위에서 누락될 수 있는 항목이 확인됐다.

핵심 문제는 새로 추가된 helper 파일과 분할 문서가 `git status` 상 `??` 상태라는 점이다. 현재 로컬 파일이 존재하기 때문에 UBT는 `Result: Succeeded`로 종료됐지만, 추적되지 않은 파일이 커밋/패치에 포함되지 않으면 다른 환경에서는 include 누락 또는 문서 링크 깨짐이 발생한다.

## 리뷰 결과 요약

### High

- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

두 파일이 현재 untracked 상태다.

영향 지점:

- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp` line 12
  - `#include "Inventory/ItemStackMoveUtils.h"`
- `Source/BeekeepingSim/Private/Inventory/StorageBoxComponent.cpp` line 6
  - `#include "Inventory/ItemStackMoveUtils.h"`

이 상태로 tracked diff만 제출하면 helper header/cpp가 빠지고 빌드가 실패한다.

### Medium

- `.md/Architecture/` 전체가 untracked 상태다.
- `.md/0_ARCHITECTURE.md` line 28-35는 `.md/Architecture/*.md` 문서 8개를 링크한다.

tracked diff만 제출하면 지도 문서의 링크 대상이 누락되어 7단계 문서 분할 요구를 충족하지 못한다.

### Medium

- `Content/UI/WBP_ItemVisual.uasset`가 수정 상태다.

이번 리뷰 대상 핵심 경로와 리팩토링 규칙은 C++ 구조 이동/include 정리/문서 분할 중심이다. Binary asset 변경은 코드 리뷰로 의미 있는 diff 검증이 불가능하므로, 의도된 Blueprint migration 결과인지 확인해야 한다.

## 구현 목표

1. 8단계 리팩토링 산출물이 커밋/패치 단위에서도 재현 가능하도록 추적 누락을 해소한다.
2. 범위 밖 binary asset 변경이 의도된 것인지 확인하고, 의도되지 않았다면 별도 승인 후 되돌린다.
3. 동작 변경 없이 현재 통과한 include/API/build 상태를 유지한다.

## 작업 범위

허용 작업:

- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h` 추적 포함
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp` 추적 포함
- `.md/Architecture/*.md` 8개 문서 추적 포함
- `Content/UI/WBP_ItemVisual.uasset` 변경 의도 확인
- 의도되지 않은 asset 변경을 되돌리는 작업은 반드시 사용자 승인 후 수행

금지 작업:

- 클래스명/파일명 rename
- UCLASS/USTRUCT/UENUM 이름 변경
- public Blueprint API 삭제 또는 시그니처 변경
- `UItemDragVisualWidget` 제거
- `UFocusTargetComponent::bClearFocusOnConfirm` 제거
- `ShouldClearFocusOnConfirm()` 제거
- `UStorageSlotDragDropOperation`, `EStorageSlotContainerType`, `FItemSlotMoveResult` 이름 변경
- Inventory stack move 동작 변경
- 분석 범위 밖 C++ 수정
- 승인 없는 binary asset revert

## 세부 요구사항

### 1. Helper 파일 추적 누락 해소

`ItemStackMoveUtils.*`가 반드시 최종 제출 대상에 포함되도록 한다.

확인 명령:

```powershell
git status --short -- Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp
```

기대 상태:

- `A  Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `A  Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

주의:

- 파일 내용을 변경하지 않는다.
- helper는 계산/생성 공통화만 담당해야 한다.
- Hotbar 생성 item outer는 `UBeekeeperHotbarComponent`여야 한다.
- Storage 생성 item outer는 `UStorageBoxComponent`여야 한다.

### 2. Architecture 문서 추적 누락 해소

`.md/Architecture/`의 8개 문서가 최종 제출 대상에 포함되도록 한다.

필수 파일:

- `.md/Architecture/CameraSystem.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/WorldActorsSystem.md`

확인 명령:

```powershell
Get-ChildItem .md/Architecture -File | Select-Object -ExpandProperty Name
git status --short -- .md/Architecture
```

기대 상태:

- 8개 문서가 존재한다.
- 8개 문서가 `A` 상태로 추적 대상에 포함된다.

### 3. Binary asset 변경 검증

`Content/UI/WBP_ItemVisual.uasset` 변경이 이번 리팩토링에 필요한지 확인한다.

확인 명령:

```powershell
git status --short -- Content/UI/WBP_ItemVisual.uasset
```

판단 기준:

- 의도된 Blueprint asset migration이면 리뷰/커밋 메시지에 사유를 명시한다.
- 의도되지 않은 변경이면 사용자 승인 후 되돌린다.
- 승인 없이 revert하지 않는다.

### 4. 기존 통과 항목 재검증

다음 항목은 현재 리뷰에서 통과했으므로, 후속 작업 후에도 유지해야 한다.

- `rg '#include "Public/' Source/BeekeepingSim` 결과 없음
- `.generated.h` 이후 일반 include 없음
- 60개 대상 파일 rename 상태 유지
- `Core` source 폴더 없음
- public Blueprint API 변경 없음
- `UItemDragVisualWidget` 유지
- `bClearFocusOnConfirm` / `ShouldClearFocusOnConfirm()` 유지
- `StorageSlotDragDropOperation` / `StorageSlotDragDropTypes` 이름 유지
- UBT 빌드 통과

## 검증 명령

```powershell
rg '#include "Public/' Source/BeekeepingSim
rg 'class BEEKEEPINGSIM_API UStorageSlotDragDropOperation|enum class EStorageSlotContainerType|struct FItemSlotMoveResult' Source/BeekeepingSim
git status --short
```

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe' BeekeepingSimEditor Win64 Development -Project='C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject' -WaitMutex -NoHotReloadFromIDE
```

## 완료 기준

- `ItemStackMoveUtils.h/.cpp`가 최종 제출 대상에 포함된다.
- `.md/Architecture/*.md` 8개 문서가 최종 제출 대상에 포함된다.
- `Content/UI/WBP_ItemVisual.uasset` 변경 사유가 확인되거나, 사용자 승인 후 정리된다.
- `#include "Public/` 잔여가 없다.
- UBT 빌드가 성공한다.
- 리팩토링 고정 제약 위반이 없다.
