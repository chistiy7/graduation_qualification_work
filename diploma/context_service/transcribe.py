from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .log import progress

def _ensure_ffmpeg_available() -> None:
    # faster-whisper декодирует большинство форматов (mp3/m4a/webm/mp4/...) через ffmpeg.
    import shutil

    if shutil.which("ffmpeg"):
        return
    raise RuntimeError(
        "Не найден ffmpeg. Он нужен для декодирования mp3/m4a/webm/mp4 и др.\n"
        "Установка на macOS: brew install ffmpeg\n"
        "Проверка: ffmpeg -version"
    )


@dataclass
class TranscriptSegment:
    start: float
    end: float
    text: str


@dataclass
class Transcript:
    language: str | None
    segments: list[TranscriptSegment]

    @property
    def full_text(self) -> str:
        return "\n".join(s.text.strip() for s in self.segments if s.text.strip())

    def to_markdown(self, title: str) -> str:
        lines = [f"# Транскрипт: {title}", ""]
        if self.language:
            lines.append(f"Язык (авто): `{self.language}`")
            lines.append("")
        lines.append("## Полный текст")
        lines.append("")
        lines.append(self.full_text or "_(пусто)_")
        if self.segments:
            lines.extend(["", "## По сегментам", ""])
            for seg in self.segments:
                start = _format_ts(seg.start)
                end = _format_ts(seg.end)
                lines.append(f"- **{start}–{end}:** {seg.text.strip()}")
        lines.append("")
        return "\n".join(lines)


def _format_ts(seconds: float) -> str:
    total = int(seconds)
    h, rem = divmod(total, 3600)
    m, s = divmod(rem, 60)
    if h:
        return f"{h:d}:{m:02d}:{s:02d}"
    return f"{m:02d}:{s:02d}"


def transcribe_audio(
    audio_path: Path,
    *,
    model: str = "small",
    device: str = "auto",
    compute_type: str = "int8",
    language: str | None = "ru",
) -> Transcript:
    try:
        from faster_whisper import WhisperModel
    except ImportError as exc:
        raise ImportError(
            "Установите зависимости (из каталога diploma, с активным .venv):\n"
            "  python3 -m pip install -r requirements-context.txt"
        ) from exc

    _ensure_ffmpeg_available()

    resolved_device = device
    if device == "auto":
        try:
            import torch

            resolved_device = "cuda" if torch.cuda.is_available() else "cpu"
        except ImportError:
            resolved_device = "cpu"

    size_mb = audio_path.stat().st_size / (1024 * 1024)
    progress(
        f"[1/3] Загрузка модели Whisper «{model}» ({resolved_device}, {compute_type})…"
    )
    progress(
        "      При первом запуске скачивается ~500 МБ — подождите, это не зависание."
    )
    whisper = WhisperModel(model, device=resolved_device, compute_type=compute_type)

    progress(f"[2/3] Транскрибация {audio_path.name} ({size_mb:.1f} МБ)…")
    progress("      На CPU длинная запись может обрабатываться 10–30+ минут.")
    try:
        segments_iter, info = whisper.transcribe(
            str(audio_path),
            language=language,
            vad_filter=True,
        )
    except Exception as exc:
        raise RuntimeError(
            "Не удалось прочитать/декодировать файл. "
            "Проверьте, что файл не повреждён и ffmpeg установлен."
        ) from exc
    segments: list[TranscriptSegment] = []
    for seg in segments_iter:
        segments.append(
            TranscriptSegment(start=seg.start, end=seg.end, text=seg.text)
        )
        if len(segments) % 20 == 0:
            progress(f"      … сегментов: {len(segments)}")

    progress(f"[3/3] Готово: {len(segments)} сегментов, язык={info.language}")
    return Transcript(language=info.language, segments=segments)
