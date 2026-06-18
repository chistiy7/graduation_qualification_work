from __future__ import annotations

import re
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

from .config import Settings
from .log import progress
from .transcribe import transcribe_audio


@dataclass
class SessionResult:
    session_dir: Path
    transcript_path: Path


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

    if notes.strip():
        notes_path = session_dir / "notes.md"
        notes_path.write_text(notes.strip() + "\n", encoding="utf-8")
        progress(f"Сохранён: {notes_path.name}")

    return SessionResult(
        session_dir=session_dir,
        transcript_path=transcript_path,
    )
