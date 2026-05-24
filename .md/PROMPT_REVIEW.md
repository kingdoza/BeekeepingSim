# 구현 리뷰 프롬프트: Beekeeper Flashlight Toggle (`T`)

## 우선순위

1. High: 책임 분리 준수 (`ABeekeeperCharacter` 입력 라우팅 / `UBeekeeperFlashlightComponent` 상태·조명 관리)
2. High: 입력/토글 동작 정확성 (`FlashlightToggleAction` Started 바인딩, focus lock 무관 토글)
3. High: 카메라 부착 및 로컬 정책 준수 (1인칭 카메라 추종, replication 미도입)
4. Medium: 기존 입력/시스템 회귀 없음
5. Medium: 문서/Editor 수동 작업 안내 정합성

---

## 리뷰 대상 파일

### 코드
- `Source/BeekeepingSim/Public/Character/BeekeeperFlashlightComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperFlashlightComponent.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`

### 문서
- `.md/Architecture/CharacterSystem.md`
- `.md/0_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`

---

## 핵심 검증 항목

### High 1: 책임 분리
- `ABeekeeperCharacter`가 손전등 상태를 직접 소유하지 않고 입력에서 `ToggleFlashlight()`만 호출하는가
- `UBeekeeperFlashlightComponent`가 아래를 담당하는가:
  - `USpotLightComponent` 소유
  - 내부 on/off 상태
  - 조명 설정 적용(강도/거리/콘각/색/그림자)

### High 2: 입력/토글
- `FlashlightToggleAction` property가 추가되었는가
- `SetupPlayerInputComponent()`에서 null check 후 `ETriggerEvent::Started`로 바인딩하는가
- `FlashlightToggleInput()`이 `bIsFocusInteractionInputLocked` 조건으로 차단되지 않는가
- `SetFlashlightEnabled(true/false)`가 실제 `SpotLight` visibility와 동기화되는가

### High 3: 카메라 부착 및 로컬 정책
- `BeginPlay()`에서 손전등이 `FirstPersonCamera`에 부착되는가
- 시점 회전에 따라 손전등 방향이 카메라를 추종하는 구조인가
- replication/RPC 코드가 추가되지 않았는가
- 손전등이 gameplay authority/AI 감지 경로로 연결되지 않았는가

### Medium 1: 회귀 방지
- 기존 입력 property (`PartFocusF/R/C`, focus/hotbar/sprint/jump/move/look) rename/delete 없음
- 기존 Focus/Inventory/UI/Environment 로직 변경 없음
- `#include "Public/..."` 패턴 없음

### Medium 2: 문서/수동 작업
- Character 문서에 flashlight 컴포넌트/흐름/검증 포인트가 반영되었는가
- `USER_UNREAL`에 아래 수동 작업이 명시되었는가:
  - `IA_FlashlightToggle` 생성
  - Mapping Context에 추가 + `T` 키 매핑
  - `BP_BeekeeperCharacter.FlashlightToggleAction` 할당
  - BP compile/save

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`

- 검색:
  - `rg "BeekeeperFlashlight|FlashlightToggleAction|FlashlightToggleInput|ToggleFlashlight|SetFlashlightEnabled|InitializeFlashlightAttachment" Source/BeekeepingSim/Public/Character Source/BeekeepingSim/Private/Character -n`
  - `rg "bIsFocusInteractionInputLocked" Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp -n`
  - `rg "GetLifetimeReplicatedProps|DOREPLIFETIME|Server|Client|NetMulticast" Source/BeekeepingSim/Public/Character Source/BeekeepingSim/Private/Character -n`
  - `rg '#include \"Public/' Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`

---

## 수동 검증 가이드 (PIE)

1. `T` 입력 시 손전등 On
2. 다시 `T` 입력 시 Off
3. 마우스 시점 이동 시 손전등 방향이 카메라와 함께 회전
4. FocusEngaged/입력 잠금 상태에서도 `T` 토글 동작
5. 이동/점프/스프린트/focus/hotbar 기존 입력 정상 동작
6. `BeekeeperFlashlight` Details 값 변경 시 밝기/거리/콘각/그림자 반영

---

## 리뷰 결과 출력 형식

1. Findings (High / Medium / Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail

