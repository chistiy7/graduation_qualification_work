# Сервис захвата контекста из аудио

Вспомогательный инструмент для диплома: **аудио разговора → транскрипт → структурированные выводы** (требования к ПО, термины, метрики, открытые вопросы). Основная программа диплома (планировщик лаборатории) — отдельный проект; этот сервис помогает не терять договорённости с научруком и экспертами лаборатории.

## Что делает

1. **Транскрибация** — [faster-whisper](https://github.com/SYSTRAN/faster-whisper) локально (русский по умолчанию).
2. **Брифинг (контекст)** — `context.md` по единой структуре `prompts/extract_context.md`.
3. **Сохранение сессии** — каталог `03_capture/sessions/YYYYMMDD_HHMM_название/`:
   - `transcript.md` — полный текст и таймкоды;
   - `context.md` — брифинг по промпту.

## Установка

На macOS команды `python` и `pip` часто **отсутствуют** — используйте `python3` и `python3 -m pip`, либо виртуальное окружение.

```bash
cd "/Users/savok/Desktop/UNI/диплом/diploma"

python3 -m venv .venv
source .venv/bin/activate

python3 -m pip install -r requirements-context.txt
```

Для форматов вроде `mp3`, `m4a`, `webm`, `mp4` нужен **FFmpeg**:

```bash
ffmpeg -version
# если нет:
brew install ffmpeg
```

После `source .venv/bin/activate` в терминале появится `(.venv)` — тогда можно писать `python` и `pip` без `3`.

На macOS при первом запуске Whisper скачает модель (несколько сотен МБ для `small`).

## Запуск

Запускать нужно из каталога **`diploma`**, не из `context_service/`:

```bash
cd "/Users/savok/Desktop/UNI/диплом/diploma"
source .venv/bin/activate

python3 -m context_service info

python3 -m context_service process \
  "./New-Recording-6.mp3" \
  --title "Консультация с научруком — маршруты испытаний"
```

Несколько дорожек (склейка в одну, затем обработка):

```bash
python3 -m context_service process \
  part1.m4a part2.m4a part3.webm \
  -t "Созвон" \
  --merge \
  --context
```

Порядок файлов в командной строке = порядок склейки.

Транскрипт + брифинг (`transcript.md` и `context.md`) создаются по умолчанию:

```bash
python3 -m context_service process "./New-Recording-6.mp3" -t "Созвон"
```

Флаг `--context` можно указывать явно, результат тот же:

```bash
python3 -m context_service process "./New-Recording-6.mp3" -t "Созвон" --context
```

Сначала сохраняется транскрипт, затем создаётся `context.md` по структуре `prompts/extract_context.md`.

Только транскрипт:

```bash
python3 -m context_service process "./New-Recording-6.mp3" -t "Созвон" --transcript-only
```

## Очередь правок по меткам `#todo` / `#`

Если вы оставляете в файлах проекта пометки `#todo` (или ставите `#` в конце строки как маркер правки), можно собрать единый список:

```bash
python3 -m context_service fix
```

Отчёт сохранится в `00_context/fix_queue.md`.

Также можно обновлять очередь автоматически после транскрибации:

```bash
python3 -m context_service process "./New-Recording-6.mp3" -t "Созвон" --transcript-only --fix
```

## Брифинг по готовой транскрипции

Если вы уже сделали `--transcript-only` и у вас есть `transcript.md`, можно получить брифинг отдельно:

```bash
python3 -m context_service brief \
  "03_capture/sessions/20260527_150430_созвон/transcript.md" \
  -t "Созвон"
```

Без активации venv (одной строкой):

```bash
cd "/Users/savok/Desktop/UNI/диплом/diploma"
.venv/bin/python3 -m context_service process "./New-Recording-6.mp3" -t "Созвон"
```

## После обработки

Перенесите проверенные пункты из `context.md` в:

- `00_context/state.md` — факты, которые нельзя повторять в главах;
- `01_chapters/` — формулировки для текста;
- `00_context/glossary.md` — новые термины.

## Переменные окружения

| Переменная | Назначение |
|------------|------------|
| `WHISPER_MODEL` | `tiny`, `base`, `small`, `medium` |
| `WHISPER_DEVICE` | `auto`, `cpu`, `cuda` |
| `WHISPER_COMPUTE_TYPE` | `int8` на CPU, `float16` на GPU |

## Ограничения

- Качество транскрипта зависит от шума и микрофона.
- Брифинг строится по транскрипту; спорные формулировки проверяйте по записи.
- Для защиты диплома этот сервис обычно **не входит** в демонстрационную программу; его можно описать во вспомогательном разделе или не включать в отчёт, если не требуется методичка.
