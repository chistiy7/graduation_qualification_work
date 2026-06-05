from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from .log import progress


def _ensure_ffmpeg() -> str:
    ffmpeg = shutil.which("ffmpeg")
    if not ffmpeg:
        raise RuntimeError(
            "Не найден ffmpeg. Нужен для склейки дорожек.\n"
            "Установка на macOS: brew install ffmpeg"
        )
    return ffmpeg


def merge_audio_tracks(paths: list[Path], *, output_path: Path | None = None) -> Path:
    """
    Склеивает аудиофайлы в один (порядок — как в списке paths).
    Возвращает путь к результирующему файлу.
    """
    if len(paths) < 2:
        raise ValueError("Для склейки нужно минимум 2 файла")

    resolved = [p.expanduser().resolve() for p in paths]
    for p in resolved:
        if not p.is_file():
            raise FileNotFoundError(f"Аудиофайл не найден: {p}")

    ffmpeg = _ensure_ffmpeg()

    if output_path is None:
        suffix = resolved[0].suffix or ".m4a"
        fd, tmp_name = tempfile.mkstemp(suffix=suffix, prefix="merged_")
        os.close(fd)
        output_path = Path(tmp_name)
    else:
        output_path = output_path.expanduser().resolve()
        output_path.parent.mkdir(parents=True, exist_ok=True)

    list_file = output_path.with_suffix(output_path.suffix + ".concat.txt")
    try:
        lines = [f"file '{p.as_posix()}'" for p in resolved]
        list_file.write_text("\n".join(lines) + "\n", encoding="utf-8")

        progress(f"Склейка {len(resolved)} дорожек → {output_path.name}")
        cmd = [
            ffmpeg,
            "-y",
            "-f",
            "concat",
            "-safe",
            "0",
            "-i",
            str(list_file),
            "-ac",
            "1",
            "-ar",
            "16000",
            str(output_path),
        ]
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            detail = (result.stderr or result.stdout or "").strip()
            raise RuntimeError(f"ffmpeg не смог склеить дорожки:\n{detail}")

        if not output_path.is_file() or output_path.stat().st_size == 0:
            raise RuntimeError("Склейка завершилась без выходного файла")

        size_mb = output_path.stat().st_size / (1024 * 1024)
        progress(f"Склеено: {output_path} ({size_mb:.1f} МБ)")
        return output_path
    finally:
        if list_file.is_file():
            list_file.unlink(missing_ok=True)
