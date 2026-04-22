다음 작업을 Unreal Editor 내부에서 단계별로 도와줘.

현재 전제:
- C++ 포커스 시스템이 이미 구현되어 있다.
- `ABeekeeperCharacter` 는 `UBeekeeperFocusComponent` 를 가진다.
- `UBeekeeperFocusComponent` 는 아래 데이터를 제공한다.
  - `OnFocusPromptChanged`
  - `OnFocusRuleChanged`
  - `GetCurrentPromptData()`
  - `HasFocusTarget()`
- `FFocusPromptData` 구조에는 아래 값이 있다.
  - `bIsValid`
  - `DisplayName`
  - `InteractionKeyText`
- 포커스 대상은 `UFocusTargetComponent` 에서 `DisplayName`, `InteractionKeyText` 를 제공한다.
- 목표는 포커스 진입 시 화면에 `F` 상호작용 팝업 UI를 띄우고, 포커스 이탈 시 숨기는 것이다.
- UI는 C++에서 직접 그리지 않고 Blueprint 위젯이 포커스 데이터를 받아 표시하는 구조로 만든다.

목표:
1. 상호작용 팝업용 UMG 위젯 Blueprint를 만든다.
2. `DisplayName` 과 `InteractionKeyText` 를 표시한다.
3. 포커스 대상이 있으면 표시하고, 없으면 숨긴다.
4. 가능하면 간단한 등장/사라짐 애니메이션도 넣는다.
5. 실제 플레이어 화면에 붙도록 연결한다.

작업 요청 1: 위젯 Blueprint 생성
1. 새 Widget Blueprint 하나를 생성해줘.
   - 이름: `WBP_FocusPrompt`
2. 저장 위치는 가능하면 `Content/Beekeeper/UI/` 로 해줘.
3. 위젯은 화면 중앙 근처 또는 하단 중앙에 놓이되, 1인칭 포커스 UI로 자연스러운 위치를 잡아줘.
4. 레이아웃은 너무 복잡하지 않게 해줘.
   - 키 표시 영역
   - 상호작용 대상 이름 영역
5. 예시 표시 형태는 아래 느낌으로 해줘.
   - `[F] Open Beehive`
   - 또는 왼쪽에 `F` 배지, 오른쪽에 대상 이름

작업 요청 2: 위젯 내부 구성
1. 아래 텍스트/시각 요소를 만들어줘.
   - 키 텍스트용 TextBlock
   - 대상 이름용 TextBlock
   - 배경용 Border 또는 SizeBox
2. 기본 상태에서는 위젯이 숨겨져 있어도 된다.
3. 포커스가 있을 때만 보이도록 만들고, 포커스가 없을 때는 `Collapsed` 또는 `Hidden` 처리해줘.
4. 키 텍스트가 비어 있지 않으면 예: `F`
5. 대상 이름은 `DisplayName`
6. 필요하면 기본 프리뷰 값도 넣어줘.

작업 요청 3: 위젯 업데이트 함수 만들기
1. `WBP_FocusPrompt` 에서 포커스 데이터를 받아 갱신하는 함수 하나를 만들어줘.
   - 예: `UpdateFocusPrompt`
2. 입력 파라미터는 아래 기준으로 해줘.
   - `bIsValid` 또는 `FFocusPromptData`
3. 이 함수에서:
   - 유효하면 키 텍스트와 대상 이름 갱신
   - 유효하지 않으면 UI 숨김
4. 가능하면 `FFocusPromptData` 구조체를 직접 받는 방식으로 구성해줘.

작업 요청 4: 표시/숨김 애니메이션
1. 등장 시 Fade In 또는 약한 Scale In 정도의 가벼운 애니메이션을 넣어줘.
2. 사라질 때도 짧은 Fade Out 정도만 넣어줘.
3. 너무 과한 모션은 피하고 실제 플레이용 UI로 만들어줘.

작업 요청 5: 화면 연결
1. `ABeekeeperCharacter` 또는 플레이어 기준으로 이 위젯이 화면에 추가되도록 연결 방법을 제안해줘.
2. 가능하면 Blueprint 단계에서는 아래 흐름으로 설명해줘.
   - BeginPlay 에서 위젯 생성
   - Viewport 에 추가
   - `BeekeeperFocus` 컴포넌트 참조
   - `OnFocusPromptChanged` 델리게이트 바인딩
   - 델리게이트 수신 시 `UpdateFocusPrompt` 호출
3. 현재 코드 구조상 가장 자연스러운 연결 주체가 누구인지 같이 제안해줘.
   - `BeekeeperCharacter` BP
   - 또는 `BeekeeperController` BP
4. 가능하면 현재 구조에서는 어떤 쪽이 더 적합한지 이유도 짧게 설명해줘.

작업 요청 6: 테스트 시나리오
1. `ABeehive` 기반 포커스 대상 Blueprint 하나를 기준으로 테스트 흐름을 제안해줘.
2. 테스트 항목:
   - 벌통을 바라보면 `F` 팝업 표시
   - 벌통에서 시선을 떼면 팝업 숨김
   - 다른 포커스 대상으로 바뀌면 텍스트 갱신
3. `DisplayName`, `InteractionKeyText` 가 비어 있을 때 표시가 어떻게 되는지도 같이 점검해줘.

디자인 방향:
- 실제 플레이용 1인칭 상호작용 UI
- 너무 크지 않게
- 배경은 반투명 dark panel 정도
- 키 표시는 읽기 쉬운 강조 배지 형태
- 전체는 미니멀하고 빠르게 읽히는 느낌

최종 출력 형식:
- 생성한 위젯 Blueprint 이름
- 저장 경로
- 위젯에 포함된 주요 위젯 요소 목록
- 갱신 함수 이름과 입력 파라미터
- 화면 연결 방법 요약
- 테스트 체크리스트
