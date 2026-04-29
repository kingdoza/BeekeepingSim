# Codex Agent — 행동 원칙

> Legacy note: 이 파일은 리팩토링 전 에이전트 지침 기록이다. 현재 작업 기준은 프로젝트 루트의 `AGENTS.md`, `.md/0_ARCHITECTURE.md`, `.md/Architecture/*.md`를 우선한다. 아래 내용 중 `Source/ARCHITECTURE.md`를 정본으로 보는 규칙은 더 이상 현재 구조의 기준이 아니다.

---

## 최우선 규칙

명시적인 지시 없이 파일을 생성, 수정, 삭제하지 않는다.
단, 설명을 위한 간단한 코드 예시는 허용된다.

---

## 세션 시작 시 기본 절차

1. .md/ARCHITECTURE.md를 먼저 읽고 현재 설계를 파악한다
2. .md/QNA_ARCHITECTURE.md가 존재할 경우 함께 읽고 최신 사용자 답변을 반영한다
3. 프로젝트 디렉토리 구조를 파악한다
4. 작업과 관련된 핵심 파일을 우선적으로 탐색한다
5. 필요 시 범위를 점진적으로 확장하여 추가 파일을 분석한다
6. 기존 클래스 구조, 변수명, 함수명, 코딩 컨벤션을 파악한다
7. 파악한 내용을 요약해서 보고한다
8. 지시를 기다린다

※ 전체 파일을 무조건 읽지 않는다. 필요한 범위만 분석한다.
※ ARCHITECTURE.md의 내용이 기존 기억보다 우선한다.
※ QNA_ARCHITECTURE.md의 사용자 답변은 설계 판단의 최우선 기준이다.

---

## 분석 단계 규칙

- 분석 중 발견한 구조적 문제나 개선 포인트는 요약하여 보고할 수 있다
- 단, 구현 제안이나 코드 작성은 지시 이후에 수행한다
- 불확실한 내용은 반드시 "추측"으로 명시한다
- "추측"이 필요한 경우 우선적으로 QNA_ARCHITECTURE.md 질문 생성으로 대체한다

---

## 코드 작성 금지 조건

아래 상황에서는 코드 작성을 금지한다.

- 명시적인 코드 작성 지시가 없을 때
- 요구사항이 불명확할 때
- 설계가 애매하거나 선택지가 존재할 때
- 기존 코드와 충돌 가능성이 있을 때

---

## 코드 작성 지시를 받은 경우

1. 작업 범위를 정리하여 보고한다
2. 영향 받는 파일 목록을 제시한다
3. 필요한 경우 추가로 확인할 파일을 요청한다
4. 승인 후 작업을 수행한다
5. 작업 완료 후 변경 사항을 보고한다

---

## 코딩 원칙

- 기존 프로젝트의 코딩 컨벤션을 그대로 따른다
- 임의로 아키텍처를 변경하지 않는다
- 기존 코드 삭제는 반드시 사전 승인 후 수행한다
- 파악되지 않은 영역이 있을 경우 작업 전에 보고한다

---

## Unreal Rules

- UObject의 GC 및 Lifecycle을 고려한다
- Actor는 상태 중심, 기능은 Component로 분리한다
- Tick 사용을 최소화하고 Event / Delegate 기반을 우선한다
- Blueprint는 UI 및 단순 로직에 한정한다
- 핵심 로직은 C++로 구현한다

---

## Unreal Architecture

- Subsystem (GameInstance, World 등)을 우선적으로 고려한다
- Module 단위 구조를 유지하고 의존성을 최소화한다
- BeginPlay / Initialize / EndPlay 흐름을 명확히 구분한다
- Gameplay 시스템은 확장성을 고려하여 설계한다

---

## Build

- Unreal Build Tool을 사용한다
- .uproject 기준으로 빌드한다

---

## Structure

- Source/ 아래 모듈 단위 구조를 유지한다
- Public / Private 폴더 구조를 유지한다

---

## Documentation Rules

- 구조 변경이 발생하면 ARCHITECTURE.md를 최신 상태로 업데이트한다
- 변경된 부분만 반영하고 전체를 다시 작성하지 않는다
- 코드 구조 요약은 간결하게 유지한다
- ARCHITECTURE.md 반영 기준은 다음 경로로 제한한다:
  - Source/BeekeepingSim/Public
  - Source/BeekeepingSim/Private
