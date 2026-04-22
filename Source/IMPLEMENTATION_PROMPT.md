다음 문서를 먼저 읽고 그 규칙을 기준으로 작업해:
- Source/CODEX_IMPLEMENTATION.md
- Source/ARCHITECTURE.md

다음 작업을 수행해.

[목표]
Beekeeper 캐릭터가 화면 중앙으로 포커스 대상을 바라볼 때 재사용 가능한 포커스 시스템이 동작하도록 구현한다.
이 시스템은 외곽선 표시, `F` 상호작용 팝업 UI, 포커스 진입/이탈 이벤트, 포커스 대상별 허용 아이템 태그 기반 핫바 활성/비활성 제어를 지원해야 한다.
벌통 액터는 이 공통 시스템을 사용하는 예시 대상 중 하나로 동작해야 한다.

[작업 대상 범위]
오직 아래 경로의 실제 C++ 파일만 수정 또는 생성 대상으로 고려한다.
- Source/BeekeepingSim/Public
- Source/BeekeepingSim/Private

[설계 기준]
- 현재 `Source/ARCHITECTURE.md` 기준으로 `ABeekeeperCharacter`, `ABeekeeperController`, `UBeekeeperMovementComponent`, `UBeekeeperCameraShakeComponent` 가 존재한다
- 이번 작업은 위 구조를 유지한 채 확장한다
- Actor는 상태 중심, 기능은 Component로 분리한다
- 포커스 시스템은 벌통 전용 하드코딩이 아니라 재사용 가능한 구조로 만든다

[예상 대상 파일]
- 기존 파일
  - Source/BeekeepingSim/Public/BeekeeperCharacter.h
  - Source/BeekeepingSim/Private/BeekeeperCharacter.cpp
- 신규 파일 후보
  - Source/BeekeepingSim/Public/BeekeeperFocusComponent.h
  - Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp
  - Source/BeekeepingSim/Public/FocusTargetComponent.h
  - Source/BeekeepingSim/Private/FocusTargetComponent.cpp
  - Source/BeekeepingSim/Public/Beehive.h
  - Source/BeekeepingSim/Private/Beehive.cpp
  - Source/BeekeepingSim/Public/FocusInteractable.h
  - Source/BeekeepingSim/Private/FocusInteractable.cpp
- 필요 시 위 목록을 작업 시작 보고에서 조정한다

[구현 요구사항]
1. 플레이어 캐릭터는 화면 중앙점 기준으로 포커스 대상을 감지해야 한다
2. 포커스 대상은 재사용 가능한 구조여야 한다
3. 포커스 진입 시:
   - 대상 외곽선 머터리얼 또는 외곽선용 CustomDepth 표시가 활성화되어야 한다
   - UI에 `F` 상호작용 팝업이 뜰 수 있도록 데이터가 제공되어야 한다
4. 포커스 이탈 시:
   - 외곽선 표시가 꺼져야 한다
   - UI에 포커스 정보가 내려가야 한다
5. 포커스 진입과 포커스 아웃(`ESC`, `F`) 시마다 대상별 특정 행동이 발생할 수 있어야 한다
6. 벌통 예시에서는 포커스 진입/이탈에 따라 뚜껑 열기/닫기 같은 행동을 수행할 수 있어야 한다
7. 포커스 대상은 핫바 아이템 필터링 규칙을 제공해야 한다
8. 아이템 필터링은 슬롯 기준이 아니라 아이템 태그 기준으로 동작해야 한다
9. 필터링 규칙은 `FGameplayTagContainer AllowedItemTags` 로 구현한다
10. `AllowedItemTags` 가 비어 있으면 모든 아이템 비활성화로 처리한다
11. 모든 아이템 활성화는 `AllowedItemTags` 에 모든 아이템의 최상위 태그가 포함된 경우로 처리한다
12. 같은 포커스 대상 C++ 클래스라도 실제 필터 규칙은 Blueprint 단위에서 자유롭게 편집 가능해야 한다
13. 포커스 대상이 없는 경우 핫바는 기본 상태로 복구 가능해야 한다
14. null 체크와 상태 전이 체크를 포함한다
15. 추측이 필요한 내용은 반드시 "추측"이라고 명시한다

[아이템 필터링 정책]
- 포커스 대상은 아래와 같은 구조 또는 동등한 구조를 사용한다

```cpp
USTRUCT(BlueprintType)
struct FFocusItemRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Focus")
	FGameplayTagContainer AllowedItemTags;
};
```

- 핫바 슬롯은 고정 아이템 구성이 아니다
- 플레이어가 아이템을 이동하거나 외부 창고에 보관할 수 있다
- 따라서 슬롯 인덱스로 필터링하지 말고, 각 슬롯에 현재 들어있는 아이템의 태그를 `AllowedItemTags` 와 비교해 활성/비활성 여부를 계산한다
- 비교 규칙은 다음을 따른다
  - `AllowedItemTags` 가 비어 있으면 모든 아이템 비활성화
  - `AllowedItemTags` 에 모든 아이템의 최상위 태그가 있으면 모든 아이템 활성화
  - 그 외에는 아이템의 태그 컨테이너가 `AllowedItemTags` 와 `HasAny` 를 만족할 때만 활성화

