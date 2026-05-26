# VFX Item Presentation Actor 구현 프롬프트

## 전제

기존 held item active lifecycle 구현은 이미 완료된 상태로 본다.

이미 존재한다고 가정하는 경로:

- `AItemPresentationActor::BeginItemUseActive()`
- `AItemPresentationActor::EndItemUseActive(bool bCanceled)`
- `AItemPresentationActor::ReceiveItemUseActiveStarted`
- `AItemPresentationActor::ReceiveItemUseActiveEnded`
- `UBeekeeperHeldItemVisualizerComponent`가 held presentation actor에 active 시작/종료를 전달
- `UDisinfectantUseAction`은 presentation actor/VFX를 직접 모르고 held item visualizer에 active 시작/종료만 알림

이번 작업은 `AItemPresentationActor`의 재사용 가능한 VFX subclass를 추가하는 것이다.

## 목표

여러 held item presentation actor가 공통으로 사용할 수 있는 generic VFX presentation actor를 구현한다.

클래스 역할:

- C++에서 Niagara component를 생성하고 소유한다.
- active 시작 시 Niagara VFX를 재생한다.
- active 종료 시 Niagara VFX를 정지한다.
- Niagara System asset, transform, renderer/parameter 세팅은 BP의 Niagara component Details에서 설정한다.

## 클래스 이름

권장:

```cpp
AVfxItemPresentationActor : public AItemPresentationActor
```

권장 파일:

- `Source/BeekeepingSim/Public/Inventory/VfxItemPresentationActor.h`
- `Source/BeekeepingSim/Private/Inventory/VfxItemPresentationActor.cpp`

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/USER_UNREAL.md`
- `.md/QNA_IMPLEMENTATION.md`

## 설계 원칙

`AVfxItemPresentationActor`에는 NiagaraComponent 자체가 이미 가진 설정과 중복되는 UPROPERTY를 만들지 않는다.

만들지 말 것:

- `UNiagaraSystem* UseVfxSystem`
- `NiagaraSystemAsset`
- `UseVfxRelativeTransform`
- `UseVfxAttachSocketName`
- 별도 `bAutoActivate` property

이유:

- Niagara asset은 `UNiagaraComponent`의 Details에서 직접 설정한다.
- transform은 `UseVfxComponent` 자체 transform으로 조정한다.
- attach/socket은 BP component hierarchy에서 처리한다.
- auto activate는 NiagaraComponent 기본 property이며, C++ constructor에서 false 기본값만 지정한다.

클래스에 둘 것:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UNiagaraComponent> UseVfxComponent;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Presentation|VFX")
bool bResetVfxOnStart = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Presentation|VFX")
bool bDeactivateImmediatelyOnEnd = false;
```

`bResetVfxOnStart`, `bDeactivateImmediatelyOnEnd`는 NiagaraComponent asset/transform 설정이 아니라 active lifecycle 정책이므로 subclass property로 허용한다.

## 구현 요구

### 생성자

- `UseVfxComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("UseVfxComponent"))`
- 기존 `AItemPresentationActor`의 root/component 구조를 확인해 적절한 parent에 attach한다.
  - root component가 있으면 root에 attach
  - item mesh/fallback mesh에 attach하는 기존 패턴이 있으면 그 패턴을 따른다.
- `UseVfxComponent->SetAutoActivate(false)`
- tick/collision 설정은 Niagara 기본값을 유지한다.

### active 시작

`ReceiveItemUseActiveStarted_Implementation()` override:

```cpp
void AVfxItemPresentationActor::ReceiveItemUseActiveStarted_Implementation()
{
	Super::ReceiveItemUseActiveStarted_Implementation();

	if (!UseVfxComponent)
	{
		return;
	}

	if (bResetVfxOnStart)
	{
		UseVfxComponent->ResetSystem();
	}

	UseVfxComponent->Activate(true);
}
```

### active 종료

`ReceiveItemUseActiveEnded_Implementation(bool bCanceled)` override:

