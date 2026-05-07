# Cursor Part Focus 공통 Edge Cancel Thickness 구현 프롬프트

## 목표

`ScreenEdgeCancelRegionThickness`를 `UCursorPartFocusScopeComponent` 인스턴스별 값이 아니라 프로젝트 공통 설정값으로 변경한다.

현재 문제:

```text
ScreenEdgeCancelRegionThickness가 CursorPartFocusScopeComponent마다 개별 EditAnywhere 값으로 노출됨
각 액터/BP에 붙은 Scope마다 값이 달라질 수 있음
모든 Part Focus Scope에 동일한 외곽 취소 영역 정책을 적용하기 불편함
```

수정 후:

```text
ScreenEdgeCancelRegionThickness는 프로젝트/게임 전역 Focus 설정에 속한다.
모든 UCursorPartFocusScopeComponent는 동일한 공통값을 읽어서 사용한다.
Scope 컴포넌트 Details에서 개별 조절하지 않는다.
```

## 수정 대상

우선 다음 파일을 확인한다.

- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- 필요 시 `Source/BeekeepingSim/Public/Focus/BeekeepingSimFocusSettings.h`
- 필요 시 `Source/BeekeepingSim/Private/Focus/BeekeepingSimFocusSettings.cpp`
- 필요 시 `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/USER_UNREAL.md`

## 현재 구조

`UCursorPartFocusScopeComponent`에 다음과 같은 인스턴스별 값이 있다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
float ScreenEdgeCancelRegionThickness = 64.0f;
```

`HandleEdgeCancelClick()`은 이 멤버 값을 사용해 화면 외곽 취소 영역을 판정한다.

## 변경 정책

`ScreenEdgeCancelRegionThickness`의 소유권을 다음으로 이동한다.

```text
소유: UBeekeepingSimFocusSettings
사용: UCursorPartFocusScopeComponent::HandleEdgeCancelClick()
저장: Config=Game / DefaultGame.ini
노출: Project Settings
```

`UCursorPartFocusScopeComponent`는 값을 소유하지 않고 읽기만 한다.

## 설정 클래스 추가

프로젝트에 이미 공통 Focus 설정용 `UDeveloperSettings` 클래스가 있다면 그 클래스를 사용한다.

없다면 다음 성격의 클래스를 추가한다.

예시:

```cpp
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Beekeeping Sim Focus"))
class BEEKEEPINGSIM_API UBeekeepingSimFocusSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Cursor Part Focus", meta=(ClampMin="0.0"))
    float ScreenEdgeCancelRegionThickness = 64.0f;
};
```

필요 include:

```cpp
#include "Engine/DeveloperSettings.h"
```

Build.cs에 추가 모듈이 필요한지 확인한다.  
일반적으로 `DeveloperSettings` 사용을 위해 모듈 의존성이 필요하면 `BeekeepingSim.Build.cs`에 반영한다.

## Scope 컴포넌트 수정

`UCursorPartFocusScopeComponent`에서 기존 인스턴스별 속성을 제거한다.

제거 대상:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
float ScreenEdgeCancelRegionThickness = 64.0f;
```

목표:

```text
CursorPartFocusScopeComponent Details에서 ScreenEdgeCancelRegionThickness가 개별 값으로 노출되지 않는다.
```

## HandleEdgeCancelClick 수정

`HandleEdgeCancelClick()`은 공통 설정값을 읽어서 사용한다.

개념:

```cpp
const UBeekeepingSimFocusSettings* Settings = GetDefault<UBeekeepingSimFocusSettings>();
const float T = Settings
    ? FMath::Max(0.0f, Settings->ScreenEdgeCancelRegionThickness)
    : 64.0f;
```

요구사항:

```text
Settings가 없어도 안전하게 기본값 64.0f를 사용한다.
음수 값은 0으로 clamp한다.
기존 화면 좌표 판정 로직은 유지한다.
thickness 공급원만 공통 설정으로 변경한다.
```

## 문서 갱신

`.md/USER_UNREAL.md`에 다음 내용을 반영한다.

```text
ScreenEdgeCancelRegionThickness는 개별 CursorPartFocusScopeComponent에서 수정하지 않는다.
Project Settings > Beekeeping Sim Focus > Cursor Part Focus에서 공통값으로 조절한다.
```

만약 프로젝트 설정 UI 등록 대신 `DefaultGame.ini` 설정만 사용한다면, 해당 ini 경로와 설정 예시를 문서화한다.

필요하면 `.md/Architecture/WorldActorsSystem.md`에도 다음 책임 분리를 기록한다.

```text
UBeekeepingSimFocusSettings
- Cursor Part Focus 공통 설정 소유
- ScreenEdgeCancelRegionThickness 제공

UCursorPartFocusScopeComponent
- 공통 설정값을 읽어 외곽 취소 영역 판정에 사용
- Scope별 thickness 값을 소유하지 않음
```

## 검증 기준

- 빌드 성공
- `ScreenEdgeCancelRegionThickness`가 `UCursorPartFocusScopeComponent` Details에서 개별 조절값으로 노출되지 않는다.
- Project Settings 또는 Config에서 `ScreenEdgeCancelRegionThickness`를 한 번만 설정할 수 있다.
- 모든 `UCursorPartFocusScopeComponent`가 동일한 edge cancel thickness 값을 사용한다.
- thickness 값을 변경하면 모든 Part Focus Scope의 외곽 취소 영역 판정에 동일하게 반영된다.
- 기존 edge cancel 클릭 판정 동작은 유지된다.

## QnA 필요 여부

추가 QnA는 필요 없다.

확정 기준:

```text
ScreenEdgeCancelRegionThickness는 Scope별 개별값이 아니라 프로젝트 공통 Focus 설정값이다.
```
