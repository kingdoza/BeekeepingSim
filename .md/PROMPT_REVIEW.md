# HoneyStreamNiagara Source Ownership 변경안 리뷰 프롬프트

## 리뷰 목표

이번 리뷰는 꿀 소분 작업의 `HoneyStreamNiagara` 소유권과 `TargetDropLengthCm` 산출 기준 변경만 검토한다.

핵심 요구사항:

- 꿀 줄기 Niagara component는 source `AHoneyContainerActor`가 소유한다.
- `UHoneyTransferComponent`는 active source container의 `HoneyStreamNiagara`를 우선 제어한다.
- `TargetDropLengthCm`는 source stream Z와 target pour target Z 차이로 계산한다.
- 계산식은 `Max(0.0f, SourceStream.Z - TargetPourTarget.Z)`다.
- 기존 `AHoneyDecantingTable::HoneyStreamNiagara`는 Blueprint 참조 파손을 피하기 위한 legacy fallback으로만 남는다.

이번 리뷰는 전체 꿀 용기/소분 작업대 신규 구현 전체를 다시 리뷰하는 목적이 아니다. 기존 전체 구현 이슈는 이 변경이 직접 악화시키거나 새로 만든 경우에만 언급한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`

필요 시:

- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`

## 리뷰 대상 파일

Source:

- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerActor.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyTransferComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyTransferComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyDecantingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyDecantingTable.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_IMPLEMENTATION.md`

`Content/` asset은 직접 수정 대상이 아니다. 다만 Blueprint compile/save 또는 수동 설정 필요성은 리뷰 결과에 명시한다.

## 기대 구현

### 1. `AHoneyContainerActor`

- `UNiagaraComponent* HoneyStreamNiagara`를 native component로 소유해야 한다.
- component 이름은 `HoneyStreamNiagara`여야 한다.
- 기본 상태는 auto activate off, hidden이어야 한다.
- source 용기 BP에서 Niagara System과 transform을 authoring할 수 있어야 한다.
- public getter `GetHoneyStreamNiagaraComponent()`가 있어야 한다.
- 기존 `NozzleOrigin`은 nozzle hover/click 기준점으로 유지되며, DropLength 목표 길이 계산 기준으로 쓰이지 않아야 한다.
- 기존 `PourTarget`은 target container일 때 도착 높이 기준점으로 계속 쓰여야 한다.

리뷰 포인트:

- 새 component 추가가 기존 Blueprint native parent 참조를 깨는 rename/delete를 동반하지 않는지 확인한다.
- `HoneyStreamNiagara`가 `UPROPERTY`로 GC 안전하게 잡혀 있는지 확인한다.
- target-only 용기도 component를 갖는 구조가 런타임 오류를 만들지 않는지 확인한다.

### 2. `UHoneyTransferComponent`

- active stream resolve 우선순위:
  1. active source container `GetHoneyStreamNiagaraComponent()`
  2. legacy fallback `SetHoneyStreamNiagara(...)`로 전달된 table Niagara
- `ApplyNiagaraTransferParameters`, `ApplyDropLengthParameter`, `ActivateHoneyStream`, `DeactivateHoneyStream`는 같은 active stream resolve 경로를 사용해야 한다.
- `StopTransfer(true)`는 active source/target을 null로 지우기 전에 DropLength 0 적용과 Niagara deactivate를 처리해야 한다.
- `EndPlay`, source empty, target full, slot occupant 변경, explicit stop에서 stream이 즉시 사라져야 한다.
- `SetHoneyStreamNiagara(...)`는 신규 기본 경로가 아니라 legacy fallback/compatibility API로만 동작해야 한다.

리뷰 포인트:

- active source가 invalid가 된 뒤 `StopTransfer`가 fallback Niagara만 끄고 source Niagara를 못 끄는 경로가 없는지 확인한다.
- `ValidateActiveTransfer()` 실패 시 source actor가 이미 pending kill/invalid여도 크래시 없이 정지되는지 확인한다.
- fallback과 source stream이 둘 다 있을 때 둘 중 하나만 켜지고, stale table Niagara가 남지 않는지 확인한다.

### 3. `TargetDropLengthCm` 계산

기대 계산:

```cpp
TargetDropLengthCm = Max(0.0f, SourceHoneyStream.WorldLocation.Z - TargetPourTarget.WorldLocation.Z);
```

세부 조건:

- source 기준은 `AHoneyContainerActor::HoneyStreamNiagara`다.
- target 기준은 `ActiveTargetContainer->GetPourTargetComponent()`다.
- target container `PourTarget`이 없으면 target slot `GetSlotPourTargetComponent()` fallback을 유지한다.
- source stream 또는 target 기준점을 못 찾으면 `DefaultDropLengthCm` fallback을 사용한다.
- 더 이상 `NozzleOrigin`과 `PourTarget`의 3D distance로 계산하지 않아야 한다.
- target `PourTarget`이 stream보다 위에 있으면 목표 길이는 0이 되어야 한다.

리뷰 포인트:

- `FMath::Abs`를 쓰지 않았는지 확인한다. 위쪽 target 배치를 정상 길이처럼 처리하면 안 된다.
- `FVector::Distance` 기반 기존 계산이 남아 있지 않은지 확인한다.
- 목표 길이 0인 경우 다음 tick에서 즉시 `Transferring`으로 전환되는 것이 현재 설계와 맞는지 확인한다.

### 4. `AHoneyDecantingTable`

- 신규 이송 VFX를 소유하지 않아야 한다.
- source/target slot child actor와 `UHoneyTransferComponent` 조립 host 역할만 유지해야 한다.
- 기존 `HoneyStreamNiagara` component가 남아 있다면 legacy fallback임이 문서와 코드에서 일관되어야 한다.
- 작업대에 transfer 계산이 하드코딩되지 않아야 한다.

리뷰 포인트:

- 기존 Blueprint serialized component 삭제/rename 없이 유지되었는지 확인한다.
- `ConfigureTransferComponent()`가 source/target slot을 계속 정상 연결하는지 확인한다.
- fallback table Niagara가 신규 source Niagara authoring을 방해하지 않는지 확인한다.

### 5. 문서/수동 작업 정합성

정본 문서가 아래 내용을 반영해야 한다.

- `AHoneyContainerActor`가 source 배출용 `HoneyStreamNiagara`를 소유한다.
- `UHoneyTransferComponent`가 active source stream을 제어한다.
- target length는 `Max(0, SourceStream.Z - TargetPourTarget.Z)`다.
- 신규 authoring 위치는 source `BP_HoneyContainerActor`의 `HoneyStreamNiagara`다.
- `AHoneyDecantingTable::HoneyStreamNiagara`는 legacy fallback이다.

리뷰 포인트:

- `.md/USER_UNREAL.md`에서 사용자가 작업대 Niagara가 아니라 source container Niagara에 System/transform을 세팅하도록 안내하는지 확인한다.
- `.md/QNA_ARCHITECTURE.md`에 기존 `NozzleOrigin` distance 답변이 최신 확정과 충돌하지 않게 정리되어 있는지 확인한다.
- `.md/PROMPT_IMPLEMENTATION.md`에 오래된 target length 설명이 남아 있지 않은지 확인한다.

## 금지/주의 사항

- `Content/` asset 수정/저장 금지.
- UCLASS/USTRUCT/UENUM rename 금지.
- 기존 Blueprint API 삭제/rename 금지.
- Core Redirect 추가 금지.
- `HoneyRipness` 오탈자를 새 정본으로 만들지 않는다.
- `AHoneyDecantingTable`에 target length 계산 또는 transfer 계산을 새로 넣지 않는다.

## 권장 검색

```powershell
rg -n 'HoneyStreamNiagara|LegacyHoneyStreamNiagara|ResolveActiveHoneyStreamNiagara|GetHoneyStreamNiagaraComponent|ResolveTargetDropLengthCm' Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg -n 'FVector::Distance|NozzleOrigin.*PourTarget|source nozzle origin|source container NozzleOrigin' Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n 'SourceStream.Z|TargetPourTarget.Z|Max\(0|FMath::Abs|User.DropLength|HoneyRipness' Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## PIE 수동 확인

