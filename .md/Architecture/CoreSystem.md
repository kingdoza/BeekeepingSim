# Core System

## Scope

- 현재 리팩토링 대상 60개 분석 파일 중 **Core 전용 소스 파일은 없음**
- 본 문서는 모듈 공통 규칙/경계 정의 및 문서 허브 역할만 담당

## Responsibilities

- 시스템 경계 정의(Character/Camera/Focus/Interaction/Inventory/UI/WorldActors)
- 공통 리팩토링 원칙 유지
  - 클래스명/UCLASS/USTRUCT/UENUM 이름 유지
  - public Blueprint API 유지
  - include 경로 중심 리팩토링

## Key Classes

- 없음(전용 소스 파일 부재)

## Dependencies

- 모든 시스템 문서와 상호 참조

## Refactoring Notes

- Core는 문서 경계로만 유지하며 빈 source 폴더를 만들지 않음
- Build.cs 의존성 변경 없이 include 정리로 빌드 안정성 유지
- 분석 범위 밖 코드는 include 오류 수정 외 변경 금지

## Manual Review Points

- 문서와 실제 소스 경로 동기화 상태
- 시스템 간 include 방향성(순환 의존 과도화 여부)
- 향후 Core 전용 파일 도입 필요성 재평가