[포커스 시스템 권장 구조]
1. `UBeekeeperFocusComponent`
   - `ABeekeeperCharacter` 에 부착
   - 화면 중앙 라인트레이스로 현재 포커스 대상 탐지
   - 이전 대상과 현재 대상 비교
   - 포커스 진입/이탈 처리
   - `F`, `ESC` 입력과 포커스 상태 연결
2. `UFocusTargetComponent`
   - 포커스 가능한 액터에 부착
   - 외곽선 표시 대상 메시 참조 관리
   - UI 표시용 이름/키 텍스트 제공
   - `FFocusItemRule` 보유
   - Blueprint에서 `AllowedItemTags` 편집 가능
3. `IFocusInteractable`
   - 대상 액터별 고유 행동 진입점 제공
   - 예: `OnFocusEnter`, `OnFocusExit`, `OnFocusConfirm`
4. `ABeekeeperBeehiveActor`
   - 벌통 예시 액터
   - `UFocusTargetComponent` 를 소유
   - 포커스 진입/이탈 시 뚜껑 여닫기 동작 수행 가능

[상세 기능 요구사항]
- `ABeekeeperCharacter` 또는 관련 컴포넌트가 포커스 입력을 받는다
- 중앙 라인트레이스로 포커스 대상 액터를 찾는다
- 포커스 대상이 바뀌면:
  - 이전 대상 `OnFocusExit`
  - 이전 대상 외곽선 끄기
  - 새 대상 외곽선 켜기
  - 새 대상 `OnFocusEnter`
  - 새 대상의 `FFocusItemRule` 을 핫바 시스템에 전달
  - UI에 `F` 팝업용 데이터 전달
- `F` 입력 시:
  - 현재 포커스 대상에 상호작용 전달
  - 설계상 `F` 를 포커스 아웃 트리거로도 사용할 수 있도록 구조를 만든다
- `ESC` 입력 시:
  - 현재 포커스 종료
  - `OnFocusExit` 호출
  - 외곽선 해제
  - UI 숨김
  - 핫바 기본 상태 복구

[핫바 연동 요구사항]
- 실제 핫바 시스템이 현재 범위에 없으면, 최소한 아래 둘 중 하나를 구현 가능 구조로 제공한다
  - 핫바에 전달할 수 있는 델리게이트/이벤트
  - 핫바 슬롯 활성 여부를 계산하는 함수
- 포커스 시스템이 직접 UI를 하드코딩하지 말고, 포커스 상태 데이터와 아이템 필터 결과를 외부 시스템이 읽을 수 있게 한다
- 빈 슬롯은 단순 빈 슬롯으로 유지하고, 아이템이 있는 슬롯에 대해서만 활성/비활성 판정을 수행한다

[외곽선 처리 요구사항]
- 외곽선 표시는 메시별로 켜고 끌 수 있어야 한다
- 구현은 `Render CustomDepth` 기반 토글 또는 동등한 범용 방식으로 한다
- 벌통뿐 아니라 다른 포커스 대상에도 재사용 가능해야 한다

[Blueprint 편집 요구사항]
- `AllowedItemTags` 는 C++에 하드코딩하지 않는다
- 각 포커스 대상 Blueprint에서 자유롭게 편집 가능해야 한다
- 같은 C++ 클래스라도 Blueprint마다 다른 `AllowedItemTags` 를 설정 가능해야 한다

[Unreal 관련 제약 조건]
- UObject의 라이프사이클과 GC를 고려한다
- Component 분리를 우선한다
- Tick 사용은 최소화하되, 포커스 탐지에 필요하면 비용을 낮게 유지한다
- Blueprint는 UI 바인딩과 연출 훅 위주로 제한한다
- 핵심 포커스 판정, 상태 전이, 태그 필터링은 C++로 구현한다

[작업 절차]
1. 작업 범위를 요약한다
2. 영향 받는 파일 목록을 제시한다
3. 필요한 신규 파일 생성 계획을 제시한다
4. 승인 후 구현한다
5. 구현 완료 후 변경 사항을 요약한다
6. 구조 변경이 있으면 `Source/ARCHITECTURE.md` 에 변경된 부분만 반영한다
7. 코드 리뷰용 프롬프트를 생성하고 `Source/REVIEW_PROMPT.md` 에 작성한다
8. 요청 시 Unreal용 설정 프롬프트를 `Source/UNREAL_PROMPT.md` 에 작성한다

[출력 요구사항]
- [상태] 완료
- [요약] 수행 내용
- [영향 파일] 변경 파일 목록
- [ARCHITECTURE.md 반영 여부] 반영 내용 요약
- [검토 필요 로직] 인간 검토 필요 항목
- [코드 리뷰 프롬프트] 생성된 리뷰 요청 문장
- [다음] 추가 지시 대기
