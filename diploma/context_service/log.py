from __future__ import annotations

import sys


def progress(message: str) -> None:
    print(message, file=sys.stderr, flush=True)