수동 확인은 `.md/USER_UNREAL.md`를 따른다. 이번 변경안에서 특히 확인할 항목:

- source container `HoneyStreamNiagara`에 Niagara System이 설정되어 있어야 배출 VFX가 나온다.
- 배출 시작 시 source container Niagara가 activate되고 table Niagara가 기본 VFX로 쓰이지 않아야 한다.
- source `HoneyStreamNiagara` Z를 올리거나 내리면 target 주입 시작 타이밍이 바뀐다.
- target container `PourTarget` Z를 올리거나 내리면 target 주입 시작 타이밍이 바뀐다.
- target `PourTarget`이 source stream보다 위에 있으면 DropLength 목표가 0이 되어 거의 즉시 주입이 시작된다.
- 배출 중 source/target 회수 또는 slot clear 시 source container Niagara가 즉시 꺼진다.

## 리뷰 결과 작성 형식

리뷰 결과는 `.md/AGENT_REVIEW.md` 기준으로 작성한다.

- Findings first: severity, file/line, 문제, 영향, 수정 방향
- blocking/major issue가 없으면 "검토 범위에서 발견된 blocking/major issue 없음"을 명확히 적는다.
- 남은 리스크는 UBT/PIE/BP 수동 확인 여부와 연결해서 적는다.
- 구현 요약은 findings 이후에 짧게 둔다.