- 위 경로 외의 파일 변경은 ARCHITECTURE.md에 반영하지 않는다

---

## Implementation Prompt Rules

- 구현 작업 완료 후, 해당 작업을 재현할 수 있는 프롬프트를 작성한다
- 프롬프트는 Source/PROMPT_IMPLEMENTATION.md 파일에 기록한다

---

## Implementation Prompt 작성 규칙

프롬프트는 다음 내용을 반드시 포함해야 한다:

1. 작업 목표
2. 대상 파일 목록
3. 구현 요구사항
4. 구현 원칙
5. 세부 기능 설명
6. Unreal 관련 제약 조건
7. 출력 요구사항 (보고 형식 포함)

---

## Implementation Prompt 작성 방식

- 실제 Codex 또는 AI에게 그대로 전달 가능한 형태로 작성한다
- 불필요한 설명 없이 실행 가능한 명령 형태로 작성한다
- 기존 Source/ARCHITECTURE.md 구조를 반영한다
- 변경된 기능만 포함한다 (전체 재작성 금지)

---

## Implementation Prompt 범위 제한

- 프롬프트는 다음 경로 기준으로 작성한다:
  - Source/BeekeepingSim/Public
  - Source/BeekeepingSim/Private
- 외부 코드, 엔진 코드, 테스트 코드는 포함하지 않는다

---

## 완료 조건 (프롬프트 관련)

구현 작업은 다음 조건을 만족해야 완료로 간주한다:

- Source/ARCHITECTURE.md가 최신 상태로 반영되었을 것
- Source/PROMPT_IMPLEMENTATION.md가 생성 또는 업데이트되었을 것
- 프롬프트가 실제 구현을 재현 가능한 수준일 것

---

## Architecture Reference Rules

- 모든 작업은 Source/ARCHITECTURE.md를 기준으로 수행한다
- 작업 시작 시 반드시 최신 상태의 ARCHITECTURE.md를 다시 읽는다
- 기존 컨텍스트보다 문서 내용을 우선한다
- 설계와 코드가 불일치할 경우 먼저 보고한다
- ARCHITECTURE.md의 구조 정의는 다음 경로의 코드 기준으로 해석한다:
  - Source/BeekeepingSim/Public
  - Source/BeekeepingSim/Private

---

## Architecture QnA Rules

설계에 불명확하거나 선택지가 존재하는 경우,
즉시 구현을 진행하지 않고 질문을 생성한다.

- 질문은 Source/QNA_ARCHITECTURE.md 파일에 기록한다
- 질문 없이 추측으로 구현하는 것을 금지한다

---

## QnA 작성 규칙

질문은 반드시 다음 구조를 따른다:

### [질문 항목]

1. 질문 제목
- 질문 내용
- 필요한 이유
- 선택지

---

## 선택지 작성 규칙 (중요)

선택지가 있는 경우 반드시 옵션 형태로 제공한다:

예시:

- 옵션 A: Component 기반 처리
- 옵션 B: Character 내부 처리
- 옵션 C: 별도 시스템으로 분리

사용자가 선택할 수 있도록 명확하게 제시한다.

---

## 질문 유형

다음 상황에서는 반드시 질문을 생성한다:

- 설계 해석이 여러 방향으로 가능한 경우
- 구현 방식에 선택지가 존재하는 경우
- 성능 vs 구조 트레이드오프가 있는 경우
- 기존 구조와 충돌 가능성이 있는 경우

---

## 사용자 응답 처리 규칙

사용자가 Source/QNA_ARCHITECTURE.md에 답변을 작성한 경우:

- 해당 문서를 다시 읽고 최신 상태를 반영한다
- 사용자 응답을 기존 설계보다 우선한다
- 응답 기반으로 설계를 확정한다
- 필요 시 Source/ARCHITECTURE.md에 반영한다

---

## 구현 진행 조건

다음 조건을 만족해야 구현을 진행할 수 있다:

1. 설계가 명확한 경우
2. QNA_ARCHITECTURE.md의 질문에 대해 사용자 답변이 완료된 경우

---

## 금지 사항

- 추측 기반 구현 금지
- QNA_ARCHITECTURE.md 무시 금지
- 사용자 선택 무시 금지

---

## 보고 형식

[상태] 파악 완료 / 대기 중 / 작업 중 / 완료  
[요약] 핵심 내용 정리  
[다음] 필요한 지시 또는 대기 상태
