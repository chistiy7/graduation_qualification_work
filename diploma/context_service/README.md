# Сервис захвата контекста из аудио

Вспомогательный инструмент для диплома: **аудио разговора → транскрипт → брифинг агентом в чате**.

## Что делает

1. **Транскрибация** — [faster-whisper](https://github.com/SYSTRAN/faster-whisper) локально (русский по умолчанию).
2. **Брифинг** — в чате Cursor: агент читает `transcript.md` и пишет `context.md` по `prompts/extract_context.md`.
3. **Сохранение сессии** — каталог `03_capture/sessions/YYYYMMDD_HHMM_название/`:
   - `transcript.md` — полный текст и таймкоды;
   - `context.md` — создаёт агент в чате (не CLI);
   - `notes.md` — опционально, если передан `--notes`.

## Установка

```bash
cd "/Users/savok/Desktop/UNI/диплом/diploma"

python3 -m venv .venv
source .venv/bin/activate

python3 -m pip install -r requirements-context.txt
```

Для форматов `mp3`, `m4a`, `webm`, `mp4` нужен **FFmpeg**:

```bash
brew install ffmpeg
```

## Запуск

Из каталога **`diploma`**:

```bash
python3 -m context_service process "./recording.m4a" -t "Созвон"
```

Несколько дорожек:

```bash
python3 -m context_service process part1.m4a part2.m4a -t "Созвон" --merge
```

## Брифинг после транскрипции

В чате Cursor:

```
сделай брифинг по transcript.md
```

Агент использует `context_service/prompts/extract_context.md` и сохраняет `context.md` рядом с транскриптом.

## Очередь правок

```bash
python3 -m context_service fix
python3 -m context_service process "./recording.m4a" -t "Созвон" --fix
```

## Переменные окружения

| Переменная | Назначение |
|------------|------------|
| `WHISPER_MODEL` | `tiny`, `base`, `small`, `medium` |
| `WHISPER_DEVICE` | `auto`, `cpu`, `cuda` |
| `WHISPER_COMPUTE_TYPE` | `int8` на CPU, `float16` на GPU |
