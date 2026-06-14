# 구현 수정 프롬프트: 왕롱 포획/분봉 최종 완료 조건 리뷰 Finding

## 우선순위

1. Minor: `WorldActorsSystem.md` 본문에 남은 이전 captured 조건 문구 정리

## 발견 문제

### 1. 분봉 본진 본문 설명이 최신 최종 완료 조건과 충돌

- 대상 파일: `.md/Architecture/WorldActorsSystem.md`
- 문제: `ABeeSwarmClusterActor` 본문 설명에 `CapturedBeeAmount >= SpawnAmount` 또는 잔여 벌 수 0이면 captured로 전환된다는 이전 조건이 남아 있다. 현재 설계/구현은 이 시점에 `bBeesCaptured`만 true가 되고, `AliveRadius=0`, BeeCarrier use-area 비활성화, descriptor rebuild만 수행한다. 최종 `bCaptured` 및 `ReceiveSwarmCaptured`는 `bBeesCaptured && bQueenCaptured`일 때만 1회 발생해야 한다.
- 영향: 같은 정본 문서 안에서 본문 설명과 2026-06-14 업데이트 설명이 충돌해 후속 구현/리뷰가 오래된 완료 조건을 따를 수 있다.
- 수정 방향: 해당 본문을 벌 포획 완료(`bBeesCaptured`)와 최종 분봉 완료(`bCaptured`)의 분리된 조건으로 정리한다.

## 검증 방법

```powershell
rg -n "CapturedBeeAmount >= SpawnAmount|bBeesCaptured|bQueenCaptured|ReceiveSwarmCaptured" .md/Architecture/WorldActorsSystem.md .md/0_ARCHITECTURE.md
```

```powershell
git diff --check -- .md/Architecture/WorldActorsSystem.md .md/PROMPT_IMPLEMENTATION_R.md
```

## 문서 반영 필요 여부

- 필요: `.md/Architecture/WorldActorsSystem.md`
