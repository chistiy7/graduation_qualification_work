from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class FixItem:
    path: Path
    line_no: int
    line: str
    kind: str  # "todo" | "hash"


DEFAULT_EXTS = {
    ".md",
    ".txt",
    ".py",
    ".cpp",
    ".c",
    ".h",
    ".hpp",
    ".cmake",
    ".json",
    ".yml",
    ".yaml",
    ".toml",
    ".sh",
    ".ts",
    ".tsx",
    ".js",
    ".jsx",
}


def _is_ignored_dir(path: Path) -> bool:
    parts = set(path.parts)
    return any(
        p in parts
        for p in {
            ".git",
            ".venv",
            "node_modules",
            "build",
            "dist",
            "__pycache__",
            "03_capture",
        }
    )


_TODO_RE = re.compile(r"(?i)(?:#\s*todo\b)")
# Маркер «#» без текста: пользователь ставит в конце строки или перед EOL
_HASH_MARK_RE = re.compile(r"(?:\s#\s*$)")


def scan_fix_markers(root: Path, *, exts: set[str] | None = None) -> list[FixItem]:
    root = root.expanduser().resolve()
    if not root.is_dir():
        raise FileNotFoundError(f"Каталог не найден: {root}")

    exts = exts or DEFAULT_EXTS
    items: list[FixItem] = []

    for path in root.rglob("*"):
        if path.is_dir():
            continue
        if _is_ignored_dir(path):
            continue
        if path.suffix and path.suffix.lower() not in exts:
            continue
        if path.name.endswith(".docx"):
            continue

        try:
            raw = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue

        for idx, line in enumerate(raw.splitlines(), start=1):
            s = line.rstrip("\n")

            if _TODO_RE.search(s):
                items.append(FixItem(path=path, line_no=idx, line=s.strip(), kind="todo"))
                continue

            # Не считаем markdown-заголовки "# " маркерами (слишком шумно)
            if s.lstrip().startswith("# "):
                continue

            if _HASH_MARK_RE.search(s):
                items.append(FixItem(path=path, line_no=idx, line=s.strip(), kind="hash"))

    # стабильный порядок вывода
    items.sort(key=lambda x: (str(x.path), x.line_no, x.kind))
    return items


def render_fix_queue_md(items: list[FixItem], *, root: Path) -> str:
    root = root.expanduser().resolve()
    lines: list[str] = []
    lines.append("# Очередь правок по меткам #todo / #")
    lines.append("")
    lines.append(f"- Корень сканирования: `{root}`")
    lines.append(f"- Найдено пунктов: **{len(items)}**")
    lines.append("")
    if not items:
        lines.append("_(пометок не найдено)_")
        lines.append("")
        return "\n".join(lines)

    current_file: Path | None = None
    for it in items:
        if current_file != it.path:
            current_file = it.path
            rel = it.path.relative_to(root) if it.path.is_relative_to(root) else it.path
            lines.append(f"## `{rel}`")
            lines.append("")
        kind = "TODO" if it.kind == "todo" else "#"
        safe_line = it.line.replace("\t", "    ")
        lines.append(f"- [ ] **L{it.line_no} [{kind}]** {safe_line}")
    lines.append("")
    return "\n".join(lines)

