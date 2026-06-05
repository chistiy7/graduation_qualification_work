from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path

DIPLOMA_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT_DIR = DIPLOMA_ROOT / "03_capture" / "sessions"
DEFAULT_CONTEXT_DIR = DIPLOMA_ROOT / "00_context"
PROMPTS_DIR = Path(__file__).resolve().parent / "prompts"


@dataclass(frozen=True)
class Settings:
    whisper_model: str
    whisper_device: str
    whisper_compute_type: str
    output_dir: Path
    context_dir: Path

    @classmethod
    def from_env(cls) -> Settings:
        return cls(
            whisper_model=os.getenv("WHISPER_MODEL", "small"),
            whisper_device=os.getenv("WHISPER_DEVICE", "auto"),
            whisper_compute_type=os.getenv("WHISPER_COMPUTE_TYPE", "int8"),
            output_dir=Path(os.getenv("CAPTURE_OUTPUT_DIR", str(DEFAULT_OUTPUT_DIR))),
            context_dir=Path(os.getenv("DIPLOMA_CONTEXT_DIR", str(DEFAULT_CONTEXT_DIR))),
        )
