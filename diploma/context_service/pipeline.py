from __future__ import annotations

import re
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

from .config import Settings
from .local_brief import build_local_brief_markdown
from .log import progress
from .transcribe import Transcript, transcribe_audio


@dataclass
class SessionResult:
    session_dir: Path
    transcript_path: Path
    context_path: Path | None
    index_path: Path


def _slugify(text: str, max_len: int = 48) -> str:
    text = text.strip().lower()
    text = re.sub(r"[^\w\s-]", "", text, flags=re.UNICODE)
    text = re.sub(r"[\s_-]+", "-", text)
    text = text.strip("-")
    if not text:
        text = "session"
    return text[:max_len]


def process_audio(
    audio_path: Path,
    *,
    title: str,
    settings: Settings,
    with_context: bool = True,
    notes: str = "",
    language: str | None = "ru",
) -> SessionResult:
    audio_path = audio_path.expanduser().resolve()
    if not audio_path.is_file():
        raise FileNotFoundError(f"Аудиофайл не найден: {audio_path}")

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    slug = _slugify(title)
    session_dir = settings.output_dir / f"{stamp}_{slug}"
    progress(f"Сессия (после транскрибации): {session_dir}")

    transcript = transcribe_audio(
        audio_path,
        model=settings.whisper_model,
        device=settings.whisper_device,
        compute_type=settings.whisper_compute_type,
        language=language,
    )

    session_dir.mkdir(parents=True, exist_ok=True)

    transcript_path = session_dir / "transcript.md"
    transcript_path.write_text(transcript.to_markdown(title), encoding="utf-8")
    progress(f"Сохранён: {transcript_path.name}")

    context_path: Path | None = None
    if with_context:
        if notes.strip():
            progress("Примечание: --notes игнорируется (LLM отключён).")
        progress("Создаю context.md по prompts/extract_context.md…")
        context_md = build_local_brief_markdown(
            title=title, transcript_text=transcript.full_text
        )
        context_path = session_dir / "context.md"
        context_path.write_text(context_md, encoding="utf-8")
        progress(f"Сохранён: {context_path.name}")

    return SessionResult(
        session_dir=session_dir,
        transcript_path=transcript_path,
        context_path=context_path,
        index_path=session_dir / "session.json",
    )


def brief_from_transcript_markdown(
    transcript_md_path: Path,
    *,
    title: str,
    settings: Settings,
    notes: str = "",
) -> Path:
    transcript_md_path = transcript_md_path.expanduser().resolve()
    if not transcript_md_path.is_file():
        raise FileNotFoundError(f"Файл транскрипта не найден: {transcript_md_path}")

    raw = transcript_md_path.read_text(encoding="utf-8", errors="replace")
    transcript_text = _extract_full_text_from_transcript_md(raw)
    if not transcript_text.strip():
        raise ValueError(
            "Не удалось извлечь «Полный текст» из transcript.md (получилось пусто)."
        )

    if notes.strip():
        progress("Примечание: --notes игнорируется (LLM отключён).")
    progress("Создаю context.md по prompts/extract_context.md…")
    context_md = build_local_brief_markdown(title=title, transcript_text=transcript_text)

    out_path = transcript_md_path.parent / "context.md"
    out_path.write_text(context_md, encoding="utf-8")
    return out_path


def _extract_full_text_from_transcript_md(md: str) -> str:
    """
    Достаёт секцию «## Полный текст» из markdown, который генерирует Transcript.to_markdown().

    Формат:
      ## Полный текст
      <текст...>
      ## По сегментам
      ...
    """
    marker_start = "\n## Полный текст\n"
    marker_end = "\n## По сегментам\n"

    if marker_start not in md:
        return ""
    after = md.split(marker_start, 1)[1]
    if marker_end in after:
        return after.split(marker_end, 1)[0].strip()
    return after.strip()