```cpp
void AVfxItemPresentationActor::ReceiveItemUseActiveEnded_Implementation(bool bCanceled)
{
	Super::ReceiveItemUseActiveEnded_Implementation(bCanceled);

	if (!UseVfxComponent)
	{
		return;
	}

	if (bDeactivateImmediatelyOnEnd)
	{
		UseVfxComponent->DeactivateImmediate();
	}
	else
	{
		UseVfxComponent->Deactivate();
	}
}
```

### EndPlay 안전 정리

필요하면 `EndPlay(...)` override:

- `UseVfxComponent`가 유효하면 `DeactivateImmediate()`
- `Super::EndPlay(...)` 호출 순서는 기존 local pattern을 따른다.

단, `AItemPresentationActor` base가 이미 active 종료를 보장한다면 중복 정리를 피한다.

## BP/Editor 작업 문서화

`.md/USER_UNREAL.md`에 아래 절차를 추가한다.

소독약 VFX presentation BP:

1. `AVfxItemPresentationActor` 기반 BP 생성
2. `UseVfxComponent` 선택
3. Niagara System Asset에 소독약 분사 Niagara system 지정
4. `UseVfxComponent` 위치/회전/스케일 조정
5. `Auto Activate`는 false 유지
6. 필요하면 `bResetVfxOnStart`, `bDeactivateImmediatelyOnEnd` 조정
7. 소독약 item definition의 held/presentation actor class를 이 BP로 설정

다른 VFX item:

- 같은 `AVfxItemPresentationActor` 기반 BP를 만들고 `UseVfxComponent`의 Niagara System Asset만 다른 것으로 지정한다.

## 검증 기준

### 코드 검색

아래가 있어야 한다.

- `AVfxItemPresentationActor`
- `UseVfxComponent`
- `bResetVfxOnStart`
- `bDeactivateImmediatelyOnEnd`
- `ReceiveItemUseActiveStarted_Implementation`
- `ReceiveItemUseActiveEnded_Implementation`
- `UseVfxComponent->Activate`
- `UseVfxComponent->Deactivate`

아래가 없어야 한다.

- `AVfxItemPresentationActor`의 `UNiagaraSystem* UseVfxSystem`
- `AVfxItemPresentationActor`의 `NiagaraSystemAsset`
- `AVfxItemPresentationActor`의 `UseVfxRelativeTransform`
- `AVfxItemPresentationActor`의 `UseVfxAttachSocketName`
- `UDisinfectantUseAction`이 Niagara/VFX component를 직접 참조하는 코드
- `UBeekeeperHeldItemVisualizerComponent`가 소독약 전용 VFX component를 직접 참조하는 코드

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

### PIE 수동 검증

1. `AVfxItemPresentationActor` 기반 소독약 presentation BP를 만든다.
2. `UseVfxComponent`에 Niagara System Asset을 지정한다.
3. 소독약 item definition의 presentation actor class를 해당 BP로 설정한다.
4. 소독약 선택 후 벌통 FocusEngaged 상태에서 lid/comb use area 위 LMB hold
5. hold 시작 시 Niagara VFX가 재생된다.
6. hold release/cancel/focus exit/hotbar 변경 시 Niagara VFX가 정지된다.
7. sanitation 증가 동작은 기존처럼 유지된다.

## 문서 갱신

구현 후 갱신:

- `.md/Architecture/InventorySystem.md`
  - `AVfxItemPresentationActor` 역할 추가
  - VFX asset/transform은 NiagaraComponent Details에서 설정한다는 경계 기록
- `.md/USER_UNREAL.md`
  - `AVfxItemPresentationActor` 기반 BP 생성 및 NiagaraComponent 설정 절차 추가

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `AItemPresentationActor`의 root/component 구조상 `UseVfxComponent` attach parent를 결정할 수 없는 경우
- active lifecycle 함수/이벤트 이름이 현재 구현과 다르게 되어 있는 경우
- Niagara module dependency가 Build.cs에 없고 추가 위치가 불명확한 경우
- `EndPlay`/hidden lifecycle에서 base active 종료와 subclass VFX 정리가 중복 충돌하는 경우
- Content asset 직접 수정 없이는 검증 가능한 BP 설정이 불가능한 경우
